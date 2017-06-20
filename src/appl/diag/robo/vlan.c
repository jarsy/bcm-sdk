/*
 * $Id: vlan.c,v 1.38 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#include <shared/bsl.h>

#include <appl/diag/system.h>
#include <appl/diag/parse.h>

#include <bcm/error.h>
#include <bcm/vlan.h>
#include <bcm/l3.h>
#include <bcm/debug.h>
#include <bcm_int/robo/port.h>
#include <bcm_int/common/multicast.h>

#include <soc/mem.h>

#define ROBO_VLAN_IF_ERROR_RETURN(op)                        \
  do {                                                  \
        int __rv__;                                     \
        if ((__rv__ = (op)) < 0) {                      \
            cli_out(\
                    "Error: %s\n", bcm_errmsg(__rv__));  \
            return CMD_FAIL;                            \
        }                                               \
  } while (0)   

/* 
 * Structure used internally to get vlan action arguments. 
 * This structure corresponds to bcm_vlan_action_t but
 * has the action name strings obtained from user input.
 */
typedef struct robo_vlan_action_set_s {
    int   new_outer_vlan;
    int   new_inner_vlan;  
    int   priority; 
    char *dt_outer; 
    char *dt_outer_prio; 
    char *dt_inner;      
    char *dt_inner_prio; 
    char *ot_outer;      
    char *ot_outer_prio; 
    char *ot_inner;      
    char *it_outer;      
    char *it_inner;      
    char *it_inner_prio; 
    char *ut_outer;      
    char *ut_inner;      
    char *outer_pcp;      
    char *inner_pcp;      
} robo_vlan_action_set_t;

static struct robo_vlan_action_names {
    char              *name;
    bcm_vlan_action_t  action;
} robo_vlan_action_names[] = {
    {  "None",     bcmVlanActionNone    },
    {  "Add",      bcmVlanActionAdd     },
    {  "Replace",  bcmVlanActionReplace },    
    {  "Delete",   bcmVlanActionDelete  }    
};

static struct robo_vlan_pcp_action_names {
    char              *name;
    bcm_vlan_pcp_action_t  action;
} robo_vlan_pcp_action_names[] = {
    {  "None",     bcmVlanPcpActionNone    },
    {  "Mapped",      bcmVlanPcpActionMapped     },
    {  "IngressInnerPcp",  bcmVlanPcpActionIngressInnerPcp },    
    {  "IngressOuterPcp",  bcmVlanPcpActionIngressOuterPcp },    
    {  "PortDefault",   bcmVlanPcpActionPortDefault  }    
};

static struct robo_vlan_key_type_names {
    char                     *name;
    bcm_vlan_translate_key_t  key;
} robo_vlan_key_type_names[] = {
    { "",              bcmVlanTranslateKeyInvalid       },
    { "Double",        bcmVlanTranslateKeyDouble        },
    { "Outer",         bcmVlanTranslateKeyOuter         },
    { "Inner",         bcmVlanTranslateKeyInner         },
    { "OuterTag",      bcmVlanTranslateKeyOuterTag      },
    { "InnerTag",      bcmVlanTranslateKeyInnerTag      },
    { "PortDouble",    bcmVlanTranslateKeyPortDouble    },
    { "PortOuter",     bcmVlanTranslateKeyPortOuter     },
    { "PortInner",     bcmVlanTranslateKeyPortInner     },
    { "PortOuterTag",  bcmVlanTranslateKeyPortOuterTag  },
    { "PortInnerTag",  bcmVlanTranslateKeyPortInnerTag  }
};

STATIC void 
_robo_vlan_action_set_t_init(robo_vlan_action_set_t *data)
{
    if (data != NULL) {
        sal_memset(data, 0, sizeof(robo_vlan_action_set_t));
    }
}

STATIC INLINE void
_robo_parse_table_vlan_action_set_add(parse_table_t *pt, robo_vlan_action_set_t *act)
{
    if (pt == NULL || act == NULL) {
        return;
    }
    parse_table_add(pt, "OuterVlan",   PQ_HEX, 0,
                    &(act->new_outer_vlan), NULL);
    parse_table_add(pt, "InnerVlan",   PQ_HEX, 0, 
                    &(act->new_inner_vlan), NULL);
    parse_table_add(pt, "Prio",        PQ_INT, 0, 
                    &(act->priority), NULL);
    parse_table_add(pt, "DtOuter",     PQ_STRING, "None",
                    &(act->dt_outer), NULL);   
    parse_table_add(pt, "DtOuterPrio", PQ_STRING, "None",
                    &(act->dt_outer_prio), NULL);  
    parse_table_add(pt, "DtInner",     PQ_STRING, "None",
                    &(act->dt_inner), NULL);   
    parse_table_add(pt, "DtInnerPrio", PQ_STRING, "None", 
                    &(act->dt_inner_prio), NULL);  
    parse_table_add(pt, "OtOuter",     PQ_STRING, "None", 
                    &(act->ot_outer), NULL);   
    parse_table_add(pt, "OtOuterPrio", PQ_STRING, "None", 
                    &(act->ot_outer_prio), NULL);  
    parse_table_add(pt, "OtInner",     PQ_STRING, "None", 
                    &(act->ot_inner), NULL);   
    parse_table_add(pt, "ItOuter",     PQ_STRING, "None", 
                    &(act->it_outer), NULL);   
    parse_table_add(pt, "ItInner",     PQ_STRING, "None", 
                    &(act->it_inner), NULL);   
    parse_table_add(pt, "ItInnerPrio", PQ_STRING, "None", 
                    &(act->it_inner_prio), NULL);  
    parse_table_add(pt, "UtOuter",     PQ_STRING, "None", 
                    &(act->ut_outer), NULL);   
    parse_table_add(pt, "UtInner",     PQ_STRING, "None", 
                    &(act->ut_inner), NULL);   
    parse_table_add(pt, "OuterPcp",     PQ_STRING, "None", 
                    &(act->outer_pcp), NULL);   
    parse_table_add(pt, "InnerPcp",     PQ_STRING, "None", 
                    &(act->inner_pcp), NULL);   
}

/*
 * Return bcm_vlan_action_t action from action name string
 */
STATIC int
_robo_vlan_action_get(const char *action_name)
{
    int i, names;
    
    if (action_name == NULL || *action_name == '\0') {
        return 0;
    }

    names = sizeof(robo_vlan_action_names) / sizeof(struct robo_vlan_action_names);
    for (i = 0; i < names; i++) {
        if (sal_strcasecmp(action_name, robo_vlan_action_names[i].name) == 0) {
            return robo_vlan_action_names[i].action;
        } 
    }
    cli_out("Invalid action <%s>. Valid actions are:\n   ", action_name);
    for (i = 0; i < names; i++) {
        cli_out("%s ", robo_vlan_action_names[i].name);
    }
    cli_out("\n");
    return -1;
}

/*
 * Return bcm_vlan_pcp_action_t action from action name string
 */
STATIC int
_robo_pcp_action_get(const char *action_name)
{
    int i, names;
    
    if (action_name == NULL || *action_name == '\0') {
        return 0;
    }

    names = sizeof(robo_vlan_pcp_action_names) / sizeof(struct robo_vlan_pcp_action_names);
    for (i = 0; i < names; i++) {
        if (sal_strcasecmp(action_name, robo_vlan_pcp_action_names[i].name) == 0) {
            return robo_vlan_pcp_action_names[i].action;
        } 
    }
    cli_out("Invalid pcp action <%s>. Valid pcp actions are:\n   ", action_name);
    for (i = 0; i < names; i++) {
        cli_out("%s ", robo_vlan_pcp_action_names[i].name);
    }
    cli_out("\n");
    return -1;
}


#define ROBO_VLAN_IF_BAD_ACTION_RETURN(a, op)     \
    if (((a = (op)) == -1)) { return CMD_FAIL; }


STATIC cmd_result_t
_robo_vlan_to_vlan_action_set(bcm_vlan_action_set_t *action, robo_vlan_action_set_t *act)
{
    if (action == NULL || act == NULL) {
        return CMD_FAIL;
    }
    action->new_outer_vlan = act->new_outer_vlan;
    action->new_inner_vlan = act->new_inner_vlan;
    action->priority = act->priority;
    ROBO_VLAN_IF_BAD_ACTION_RETURN(action->dt_outer, 
                              _robo_vlan_action_get(act->dt_outer));
    ROBO_VLAN_IF_BAD_ACTION_RETURN(action->dt_outer_prio, 
                              _robo_vlan_action_get(act->dt_outer_prio));
    ROBO_VLAN_IF_BAD_ACTION_RETURN(action->dt_inner, 
                              _robo_vlan_action_get(act->dt_inner));
    ROBO_VLAN_IF_BAD_ACTION_RETURN(action->dt_inner_prio, 
                              _robo_vlan_action_get(act->dt_inner_prio));
    ROBO_VLAN_IF_BAD_ACTION_RETURN(action->ot_outer, 
                              _robo_vlan_action_get(act->ot_outer));
    ROBO_VLAN_IF_BAD_ACTION_RETURN(action->ot_outer_prio, 
                              _robo_vlan_action_get(act->ot_outer_prio));
    ROBO_VLAN_IF_BAD_ACTION_RETURN(action->ot_inner, 
                              _robo_vlan_action_get(act->ot_inner));
    ROBO_VLAN_IF_BAD_ACTION_RETURN(action->it_outer, 
                              _robo_vlan_action_get(act->it_outer));
    ROBO_VLAN_IF_BAD_ACTION_RETURN(action->it_inner, 
                              _robo_vlan_action_get(act->it_inner));
    ROBO_VLAN_IF_BAD_ACTION_RETURN(action->it_inner_prio, 
                              _robo_vlan_action_get(act->it_inner_prio));
    ROBO_VLAN_IF_BAD_ACTION_RETURN(action->ut_outer, 
                              _robo_vlan_action_get(act->ut_outer));
    ROBO_VLAN_IF_BAD_ACTION_RETURN(action->ut_inner, 
                              _robo_vlan_action_get(act->ut_inner));
    ROBO_VLAN_IF_BAD_ACTION_RETURN(action->outer_pcp, 
                              _robo_pcp_action_get(act->outer_pcp));
    ROBO_VLAN_IF_BAD_ACTION_RETURN(action->inner_pcp, 
                              _robo_pcp_action_get(act->inner_pcp));
    return CMD_OK;
}

STATIC INLINE char *
_robo_vlan_action_name_get(bcm_vlan_action_t action)
{
    int names = sizeof(robo_vlan_action_names) / sizeof(struct robo_vlan_action_names);
    return ((action >= 0 && action < names) ?  robo_vlan_action_names[action].name :
            "Invalid action");
}

STATIC INLINE char *
_robo_vlan_pcp_action_name_get(bcm_vlan_pcp_action_t action)
{
    int names = sizeof(robo_vlan_pcp_action_names) / sizeof(struct robo_vlan_pcp_action_names);
    return ((action >= 0 && action < names) ?  robo_vlan_pcp_action_names[action].name :
            "Invalid action");
}

STATIC INLINE void
_robo_vlan_action_set_print(const bcm_vlan_action_set_t *action)
{
    if (action == NULL) {
        return;
    }
    cli_out("  DT: Outer=%-7s  OuterPrio=%-7s  Inner=%-7s  InnerPrio=%-7s\n",
            _robo_vlan_action_name_get(action->dt_outer),
            _robo_vlan_action_name_get(action->dt_outer_prio),
            _robo_vlan_action_name_get(action->dt_inner),
            _robo_vlan_action_name_get(action->dt_inner_prio));
    cli_out("  OT: Outer=%-7s  OuterPrio=%-7s  Inner=%-7s\n",
            _robo_vlan_action_name_get(action->ot_outer),
            _robo_vlan_action_name_get(action->ot_outer_prio),
            _robo_vlan_action_name_get(action->ot_inner));
    cli_out("  IT: Outer=%-7s  Inner=%-7s      InnerPrio=%-7s\n",
            _robo_vlan_action_name_get(action->it_outer),
            _robo_vlan_action_name_get(action->it_inner),
            _robo_vlan_action_name_get(action->it_inner_prio));
    cli_out("  UT: Outer=%-7s  Inner=%-7s\n",
            _robo_vlan_action_name_get(action->ut_outer),
            _robo_vlan_action_name_get(action->ut_inner));
    cli_out("  OuterPcp Policy=%-7s  InnerPcp Policy=%-7s\n",
            _robo_vlan_pcp_action_name_get(action->outer_pcp),
            _robo_vlan_pcp_action_name_get(action->inner_pcp));
}

STATIC int
_robo_vlan_action_translate_key_get(const char *key_str)
{
    int  i, names;
    
    names = sizeof(robo_vlan_key_type_names) / sizeof(struct robo_vlan_key_type_names); 
 
    /* Get bcm_vlan_translate_key_t key type from key type string */
    for (i = 1; i < names; i++) {
        if (key_str && 
            (sal_strcasecmp(key_str, robo_vlan_key_type_names[i].name) == 0)) {
            return robo_vlan_key_type_names[i].key;
        }
    }
    cli_out("Invalid key type <%s>. Valid key types are:\n   ", 
            key_str ? key_str : "");
    for (i = 1;  i < names; i++) {
        cli_out("%s ", robo_vlan_key_type_names[i].name);
        if (i % 7 == 0) {
            cli_out("\n   ");
        }
    }
    cli_out("\n");
    return bcmVlanTranslateKeyInvalid; 
}

STATIC INLINE char *
_robo_vlan_action_translate_key_name_get(bcm_vlan_translate_key_t key_type)
{
    int names = sizeof(robo_vlan_key_type_names) / sizeof(struct robo_vlan_key_type_names); 
    return (key_type >= names ? robo_vlan_key_type_names[0].name : 
            robo_vlan_key_type_names[key_type].name);
}

STATIC int 
_robo_vlan_translate_action_print(int unit, bcm_gport_t gport, 
                             bcm_vlan_translate_key_t key_type, 
                             bcm_vlan_t outer_vlan, 
                             bcm_vlan_t inner_vlan, 
                             bcm_vlan_action_set_t *action, 
                             void *user_data)
{
    int             id;
    bcm_port_t      port;
    bcm_module_t    modid;
    bcm_trunk_t     trunk;

    if (action == NULL) {
        return CMD_FAIL;
    }

    if (key_type >= bcmVlanTranslateKeyPortDouble) {
        /* Port information only valid with Port+VID keys */
        ROBO_VLAN_IF_ERROR_RETURN(
            _bcm_robo_gport_resolve(unit, gport, &modid, &port, &trunk, &id));
        if (BCM_GPORT_IS_TRUNK(gport)) {
            cli_out("TGID=%d, ", trunk);
        } else {
            cli_out("Port=%d, Modid=%d, ", port, modid);
        }
    }
    cli_out("KeyType=%s, ",_robo_vlan_action_translate_key_name_get(key_type));
    if (outer_vlan == BCM_VLAN_INVALID) {
        cli_out("OldOuterVlan=--, ");
    } else {
        cli_out("OldOuterVlan=%d, ", outer_vlan);
    }
    if (inner_vlan == BCM_VLAN_INVALID) {
        cli_out("OldInnerVlan=--\n");
    } else {
        cli_out("OldInnerVlan=%d\n", inner_vlan);
    }
    _robo_vlan_action_set_print(action);

    return CMD_OK;
}

#if defined(BCM_53115) || defined(BCM_53125) || defined(BCM_POLAR_SUPPORT) || \
    defined(BCM_NORTHSTAR_SUPPORT)
STATIC int 
_robo_vlan_translate_egress_action_print(int unit, int port_class, 
                                    bcm_vlan_t outer_vlan,
                                    bcm_vlan_t inner_vlan,
                                    bcm_vlan_action_set_t *action,
                                    void *user_data) 
{
    int     parsed_port_class = 0;

    /* parse the sepcial gport type */
    parsed_port_class = BCM_GPORT_SPECIAL_GET(port_class);
    cli_out("PortClass=%d, OldOuterVlan=%d, OldInnerVlan=%d\n",
            parsed_port_class, outer_vlan, inner_vlan);
    _robo_vlan_action_set_print(action);

    return CMD_OK;
}

STATIC cmd_result_t
_robo_vlan_action_translate_egress(int unit, args_t *a)
{
    char        *subcmd;
    int         gport = -1;
    int         special = 0;    /* for SPECIAL GPORT TYPE */

    if ((subcmd = ARG_GET(a)) == NULL) {
        return CMD_USAGE;
    }

    if (sal_strcasecmp(subcmd, "add") == 0) {
        parse_table_t       pt;
        cmd_result_t        r;
        int                 old_outer_vlan, old_inner_vlan, port_class = 0;
        robo_vlan_action_set_t  act;
        bcm_vlan_action_set_t action;            
   
        _robo_vlan_action_set_t_init(&act);
        parse_table_init(unit, &pt);
        /* 
         * The bcmPortClassVlanTranslateEgress port class is created with 
         * bcm_port_class_set(). The default bcmPortClassVlanTransateEgress 
         * class for a port is its own port number.
         */
        parse_table_add(&pt, "PortClass", PQ_INT, 0, &port_class, NULL);
        parse_table_add(&pt, "OldOuterVLan", PQ_INT, 0, &old_outer_vlan, NULL);
        parse_table_add(&pt, "OldInnerVLan", PQ_INT, 0, &old_inner_vlan, NULL);
        parse_table_add(&pt, "Special", PQ_INT, 0, &special, NULL);
        _robo_parse_table_vlan_action_set_add(&pt, &act);
        if (parse_arg_eq(a, &pt) < 0) {
            cli_out("Error: invalid option %s\n", 
                    ARG_CUR(a));
            parse_arg_eq_done(&pt);
            return CMD_USAGE;
        }
        
        bcm_vlan_action_set_t_init(&action);
        r = _robo_vlan_to_vlan_action_set(&action, &act);
        if (r != CMD_OK) {
            parse_arg_eq_done(&pt);
            cli_out("Error: invalid action\n");
            return r;
        }
        /* Done with string parameters */
        parse_arg_eq_done(&pt);


        /* set GPORT type */
        if (special){
            /* set port_class to GPORT_SPECIAL_TYPE */
            BCM_GPORT_SPECIAL_SET(gport, port_class);
        } else {
            BCM_GPORT_LOCAL_SET(gport, port_class);
        }

        ROBO_VLAN_IF_ERROR_RETURN(
            bcm_vlan_translate_egress_action_add(unit, gport,
                                                 old_outer_vlan, 
                                                 old_inner_vlan,
                                                 &action));
        return CMD_OK;

    } else if ((sal_strcasecmp(subcmd, "delete") == 0) ||
               (sal_strcasecmp(subcmd, "get") == 0)) {
        parse_table_t       pt;
        cmd_result_t        r;
        int                 old_outer_vlan, old_inner_vlan, port_class = 0;

        parse_table_init(unit, &pt);
        parse_table_add(&pt, "PortClass", PQ_INT, 0, &port_class, NULL);
        parse_table_add(&pt, "OldOuterVLan", PQ_INT, 0, &old_outer_vlan, NULL);
        parse_table_add(&pt, "OldInnerVLan", PQ_INT, 0, &old_inner_vlan, NULL);
        parse_table_add(&pt, "Special", PQ_INT, 0, &special, NULL);
        if (!parseEndOk( a, &pt, &r)) {
            return r;
        }

        /* set GPORT type */
        if (special){
            /* set port_class to GPORT_SPECIAL_TYPE */
            BCM_GPORT_SPECIAL_SET(gport, port_class);
        } else {
            BCM_GPORT_LOCAL_SET(gport, port_class);
        }

        if (sal_strcasecmp(subcmd, "delete") == 0) {
            ROBO_VLAN_IF_ERROR_RETURN(bcm_vlan_translate_egress_action_delete(
                    unit, gport, old_outer_vlan, old_inner_vlan));
            return CMD_OK;
        } else { /* get */
            bcm_vlan_action_set_t action;            
            ROBO_VLAN_IF_ERROR_RETURN(
                    bcm_vlan_translate_egress_action_get(unit, gport, 
                            old_outer_vlan, old_inner_vlan, &action));
            _robo_vlan_translate_egress_action_print(unit, gport,
                    old_outer_vlan, old_inner_vlan, &action, NULL);
            return CMD_OK;
        }
    } else  if (sal_strcasecmp(subcmd, "show") == 0) {
        ROBO_VLAN_IF_ERROR_RETURN(bcm_vlan_translate_egress_action_traverse(
                unit, _robo_vlan_translate_egress_action_print, NULL));
        return CMD_OK;
    }

    return CMD_USAGE;
}
#endif  /* BCM_53115 || BCM_53125 || BCM_POLAR_SUPPORT || NORTHSTAR */

STATIC int 
_robo_vlan_translate_range_action_print(int unit, bcm_gport_t gport, 
                                   bcm_vlan_t outer_vlan_lo, 
                                   bcm_vlan_t outer_vlan_hi, 
                                   bcm_vlan_t inner_vlan_lo, 
                                   bcm_vlan_t inner_vlan_hi, 
                                   bcm_vlan_action_set_t *action, 
                                   void *user_data)
{
    int             id;
    bcm_port_t      port;
    bcm_module_t    modid;
    bcm_trunk_t     trunk;

    if (action == NULL) {
        return CMD_FAIL;
    }

    ROBO_VLAN_IF_ERROR_RETURN(
        _bcm_robo_gport_resolve(unit, gport, &modid, &port, &trunk, &id));
    if (BCM_GPORT_IS_TRUNK(gport)) {
        cli_out("TGID=%d, ", trunk);
    } else {
        cli_out("Port=%d, Modid=%d, ", port, modid);
    }
    cli_out("OuterVlanLo=%d, OuterVlanHi=%d, InnerVlanLo=%d, "
            "InnerVlanHi=%d\n", outer_vlan_lo, outer_vlan_hi,
            inner_vlan_lo, inner_vlan_hi);
    _robo_vlan_action_set_print(action);

    return CMD_OK;
}

STATIC int 
_robo_vlan_port_protocol_action_print(int unit, bcm_port_t port, 
                             bcm_port_frametype_t frame, 
                             bcm_port_ethertype_t ether, 
                             bcm_vlan_action_set_t *action, 
                             void *user_data)
{
    if (action == NULL) {
        return CMD_FAIL;
    }

    cli_out("Port=%d, ", port);

    cli_out("Frame=0x%x, ",frame);
    cli_out("EtherType=0x%x\n",ether);
    _robo_vlan_action_set_print(action);

    return CMD_OK;
}

STATIC cmd_result_t
_robo_vlan_action_translate_range(int unit, args_t *a)
{
    char       *subcmd;

    if ((subcmd = ARG_GET(a)) == NULL) {
        return CMD_USAGE;
    }

    if (sal_strcasecmp(subcmd, "add") == 0) {
        parse_table_t       pt;
        cmd_result_t        r;
        bcm_gport_t         gport;
        int                 port, 
                            outer_vlan_lo = BCM_VLAN_INVALID, 
                            outer_vlan_hi = BCM_VLAN_INVALID, 
                            inner_vlan_lo = BCM_VLAN_INVALID, 
                            inner_vlan_hi = BCM_VLAN_INVALID; 
        robo_vlan_action_set_t  act;
        bcm_vlan_action_set_t action;            
   
        _robo_vlan_action_set_t_init(&act);
        parse_table_init(unit, &pt);
        parse_table_add(&pt, "Port", PQ_INT, 0, &port, NULL);
        parse_table_add(&pt, "OuterVLanLo", PQ_DFL | PQ_INT, 0, 
                        &outer_vlan_lo, NULL);
        parse_table_add(&pt, "OuterVLanHi", PQ_DFL | PQ_INT, 0, 
                        &outer_vlan_hi, NULL);
        parse_table_add(&pt, "InnerVLanLo", PQ_DFL | PQ_INT, 0, 
                        &inner_vlan_lo, NULL);
        parse_table_add(&pt, "InnerVLanHi", PQ_DFL | PQ_INT, 0, 
                        &inner_vlan_hi, NULL);
        _robo_parse_table_vlan_action_set_add(&pt, &act);
        if (parse_arg_eq(a, &pt) < 0) {
            cli_out("Error: invalid option %s\n", 
                    ARG_CUR(a));
            parse_arg_eq_done(&pt);
            return CMD_USAGE;
        }
        
        bcm_vlan_action_set_t_init(&action);
        r = _robo_vlan_to_vlan_action_set(&action, &act);
        if (r != CMD_OK) {
            parse_arg_eq_done(&pt);
            cli_out("Error: invalid action\n");
            return r;
        }
        /* Done with string parameters */
        parse_arg_eq_done(&pt);
        BCM_GPORT_LOCAL_SET(gport, port);
        ROBO_VLAN_IF_ERROR_RETURN(
            bcm_vlan_translate_action_range_add(unit, gport,
                                                outer_vlan_lo, 
                                                outer_vlan_hi, 
                                                inner_vlan_lo,
                                                inner_vlan_hi,
                                                &action));
        return CMD_OK;

    } else if ((sal_strcasecmp(subcmd, "delete") == 0) ||
               (sal_strcasecmp(subcmd, "get") == 0)) {
        parse_table_t       pt;
        cmd_result_t        r;
        bcm_gport_t         gport;
        int                 port = 0,
                            outer_vlan_lo = BCM_VLAN_INVALID, 
                            outer_vlan_hi = BCM_VLAN_INVALID, 
                            inner_vlan_lo = BCM_VLAN_INVALID, 
                            inner_vlan_hi = BCM_VLAN_INVALID; 

        parse_table_init(unit, &pt);
        parse_table_add(&pt, "Port", PQ_INT, 0, &port, NULL);
        parse_table_add(&pt, "OuterVLanLo", PQ_DFL | PQ_INT, 0, 
                        &outer_vlan_lo, NULL);
        parse_table_add(&pt, "OuterVLanHi", PQ_DFL | PQ_INT, 0, 
                        &outer_vlan_hi, NULL);
        parse_table_add(&pt, "InnerVLanLo", PQ_DFL | PQ_INT, 0, 
                        &inner_vlan_lo, NULL);
        parse_table_add(&pt, "InnerVLanHi", PQ_DFL | PQ_INT, 0, 
                        &inner_vlan_hi, NULL);
        if (!parseEndOk( a, &pt, &r)) {
            return r;
        }
        BCM_GPORT_LOCAL_SET(gport, port);
        if (sal_strcasecmp(subcmd, "delete") == 0) {
            ROBO_VLAN_IF_ERROR_RETURN(
                bcm_vlan_translate_action_range_delete(unit, gport,
                                                       outer_vlan_lo, 
                                                       outer_vlan_hi, 
                                                       inner_vlan_lo,
                                                       inner_vlan_hi));

            return CMD_OK;

        } else { /* get */
            bcm_vlan_action_set_t action;            
            ROBO_VLAN_IF_ERROR_RETURN(
                bcm_vlan_translate_action_range_get(unit, gport,
                                                    outer_vlan_lo, 
                                                    outer_vlan_hi, 
                                                    inner_vlan_lo,
                                                    inner_vlan_hi,
                                                    &action));
            cli_out("Port=%d, OuterVlanLo=%d, OuterVlanHi=%d, InnerVlanLo=%d, "
                    "InnerVlanHi=%d\n", port, outer_vlan_lo, outer_vlan_hi,
                    inner_vlan_lo, inner_vlan_hi);
            _robo_vlan_action_set_print(&action);
            return CMD_OK;
        }
    } else if (sal_strcasecmp(subcmd, "show") == 0) {
         ROBO_VLAN_IF_ERROR_RETURN(
             bcm_vlan_translate_action_range_traverse(unit, 
                                             _robo_vlan_translate_range_action_print,
                                             NULL));
         return CMD_OK;
     } else if (sal_strcasecmp(subcmd, "clear") == 0) {
         ROBO_VLAN_IF_ERROR_RETURN(
             bcm_vlan_translate_action_range_delete_all(unit));
         return CMD_OK;
     } 

    return CMD_USAGE;
}

STATIC cmd_result_t
_robo_vlan_action_translate(int unit, args_t *a)
{
    char       *subcmd;

    if ((subcmd = ARG_GET(a)) == NULL) {
        return CMD_USAGE;
    }

#if defined(BCM_53115) || defined(BCM_53125) || defined(BCM_POLAR_SUPPORT) || \
    defined(BCM_NORTHSTAR_SUPPORT)
    if (sal_strcasecmp(subcmd, "egress") == 0) {
        return _robo_vlan_action_translate_egress(unit, a);
    } 
#endif  /* bcm_53115 || BCM_53125 || BCM_POLAR_SUPPORT || NORTHSTAR */

    if (sal_strcasecmp(subcmd, "range") == 0) {
        return _robo_vlan_action_translate_range(unit, a);
    } 

    if (sal_strcasecmp(subcmd, "add") == 0) {
        parse_table_t       pt;
        cmd_result_t        r;
        int                 old_outer_vlan, old_inner_vlan, port = 0; 
        bcm_gport_t         gport;
        char               *key_str = NULL;
        bcm_vlan_translate_key_t key_type;
        robo_vlan_action_set_t  act;
        bcm_vlan_action_set_t action;            

        _robo_vlan_action_set_t_init(&act);
        parse_table_init(unit, &pt);
        parse_table_add(&pt, "Port", PQ_INT, 0, &port, NULL);
        parse_table_add(&pt, "KeyType", PQ_STRING, 0, &key_str, NULL);
        parse_table_add(&pt, "OldOuterVLan", PQ_INT, 0, &old_outer_vlan, NULL);
        parse_table_add(&pt, "OldInnerVLan", PQ_INT, 0, &old_inner_vlan, NULL);
        _robo_parse_table_vlan_action_set_add(&pt, &act);
        if (parse_arg_eq(a, &pt) < 0) {
            cli_out("Error: invalid option %s\n", ARG_CUR(a));
            parse_arg_eq_done(&pt);
            return CMD_USAGE;
        }
        bcm_vlan_action_set_t_init(&action);
        r = _robo_vlan_to_vlan_action_set(&action, &act);
        if (r != CMD_OK) {
            parse_arg_eq_done(&pt);
            cli_out("Error: invalid action\n");
            return r;
        }
        key_type = _robo_vlan_action_translate_key_get(key_str);
        parse_arg_eq_done(&pt);
        if (key_type == bcmVlanTranslateKeyInvalid) {
            cli_out("Error: invalid key\n");
            return CMD_FAIL;
        }
        
        BCM_GPORT_LOCAL_SET(gport, port);
        ROBO_VLAN_IF_ERROR_RETURN(
            bcm_vlan_translate_action_add(unit, gport, key_type, 
                                          old_outer_vlan,
                                          old_inner_vlan, &action));
        return CMD_OK;

    } else if ((sal_strcasecmp(subcmd, "delete") == 0) ||
               (sal_strcasecmp(subcmd, "get") == 0)) {
        parse_table_t       pt;
        int                 old_outer_vlan, old_inner_vlan, port = 0; 
        char               *key_str = NULL;
        bcm_gport_t         gport;
        bcm_vlan_translate_key_t key_type;

        parse_table_init(unit, &pt);
        parse_table_add(&pt, "Port", PQ_INT, 0, &port, NULL);
        parse_table_add(&pt, "KeyType", PQ_STRING, 0, &key_str, NULL);
        parse_table_add(&pt, "OldOuterVLan", PQ_INT, 0, &old_outer_vlan, NULL);
        parse_table_add(&pt, "OldInnerVLan", PQ_INT, 0, &old_inner_vlan, NULL);
        if (parse_arg_eq(a, &pt) < 0) {
            cli_out("Error: invalid option %s\n", ARG_CUR(a));
            parse_arg_eq_done(&pt);
            return CMD_USAGE;
        }
        key_type = _robo_vlan_action_translate_key_get(key_str);
        parse_arg_eq_done(&pt);
        if (key_type == bcmVlanTranslateKeyInvalid) {
            cli_out("Error: invalid key\n");
            return CMD_FAIL;
        }
        BCM_GPORT_LOCAL_SET(gport, port);
        if (sal_strcasecmp(subcmd, "delete") == 0) {
            ROBO_VLAN_IF_ERROR_RETURN(
                bcm_vlan_translate_action_delete(unit, gport, key_type, 
                                                 old_outer_vlan,
                                                 old_inner_vlan));
            return CMD_OK; 

        } else {
            bcm_vlan_action_set_t action;            
            key_str = _robo_vlan_action_translate_key_name_get(key_type);
            ROBO_VLAN_IF_ERROR_RETURN(
                bcm_vlan_translate_action_get(unit, gport, key_type, 
                                              old_outer_vlan,
                                              old_inner_vlan,
                                              &action));
            cli_out("Port=%d, KeyType=%s, OldOuterVlan=%d, OldInnerVlan=%d\n",
                    port, key_str, old_outer_vlan, old_inner_vlan);
            _robo_vlan_action_set_print(&action);
            return CMD_OK;
        }

    } else if (sal_strcasecmp(subcmd, "show") == 0) {
        ROBO_VLAN_IF_ERROR_RETURN(
            bcm_vlan_translate_action_traverse(unit, 
                                               _robo_vlan_translate_action_print,
                                               NULL));
        return CMD_OK;
    } 
    return CMD_USAGE;
}

STATIC cmd_result_t
_robo_vlan_action_protocol(int unit, args_t *a)
{
    char       *subcmd;
 
    if ((subcmd = ARG_GET(a)) == NULL) {
        return CMD_USAGE;
    }

    if (sal_strcasecmp(subcmd, "add") == 0) {
        parse_table_t       pt;
        robo_vlan_action_set_t  act;
        cmd_result_t        r;
        bcm_pbmp_t          pbmp;
        bcm_port_t          port;
        int                 frame, ether;
        bcm_vlan_action_set_t action;            

        _robo_vlan_action_set_t_init(&act);
        parse_table_init(unit, &pt);
        parse_table_add(&pt, "PortBitMap", PQ_PBMP|PQ_BCM, 0, &pbmp, NULL);
        parse_table_add(&pt, "Frame", PQ_INT, 0, &frame, NULL);
        parse_table_add(&pt, "Ether", PQ_HEX, 0, &ether, NULL);
        _robo_parse_table_vlan_action_set_add(&pt, &act);
        if (parse_arg_eq(a, &pt) < 0) {
            cli_out("Error: invalid option: %s\n", ARG_CUR(a));
            parse_arg_eq_done(&pt);
            return CMD_USAGE;
        }
        
        bcm_vlan_action_set_t_init(&action);
        r = _robo_vlan_to_vlan_action_set(&action, &act);
        parse_arg_eq_done(&pt);
        if (r != CMD_OK) {
            cli_out("Error: invalid action\n");
            return r;
        }
        
        PBMP_ITER(pbmp, port) {
            ROBO_VLAN_IF_ERROR_RETURN(
                bcm_vlan_port_protocol_action_add(unit, port, frame, ether, 
                                                  &action));
        }
        return CMD_OK;
    } else if ((sal_strcasecmp(subcmd, "delete") == 0) ||
               (sal_strcasecmp(subcmd, "get") == 0)) {
        parse_table_t       pt;
        int                 frame, ether, port = 0; 
        bcm_pbmp_t          pbmp;

        parse_table_init(unit, &pt);
        parse_table_add(&pt, "PortBitMap", PQ_PBMP|PQ_BCM, 0, &pbmp, NULL);
        parse_table_add(&pt, "Frame", PQ_INT, 0, &frame, NULL);
        parse_table_add(&pt, "Ether", PQ_HEX, 0, &ether, NULL);
        if (parse_arg_eq(a, &pt) < 0) {
            cli_out("Error: invalid option %s\n", ARG_CUR(a));
            parse_arg_eq_done(&pt);
            return CMD_USAGE;
        }

        parse_arg_eq_done(&pt);

        if (sal_strcasecmp(subcmd, "delete") == 0) {
            PBMP_ITER(pbmp, port) {
                ROBO_VLAN_IF_ERROR_RETURN(
                    bcm_vlan_port_protocol_action_delete(unit, port, 
                                                     frame,
                                                     ether));
            }
            return CMD_OK; 

        } else {
            bcm_vlan_action_set_t action;            

            PBMP_ITER(pbmp, port) {
                bcm_vlan_action_set_t_init(&action);
                ROBO_VLAN_IF_ERROR_RETURN(
                    bcm_vlan_port_protocol_action_get(unit, port,
                                                  frame,
                                                  ether,
                                                  &action));
                cli_out("Port=%d, Frame=0x%x, EtherType=0x%x\n",
                        port, frame, ether);
                _robo_vlan_action_set_print(&action);
            }
            return CMD_OK;
        }
    } else if (sal_strcasecmp(subcmd, "clear") == 0) {
        parse_table_t       pt;
        int                 port = 0; 
        bcm_pbmp_t          pbmp;

        parse_table_init(unit, &pt);
        parse_table_add(&pt, "PortBitMap", PQ_PBMP|PQ_BCM, 0, &pbmp, NULL);
        if (parse_arg_eq(a, &pt) < 0) {
            cli_out("Error: invalid option %s\n", ARG_CUR(a));
            parse_arg_eq_done(&pt);
            return CMD_USAGE;
        }
        parse_arg_eq_done(&pt);


        PBMP_ITER(pbmp, port) {
            ROBO_VLAN_IF_ERROR_RETURN(
                bcm_vlan_port_protocol_action_delete_all(unit, port));
        }

        return CMD_OK;
    } else if (sal_strcasecmp(subcmd, "show") == 0) {
        ROBO_VLAN_IF_ERROR_RETURN(
            bcm_vlan_port_protocol_action_traverse(unit, 
                                               _robo_vlan_port_protocol_action_print,
                                               NULL));
        return CMD_OK;
    } 

    return CMD_USAGE;
}

cmd_result_t
if_robo_pvlan(int u, args_t *a)
{
    char *subcmd, *argpbm, *argvid;
    vlan_id_t vid = BCM_VLAN_INVALID;
    soc_port_t port;
    int rv;
    pbmp_t pbm;
    pbmp_t vid_pbmp, vid_ubmp;

    if (! sh_check_attached(ARG_CMD(a), u)) {
        return CMD_FAIL;
    }

    if ((subcmd = ARG_GET(a)) == NULL) {
        subcmd = "show";
    }

    if ((argpbm = ARG_GET(a)) == NULL) {
        pbm = PBMP_ALL(u);
    } else {
        if (parse_pbmp(u, argpbm, &pbm) < 0) {
            cli_out("%s: ERROR: unrecognized port bitmap: %s\n",
                    ARG_CMD(a), argpbm);
            return CMD_FAIL;
        }
        SOC_PBMP_AND(pbm, PBMP_ALL(u));
    }

    if (sal_strcasecmp(subcmd, "show") == 0) {
        rv = BCM_E_NONE;

        PBMP_ITER(pbm, port) {
            if ((rv = bcm_port_untagged_vlan_get(u, port, &vid)) < 0) {
                    cli_out("Error retrieving info for port %s: %s\n",
                            SOC_PORT_NAME(u, port), bcm_errmsg(rv));
                break;
            }
    
        cli_out("Port %s default VLAN is %d\n",
                BCM_PORT_NAME(u, port), vid);
        }

        return (rv < 0) ? CMD_FAIL : CMD_OK;
    } else if (sal_strcasecmp(subcmd, "set") == 0) {
        if ((argvid = ARG_GET(a)) == NULL) {
            cli_out("Missing VID for set.\n");
            return CMD_USAGE;
        }
        vid = sal_ctoi(argvid, 0);
    } else {
        return CMD_USAGE;
    }

    /* Set default VLAN as indicated */

    rv = BCM_E_NONE;

    /* Check with the forward pbmp of this vlanid */
    if ((rv = bcm_vlan_port_get(u, vid, &vid_pbmp, &vid_ubmp)) < 0) {
        SOC_PBMP_ASSIGN(vid_pbmp, PBMP_ALL(u));
    }
    SOC_PBMP_AND(pbm, vid_pbmp);

    PBMP_ITER(pbm, port) {
        if ((rv = bcm_port_untagged_vlan_set(u, port, vid)) < 0) {
            cli_out("Error setting port %s default VLAN to %d: %s\n",
                    SOC_PORT_NAME(u, port), vid, bcm_errmsg(rv));
            if ((rv == BCM_E_NOT_FOUND) ||
                (rv == BCM_E_CONFIG)) {
                cli_out("VLAN %d must be created and contain the ports "
                        "before being used for port default VLAN.\n", vid);
            }
            break;
        }
    }
    return CMD_OK;
}

#ifdef BCM_MAC2V_SUPPORT
int 
_mem_macvlan_get(int unit, bcm_mac_t mac_addr, 
                uint64 *data0, uint64 *data1)
{
    uint32  reg_val32, count, temp;
    uint64  reg_val64, mac_field;
    soc_control_t   *soc = SOC_CONTROL(unit);
    int rv = SOC_E_NONE;

    COMPILER_64_ZERO(reg_val64);
    COMPILER_64_ZERO(mac_field);

    if (SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit)) {
        MEM_LOCK(unit, INDEX(MAC2VLANm));
        /* 1. set arla_mac */
        if (mac_addr == NULL){
            return SOC_E_PARAM;
        }
        
        SAL_MAC_ADDR_TO_UINT64(mac_addr, mac_field);

        SOC_IF_ERROR_RETURN(soc_ARLA_MACr_field_set
            (unit, (uint32 *)&reg_val64, MAC_ADDR_INDXf, (uint32 *)&mac_field));

	    SOC_IF_ERROR_RETURN(REG_WRITE_ARLA_MACr(unit, (uint32 *)&reg_val64));
        
        /* 2. set arla_rwctl(read), check for command DONE. */
        MEM_RWCTRL_REG_LOCK(soc);
        if ((rv = REG_READ_ARLA_RWCTLr(unit, (uint32 *)&reg_val32)) < 0) {
            MEM_RWCTRL_REG_UNLOCK(soc);
            goto mem_locate_exit;
        }
        temp = 1; /* Read operation */
        soc_ARLA_RWCTLr_field_set(unit, &reg_val32, TAB_RWf, &temp);
        temp = 0x5; /* Access MAC2VLAN table */
        soc_ARLA_RWCTLr_field_set(unit, &reg_val32, TAB_INDEXf, &temp);
        temp = 1;
        soc_ARLA_RWCTLr_field_set(unit, &reg_val32, ARL_STRTDNf, &temp);
        if ((rv = REG_WRITE_ARLA_RWCTLr(unit, (uint32 *)&reg_val32)) < 0) {
            MEM_RWCTRL_REG_UNLOCK(soc);
            goto mem_locate_exit;
        }
    
        /* wait for complete */
        for (count = 0; count < SOC_TIMEOUT_VAL; count++) {
            if ((rv = REG_READ_ARLA_RWCTLr(unit, (uint32 *)&reg_val32)) < 0) {
                MEM_RWCTRL_REG_UNLOCK(soc);
                goto mem_locate_exit;
            }
            soc_ARLA_RWCTLr_field_get(unit, &reg_val32, ARL_STRTDNf, &temp);
            if (!temp) {
                break;
            }
        }
    
        if (count >= SOC_TIMEOUT_VAL) {
            rv = SOC_E_TIMEOUT;
            MEM_RWCTRL_REG_UNLOCK(soc);
            goto mem_locate_exit;
        }
        MEM_RWCTRL_REG_UNLOCK(soc);
    
        /* 3. get othere_table_data0 , othere_table_data1 */
        if ((rv = REG_READ_OTHER_TABLE_DATA0r(unit, (uint32 *)data0)) < 0) {
            goto mem_locate_exit;
        }

        if ((rv = REG_READ_OTHER_TABLE_DATA1r(unit, (uint32 *)data1)) < 0) {
            goto mem_locate_exit;
        }
    
     mem_locate_exit:
        MEM_UNLOCK(unit, INDEX(MAC2VLANm));
        return SOC_E_NONE;
    } else {
        return SOC_E_UNAVAIL;
    }
}
#endif

STATIC int
_robo_vlan_cross_connect_print(int unit,
                               bcm_vlan_t outer_vlan, bcm_vlan_t inner_vlan,
                               bcm_port_t port_1, bcm_port_t port_2, void *user_data)
{
    bcm_port_t lport_1, lport_2;
    bcm_module_t        modid;
    bcm_trunk_t         tgid;
    int                 id;

    if (BCM_GPORT_IS_SET(port_1)) {
        BCM_IF_ERROR_RETURN(
            _bcm_robo_gport_resolve(unit, port_1, &modid, &lport_1, &tgid, &id));
        if ((-1 != tgid) || (-1 != id)){
            return BCM_E_PORT;
        }
    } else {
        if (!SOC_PORT_VALID(unit, port_1)) { 
            return BCM_E_PORT; 
        }
        lport_1 = port_1;
    }

    if (BCM_GPORT_IS_SET(port_2)) {
        BCM_IF_ERROR_RETURN(
            _bcm_robo_gport_resolve(unit, port_2, &modid, &lport_2, &tgid, &id));
        if ((-1 != tgid) || (-1 != id)){
            return BCM_E_PORT;
        }
    } else {
        if (!SOC_PORT_VALID(unit, port_2)) { 
            return BCM_E_PORT; 
        }
        lport_2 = port_2;
    }
    
    cli_out("u = %d, outer vid = %d, inner vid = %d, port1 = %d, port2 = %d \n",
            unit, outer_vlan, inner_vlan, lport_1, lport_2);
    return CMD_OK;
}

#ifdef BCM_V2V_SUPPORT 
STATIC int 
_robo_vlan_translate_print(int unit, bcm_port_t gport, bcm_vlan_t old_vid,
                      bcm_vlan_t new_vid, int prio, void *user_data)
{
    int             rv, id;
    bcm_port_t      port;
    bcm_module_t    modid;
    bcm_trunk_t     trunk;

    rv = _bcm_robo_gport_resolve(unit, gport, &modid, &port, &trunk, &id);
    if (BCM_FAILURE(rv)) {
        cli_out("ERROR: Invalid gport =0x%x\n", gport);
        return CMD_FAIL;
    }
    if (BCM_GPORT_IS_TRUNK(gport)) {
        cli_out("u = %d, tgid = %d, old_vid = %d, new_vid = %d, prio = %d \n",
                unit, trunk, old_vid, new_vid, prio);
    } else {
        cli_out("u = %d, modid = %d port = %d, old_vid = %d, new_vid = %d, prio = %d \n",
                unit, modid, port, old_vid, new_vid, prio);
    }

    return CMD_OK;
}

STATIC int 
_robo_vlan_translate_range_print(int unit, bcm_port_t gport, bcm_vlan_t old_vid_lo,
                            bcm_vlan_t old_vid_hi, bcm_vlan_t new_vid, 
                            int prio, void *user_data)
{
    int             rv, id;
    bcm_port_t      port;
    bcm_module_t    modid;
    bcm_trunk_t     trunk;

    rv = _bcm_robo_gport_resolve(unit, gport, &modid, &port, &trunk, &id);
    if (BCM_FAILURE(rv)) {
        cli_out("ERROR: Invalid gport =0x%x\n", gport);
        return CMD_FAIL;
    }
    if (BCM_GPORT_IS_TRUNK(gport)) {
        cli_out("u = %d, tgid = %d, old_vid_lo = %d, old_vid_hi = %d, new_vid = %d, prio = %d \n",
                unit, trunk, old_vid_lo, old_vid_hi, new_vid, prio);
    } else {
        cli_out("u = %d, modid = %d port = %d, old_vid_lo = %d, old_vid_hi = %d, new_vid = %d, prio = %d \n",
                unit, modid, port, old_vid_lo, old_vid_hi, new_vid, prio);
    }

    return CMD_OK;
}
#endif  /* BCM_V2V_SUPPORT */

STATIC cmd_result_t
_robo_vlan_action_port(int unit, args_t *a)
{
    char       *subcmd;
    int         egress = 0;
 
    if ((subcmd = ARG_GET(a)) == NULL) {
        return CMD_USAGE;
    }
    if (sal_strcasecmp(subcmd, "egress") == 0) {
        if ((subcmd = ARG_GET(a)) == NULL) {
            return CMD_USAGE;
        }
        egress = 1;
    }
        
    if (sal_strcasecmp(subcmd, "default") == 0) {
        if ((subcmd = ARG_GET(a)) == NULL) {
            return CMD_USAGE;
        }
        if (sal_strcasecmp(subcmd, "add") == 0) {
            int                     port;
            parse_table_t           pt;
            cmd_result_t            r;
            bcm_vlan_action_set_t   action;            
            robo_vlan_action_set_t      act;
            
            _robo_vlan_action_set_t_init(&act);
            parse_table_init(unit, &pt);
            parse_table_add(&pt, "Port", PQ_INT, 0, &port, NULL);
            _robo_parse_table_vlan_action_set_add(&pt, &act);
            if (parse_arg_eq(a, &pt) < 0) {
                cli_out("Error: Invalid option %s\n", ARG_CUR(a));
                parse_arg_eq_done(&pt);
                return CMD_USAGE;
            }
            bcm_vlan_action_set_t_init(&action);
            r = _robo_vlan_to_vlan_action_set(&action, &act);
            parse_arg_eq_done(&pt);
            if (r != CMD_OK) {
                cli_out("Error: invalid action\n");
                return r;
            }
            
            if (egress == 0) {
                ROBO_VLAN_IF_ERROR_RETURN(
                    bcm_vlan_port_default_action_set(unit, port, &action));
            } else {
                ROBO_VLAN_IF_ERROR_RETURN(
                    bcm_vlan_port_egress_default_action_set(unit, port, 
                                                            &action));
            }
            return CMD_OK;

        } else if (sal_strcasecmp(subcmd, "get") == 0) {
            int                     port;
            parse_table_t           pt;
            cmd_result_t            r;
            bcm_vlan_action_set_t   action;            

            parse_table_init(unit, &pt);
            parse_table_add(&pt, "Port", PQ_INT, 0, &port, NULL);
            if (!parseEndOk( a, &pt, &r)) {
                return r;
            }

            if (egress == 0) {
                ROBO_VLAN_IF_ERROR_RETURN(
                    bcm_vlan_port_default_action_get(unit, port, &action));
            } else {
                ROBO_VLAN_IF_ERROR_RETURN(
                    bcm_vlan_port_egress_default_action_get(unit, port, 
                                                            &action));
            }
            cli_out("Port %d: New OVLAN=%d, New IVLAN=%d, New Prio=%d\n",
                    port, action.new_outer_vlan, action.new_inner_vlan,
                    action.priority);
            _robo_vlan_action_set_print(&action);
            return CMD_OK;
        } else if (sal_strcasecmp(subcmd, "delete") == 0) {
            int                     port;
            parse_table_t           pt;
            cmd_result_t            r;

            parse_table_init(unit, &pt);
            parse_table_add(&pt, "Port", PQ_INT, 0, &port, NULL);
            if (!parseEndOk( a, &pt, &r)) {
                return r;
            }

            if (egress == 0) {
                ROBO_VLAN_IF_ERROR_RETURN(
                    bcm_vlan_port_default_action_delete(unit, port));
    } else {
                ROBO_VLAN_IF_ERROR_RETURN(
                    bcm_vlan_port_egress_default_action_delete(unit, port));
    }

    return CMD_OK;
        }
    } 

    return CMD_USAGE;
}

STATIC cmd_result_t
_robo_vlan_policer(int unit, args_t *a)
{
    char       *subcmd;
    parse_table_t           pt;
    bcm_policer_config_t    pol_cfg;
    bcm_policer_t           polid = -1;
    cmd_result_t            retCode;
    bcm_port_t          port;
    bcm_vlan_t          vlan = 0;
    int             vlanid = 0;
    int                     rv = BCM_E_NONE;
    
    
    if ((subcmd = ARG_GET(a)) == NULL) {
        return CMD_USAGE;
    }

    if (sal_strcasecmp(subcmd, "create") == 0) {
        sal_memset(&pol_cfg, 0, sizeof(bcm_policer_config_t));

        /* Parse command option arguments */
        parse_table_init(unit, &pt);
        parse_table_add(&pt, "PolId", PQ_DFL|PQ_INT, 0, &polid, NULL);
        parse_table_add(&pt, "cbs", PQ_DFL|PQ_INT, 0, 
                        &pol_cfg.ckbits_burst, NULL);
        parse_table_add(&pt, "cir", PQ_DFL|PQ_INT, 0, 
                        &pol_cfg.ckbits_sec, NULL);
        if (!parseEndOk(a, &pt, &retCode)) {
            return retCode;
        }

        /* Fix the policer mode */
        pol_cfg.mode = bcmPolicerModeCommitted;

        if (polid >= 0) {
            pol_cfg.flags |= (BCM_POLICER_WITH_ID | BCM_POLICER_REPLACE);
        }

        if ((rv = bcm_policer_create(unit, &pol_cfg, &polid)) != BCM_E_NONE) {
            cli_out("Policer add failed. (%s) \n", bcm_errmsg(rv));
            return CMD_FAIL;
        }

        if (!(pol_cfg.flags & BCM_POLICER_WITH_ID)) {
            cli_out("Policer created with id: %d \n", polid);
        }

        return CMD_OK;

    } else if (sal_strcasecmp(subcmd, "destory") == 0) {
        subcmd = ARG_CUR(a);
        if (!subcmd) {
            return CMD_USAGE;
        }

        if (0 == sal_strncasecmp(subcmd, "all", 3)) {
            subcmd = ARG_GET(a);
            if ((rv = bcm_policer_destroy_all(unit)) != BCM_E_NONE) {
                cli_out("ERROR: bcm_policer_destroy_all(unit=%d) failed.(%s) \n",
                        unit, bcm_errmsg(rv));
                return CMD_FAIL;
            }
        } else {
            /* Parse command option arguments */
            parse_table_init(unit, &pt);
            parse_table_add(&pt, "PolId", PQ_DFL|PQ_INT, 0, &polid, NULL);
            if (!parseEndOk(a, &pt, &retCode)) {
                return retCode;
            }
            if (polid < 0) {
                cli_out("Invalid policer id specified\n");
                return CMD_FAIL;
            }
            if ((rv = bcm_policer_destroy(unit, polid)) != BCM_E_NONE) {
                cli_out("ERROR: bcm_policer_destroy(unit=%d, id=%d) failed (%s) \n",
                        unit, polid, bcm_errmsg(rv));
                return CMD_FAIL;
            }
        }
    } else if (sal_strcasecmp(subcmd, "attach") == 0) {
        subcmd = ARG_CUR(a);
        if (!subcmd) {
            return CMD_USAGE;
        }

        /* Parse command option arguments */
        parse_table_init(unit, &pt);
        parse_table_add(&pt, "PolId", PQ_DFL|PQ_INT, 0, &polid, NULL);
        parse_table_add(&pt, "Port", PQ_DFL|PQ_INT, 0, &port, NULL);
        parse_table_add(&pt, "Vlan", PQ_DFL|PQ_INT, 0, &vlanid, NULL);
        if (!parseEndOk(a, &pt, &retCode)) {
            return retCode;
        }
        if (polid < 0) {
            cli_out("Invalid policer id specified\n");
            return CMD_FAIL;
        }

        vlan = vlanid;
        if ((rv = bcm_vlan_port_policer_set(unit, vlan, port, polid)) != BCM_E_NONE) {
            cli_out(\
                    "ERROR: bcm_vlan_port_policer_set(unit=%d, vlan=%d, port=%d,\
                    polid=%d) failed (%s) \n", unit, vlan, port, polid, bcm_errmsg(rv));
            return CMD_FAIL;
        }
        return CMD_OK;
    } else if (sal_strcasecmp(subcmd, "detach") == 0) {
        subcmd = ARG_CUR(a);
        if (!subcmd) {
            return CMD_USAGE;
        }

        /* Parse command option arguments */
        parse_table_init(unit, &pt);
        parse_table_add(&pt, "Port", PQ_DFL|PQ_INT, 0, &port, NULL);
        parse_table_add(&pt, "Vlan", PQ_DFL|PQ_INT, 0, &vlanid, NULL);
        if (!parseEndOk(a, &pt, &retCode)) {
            return retCode;
        }

        vlan = vlanid;
        if ((rv = bcm_vlan_port_policer_set(unit, vlan, port, 0)) != BCM_E_NONE) {
            cli_out(\
                    "ERROR: bcm_vlan_port_policer_set(unit=%d, vlan=%d, port=%d,\
                    polid=%d) failed (%s) \n", unit, vlan, port, polid, bcm_errmsg(rv));
            return CMD_FAIL;
        }
        return CMD_OK;
    }
    
    return CMD_USAGE;
}

STATIC cmd_result_t
_robo_vlan_dlf(int unit, args_t *a)
{
    char       *subcmd;
    parse_table_t           pt;
    cmd_result_t            retCode;
    int                     rv = BCM_E_NONE;
#define DLF_ID_NOT_ASSIGNED -1
    int BcastId = DLF_ID_NOT_ASSIGNED;
    int MlfId = DLF_ID_NOT_ASSIGNED;
    int UlfId = DLF_ID_NOT_ASSIGNED;
    int vlanid = 0;
    bcm_vlan_control_vlan_t vlan_control;


    if ((subcmd = ARG_GET(a)) == NULL) {
        return CMD_USAGE;
    }

    if (SOC_IS_TB_AX(unit)) {
        rv = BCM_E_UNAVAIL;
        goto dlf_err;
    }

    if (sal_strcasecmp(subcmd, "set") == 0) {

        /* Parse command option arguments */
        parse_table_init(unit, &pt);
        parse_table_add(&pt, "Vlan", PQ_DFL|PQ_INT, 0, &vlanid, NULL);
        parse_table_add(&pt, "Bcast", PQ_DFL|PQ_INT,
                    INT_TO_PTR(BcastId), &BcastId, 0);
        parse_table_add(&pt, "Mlf",   PQ_DFL|PQ_INT,
                    INT_TO_PTR(MlfId), &MlfId, 0);
        parse_table_add(&pt, "Ulf", PQ_DFL|PQ_INT,
                    INT_TO_PTR(UlfId), &UlfId, 0);
        if (!parseEndOk(a, &pt, &retCode)) {
            return retCode;
        }

        if (BcastId != DLF_ID_NOT_ASSIGNED) {
            _BCM_MULTICAST_GROUP_SET(
                    BcastId, _BCM_MULTICAST_TYPE_L2, BcastId);
        }
        if (MlfId != DLF_ID_NOT_ASSIGNED) {
            _BCM_MULTICAST_GROUP_SET(
                    MlfId, _BCM_MULTICAST_TYPE_L2, MlfId);
        }
        if (UlfId != DLF_ID_NOT_ASSIGNED) {
            _BCM_MULTICAST_GROUP_SET(
                    UlfId, _BCM_MULTICAST_TYPE_L2, UlfId);
        }

        sal_memset(&vlan_control, 0, sizeof(vlan_control));
        if (VLAN_ID_VALID(vlanid)) {
            rv = bcm_vlan_control_vlan_get(unit, vlanid, &vlan_control);
            if (rv < 0) {
                goto dlf_err;
            }
            if (BcastId != DLF_ID_NOT_ASSIGNED) {
                vlan_control.broadcast_group = BcastId;
            }
            if (MlfId != DLF_ID_NOT_ASSIGNED) {
                vlan_control.unknown_multicast_group = MlfId;
            }
            if (UlfId != DLF_ID_NOT_ASSIGNED) {
                vlan_control.unknown_unicast_group = UlfId;
            }

            rv = bcm_vlan_control_vlan_set(unit, vlanid, vlan_control);
            if (rv < 0) {
                goto dlf_err;
            }
        } else {
            goto dlf_err;
        }

        return CMD_OK;

    } else if (sal_strcasecmp(subcmd, "delete") == 0) {
        /* Parse command option arguments */
        parse_table_init(unit, &pt);
        parse_table_add(&pt, "Vlan", PQ_DFL|PQ_INT, 0, &vlanid, NULL);
        if (!parseEndOk(a, &pt, &retCode)) {
            return retCode;
        }

        sal_memset(&vlan_control, 0, sizeof(vlan_control));
        if (VLAN_ID_VALID(vlanid)) {
            rv = bcm_vlan_control_vlan_get(unit, vlanid, &vlan_control);
            if (rv < 0) {
                goto dlf_err;
            }
            vlan_control.broadcast_group = 0;
            vlan_control.unknown_multicast_group = 0;
            vlan_control.unknown_unicast_group = 0;

            rv = bcm_vlan_control_vlan_set(unit, vlanid, vlan_control);
            if (rv < 0) {
                goto dlf_err;
            }
        } else {
            goto dlf_err;
        }
        return CMD_OK;

    } else if (sal_strcasecmp(subcmd, "show") == 0) {
        /* Parse command option arguments */
        parse_table_init(unit, &pt);
        parse_table_add(&pt, "Vlan", PQ_DFL|PQ_INT, 0, &vlanid, NULL);
        if (!parseEndOk(a, &pt, &retCode)) {
            return retCode;
        }

        sal_memset(&vlan_control, 0, sizeof(vlan_control));
        if (VLAN_ID_VALID(vlanid)) {
            rv = bcm_vlan_control_vlan_get(unit, vlanid, &vlan_control);
            if (rv < 0) {
                goto dlf_err;
            }

            cli_out("VLAN %d DLF configuration\n",vlanid);
            if (_BCM_MULTICAST_IS_L2(vlan_control.broadcast_group)) {
                cli_out("\tBroadcast: MGID = %d\n",_BCM_MULTICAST_ID_GET(vlan_control.broadcast_group));
            } else {
                cli_out("\tBroadcast: Disabled\n");
            }
            if (_BCM_MULTICAST_IS_L2(vlan_control.unknown_multicast_group)) {
                cli_out("\tMulticast: MGID = %d\n",_BCM_MULTICAST_ID_GET(vlan_control.unknown_multicast_group));
            } else {
                cli_out("\tMulticast: Disabled\n");
            }
            if (_BCM_MULTICAST_IS_L2(vlan_control.unknown_unicast_group)) {
                cli_out("\tUnicast  : MGID = %d\n",_BCM_MULTICAST_ID_GET(vlan_control.unknown_unicast_group));
            } else {
                cli_out("\tUnicast  : Disabled\n");
            }
        } else {
            goto dlf_err;
        }
        return CMD_OK;
    }
    
    return CMD_USAGE;

dlf_err:
    cli_out("%s: ERROR: %s\n", ARG_CMD(a), bcm_errmsg(rv));
    return CMD_FAIL;
}

/*
  * Enabled to test bcm_vlan_gport_xxx() APIs.
  */
#define TEST_GPORT_API 0

cmd_result_t
if_robo_vlan(int unit, args_t *a)
{
    char        *subcmd, *c;
    int         r = 0;
    vlan_id_t       id = VLAN_ID_INVALID;
    pbmp_t      arg_ubmp;
    pbmp_t      arg_pbmp;
    parse_table_t   pt;
    cmd_result_t    ret_code;
    char                *bcm_vlan_mcast_flood_str[] = BCM_VLAN_MCAST_FLOOD_STR;

    BCM_PBMP_CLEAR(arg_ubmp);
    BCM_PBMP_CLEAR(arg_pbmp);

    if (! sh_check_attached(ARG_CMD(a), unit)) {
        return CMD_FAIL;
    }

    if ((subcmd = ARG_GET(a)) == NULL) {
        return CMD_USAGE;
    }

    if (sal_strcasecmp(subcmd, "test") == 0) {
        uint64  test_data0, test_data1;
        int i;
        
        COMPILER_64_ZERO(test_data0);
        cli_out(" Stag 0 -------\n");
        cli_out(" data 0 :\n");
        for(i=0;i<8;i++){
            cli_out("0x%02x ",*((uint8 *)&test_data0+i));
        }
        cli_out("\n data 1 :\n");
        for(i=0;i<8;i++){
            cli_out("0x%02x ",*((uint8 *)&test_data1+i));
        }
        
        cli_out("\n\n Stag 1 -------\n");
        COMPILER_64_SET(test_data0, 0x11223344,0x55667788);
        COMPILER_64_SET(test_data1, COMPILER_64_HI(test_data0),
                COMPILER_64_LO(test_data0));
        cli_out(" data 0 :\n");
        for(i=0;i<8;i++){
            cli_out("0x%02x ",*((uint8 *)&test_data0+i));
        }
        cli_out("\n data 1 :\n");
        for(i=0;i<8;i++){
            cli_out("0x%02x ",*((uint8 *)&test_data1+i));
        }
        
        cli_out("\n\n Stag 2 -------\n");
        COMPILER_64_SHL(test_data0,12);
        COMPILER_64_SHL(test_data1,12);
        cli_out(" data 0 :\n");
        for(i=0;i<8;i++){
            cli_out("0x%02x ",*((uint8 *)&test_data0+i));
        }
        cli_out("\n data 1 :\n");
        for(i=0;i<8;i++){
            cli_out("0x%02x ",*((uint8 *)&test_data1+i));
        }

        return CMD_OK;
    }
    if (sal_strcasecmp(subcmd, "create") == 0) {
        if ((c = ARG_GET(a)) == NULL) {
            return CMD_USAGE;
        }

        id = parse_integer(c);
        parse_table_init(unit, &pt);
        parse_table_add(&pt, "PortBitMap",  PQ_DFL|PQ_PBMP,
                (void *)(0), &arg_pbmp, NULL);
        parse_table_add(&pt, "UntagBitMap",     PQ_DFL|PQ_PBMP,
                (void *)(0), &arg_ubmp, NULL);

        if (parse_arg_eq(a, &pt) < 0 || ARG_CNT(a) > 0) {
            cli_out("%s: ERROR: Unknown option: %s\n",
                    ARG_CMD(a), ARG_CUR(a));
            parse_arg_eq_done(&pt);
            return CMD_FAIL;
        }
        parse_arg_eq_done(&pt);

        if ((r = _bcm_robo_pbmp_check(unit, arg_pbmp)) < 0) {
            goto bcm_err;
        }

        if ((r = _bcm_robo_pbmp_check(unit, arg_ubmp)) < 0) {
            goto bcm_err;
        }

        if ((r = bcm_vlan_create(unit, id)) < 0) {
            goto bcm_err;
        }

       if ((r = bcm_vlan_port_add(unit, id, arg_pbmp, arg_ubmp)) < 0) {
           goto bcm_err;
       }

        return CMD_OK;
    }

    if (sal_strcasecmp(subcmd, "destroy") == 0) {

        if ((c = ARG_GET(a)) == NULL) {
            return CMD_USAGE;
        }

        id = parse_integer(c);

        if ((r = bcm_vlan_destroy(unit, id)) < 0) {
            goto bcm_err;
        }

        return CMD_OK;
    }

    if (sal_strcasecmp(subcmd, "clear") == 0) {

        if ((r = bcm_vlan_destroy_all(unit)) < 0) {
            goto bcm_err;
        }

        return CMD_OK;
    }

    if (sal_strcasecmp(subcmd, "add") == 0 || 
            sal_strcasecmp(subcmd, "remove") == 0) {
        if ((c = ARG_GET(a)) == NULL) {
            return CMD_USAGE;
        }

        id = parse_integer(c);

        parse_table_init(unit, &pt);
        parse_table_add(&pt, "PortBitMap", PQ_DFL|PQ_PBMP,
            (void *)(0), &arg_pbmp, NULL);
        if (sal_strcasecmp(subcmd, "add") == 0) {
            parse_table_add(&pt, "UntagBitMap", PQ_DFL|PQ_PBMP,
                (void *)(0), &arg_ubmp, NULL);
        }

        if (!parseEndOk( a, &pt, &ret_code)) {
            return ret_code;
        }

        if (sal_strcasecmp(subcmd, "remove") == 0) {
#if TEST_GPORT_API
            bcm_gport_t gport;
            bcm_port_t port;

            if (BCM_PBMP_EQ(arg_pbmp, PBMP_ALL(unit))) {
                if ((r = bcm_vlan_gport_delete_all(unit, id)) < 0) {
                   goto bcm_err;
                }
            } else {
                PBMP_ITER(arg_pbmp, port) {
                    if ((r = bcm_port_gport_get(unit, port, &gport)) < 0) {
                       goto bcm_err;
                    }
                    if ((r = bcm_vlan_gport_delete(unit, id, gport)) < 0) {
                       goto bcm_err;
                    }
                }
            }
#else
            if ((r = bcm_vlan_port_remove(unit, id, arg_pbmp)) < 0) {
                goto bcm_err;
            }
#endif
        } else {
#if TEST_GPORT_API
            bcm_gport_t gport;
            bcm_port_t port;

            PBMP_ITER(arg_pbmp, port) {
                if ((r = bcm_port_gport_get(unit, port, &gport)) < 0) {
                   goto bcm_err;
                }
                if ((r = bcm_vlan_gport_add(unit, id, gport, 0)) < 0) {
                   goto bcm_err;
                }
            }
            PBMP_ITER(arg_ubmp, port) {
                if ((r = bcm_port_gport_get(unit, port, &gport)) < 0) {
                   goto bcm_err;
                }
                if ((r = bcm_vlan_gport_add(unit, id, gport, 1)) < 0) {
                   goto bcm_err;
                }
            }
#else
            if ((r = bcm_vlan_port_add(unit, id, arg_pbmp, arg_ubmp)) < 0) {
               goto bcm_err;
           }
#endif
        }

        return CMD_OK;
    }

    if (sal_strcasecmp(subcmd, "MulticastFlood") == 0) {
        bcm_vlan_mcast_flood_t  mode;
        if ((c = ARG_GET(a)) == NULL) {
            return CMD_USAGE;
        }

        id = parse_integer(c);
        if ((c = ARG_GET(a)) == NULL) {
            if ((r = bcm_vlan_mcast_flood_get(unit, id, &mode)) < 0) {
                goto bcm_err;
            }
            cli_out("vlan %d Multicast Flood Mode is %s\n",
                    id, bcm_vlan_mcast_flood_str[mode]);
            return CMD_OK;
        }
        mode = parse_integer(c);
        if ((r = bcm_vlan_mcast_flood_set(unit, id, mode)) < 0) {
            goto bcm_err;
        }
        return CMD_OK;
    }

    if (sal_strcasecmp(subcmd, "show") == 0) {

        bcm_vlan_data_t *list;
        int count, i;
        char pfmtp[SOC_PBMP_FMT_LEN],
             pfmtu[SOC_PBMP_FMT_LEN];

        if ((c = ARG_GET(a)) != NULL) {
            id = parse_integer(c);
        }

        cli_out("%-8s%-10s  %-10s\n",
                "ID", "Ports", "Untagged");

        if ((r = bcm_vlan_list(unit, &list, &count)) < 0) {
            goto bcm_err;
        }

        for (i = 0; i < count; i++) {
            if (id == VLAN_ID_INVALID || list[i].vlan_tag == id) {
                cli_out("%-8d%s  %s\n",
                        list[i].vlan_tag,
                        SOC_PBMP_FMT(list[i].port_bitmap, pfmtp),
                        SOC_PBMP_FMT(list[i].ut_port_bitmap, pfmtu));
            }
        }
        bcm_vlan_list_destroy(unit, list, count);

        return CMD_OK;
    }

#if TEST_GPORT_API
    if (sal_strcasecmp(subcmd, "dump") == 0) {
        pbmp_t dump_pbmp;
        bcm_port_t dump_port;
        bcm_gport_t dump_gport;
        int untagged = 0;
        
        int rv = BCM_E_NONE;
        if ((c = ARG_GET(a)) == NULL) {
            return CMD_USAGE;
        }

        id = parse_integer(c);

        cli_out("ID %-8d member ports:\n", id);
        BCM_PBMP_ASSIGN(dump_pbmp, PBMP_ALL(unit));

        PBMP_ITER(dump_pbmp, dump_port) {
            rv = bcm_port_gport_get(unit, dump_port, &dump_gport);
            if (!rv) {
                cli_out("Get gport id form port %2d failed (%s).\n", dump_port, bcm_errmsg(rv));
            }

            rv = bcm_vlan_gport_get(unit, id, dump_gport, &untagged);
            if (rv == BCM_E_NONE) {
                cli_out("Port %2d %s\n", dump_port, (untagged) ? "(untagged)" : "");
            } else if (rv != BCM_E_NOT_FOUND) {
                cli_out("Dump port %2d failed (%s).\n", dump_port, bcm_errmsg(rv));
            }
        }

        return CMD_OK;
    }
#endif

    if (sal_strcasecmp(subcmd, "default") == 0) {

        if ((c = ARG_GET(a)) != NULL) {
            id = parse_integer(c);
        }

        if (id == VLAN_ID_INVALID) {
            if ((r = bcm_vlan_default_get(unit, &id)) < 0) {
                goto bcm_err;
            }

            cli_out("Default VLAN ID is %d\n", id);
        } else {
            if ((r = bcm_vlan_default_set(unit, id)) < 0) {
                goto bcm_err;
            }

            cli_out("Default VLAN ID set to %d\n", id);
        }

        return CMD_OK;
    }

    /* Vlan control */
    if (sal_strcasecmp(subcmd, "control") == 0 ||
    sal_strcasecmp(subcmd, "ctrl") == 0) {
    char    *value, *tname;
    int ttype, i, varg, matched;

    static struct {         /* match enum from bcm/vlan.h */
        int     type;
        char    *name;
    } typenames[] = {
        { bcmVlanDropUnknown,   "dropunknown" },
        { bcmVlanPreferIP4,     "preferip4" },
        { bcmVlanPreferMAC,     "prefermac" },
        { bcmVlanShared,        "shared" },
        { bcmVlanSharedID,      "sharedid" },
        { bcmVlanTranslate,      "translate" },
            { bcmVlanIgnorePktTag,      "ignorepkttag" },
            { bcmVlanPreferEgressTranslate,      "preferegresstranslate" },
            { bcmVlanPerPortTranslate,      "perporttranslate" },
        { bcmVlanIndependentStp,      "independentstp" },
        { bcmVlanTranslateMode,      "translatemode" },
        { bcmVlanBypassIgmpMld,      "bypassigmpmld" },
        { bcmVlanBypassArpDhcp,      "bypassarpdhcp" },
        { bcmVlanBypassMiim,      "bypassmiim" },
        { bcmVlanBypassMcast,      "bypassmcast" },
        { bcmVlanBypassRsvdMcast,      "bypassrsvdmcast" },
        { bcmVlanBypassL2UserAddr,      "bypassl2useraddr" },
        { bcmVlanUnknownLearn,      "unknownlearn" },
        { bcmVlanUnknownToCpu,      "unknowntocpu" },
        { bcmVlanMemberMismatchLearn,      "membermismatchlearn" },
        { bcmVlanMemberMismatchToCpu,      "membermismatchtocpu" },
        { 0,            NULL }      /* LAST ENTRY */
    };

    subcmd = ARG_GET(a);
    value = ARG_GET(a);

    matched = 0;

    for (i = 0; typenames[i].name != NULL; i++) {
        tname = typenames[i].name;
        if (subcmd == NULL || sal_strcasecmp(subcmd, tname) == 0) {
        matched += 1;
        ttype = typenames[i].type;
        if (value == NULL) {
            r = bcm_vlan_control_get(unit, ttype, &varg);
            if (r < 0) {
            cli_out("%-20s-\t%s\n", tname, bcm_errmsg(r));
            } else {
            cli_out("%-20s%d\n", tname, varg);
            }
        } else {
            varg = parse_integer(value);
            r = bcm_vlan_control_set(unit, ttype, varg);
            if (r < 0) {
            cli_out("%s\tERROR: %s\n", tname, bcm_errmsg(r));
            }
        }
        }
    }

    if (matched == 0) {
        cli_out("%s: ERROR: Unknown control name\n", subcmd);
        return CMD_FAIL;
    }

    return CMD_OK;
    }
    
#ifdef BCM_PROTOCOL2V_SUPPORT
    /* Protocol vlan selection */
    if (sal_strcasecmp(subcmd, "protocol") == 0 ||
    sal_strcasecmp(subcmd, "proto") == 0) {
        bcm_port_t  port;
        int     en_act= 0;

    subcmd = ARG_GET(a);
    if (subcmd == NULL) {
        cli_out("%s: ERROR: missing protocol subcommand\n", ARG_CMD(a));
        return CMD_FAIL;
    }

        if (sal_strcasecmp(subcmd, "enable") == 0 ) {
            en_act = 1;
        } else if (sal_strcasecmp(subcmd, "disable") == 0 ) {
            en_act = 0;
        } else {
            en_act = -1;
        }
        if (en_act != -1){
            bcm_pbmp_t  pbmp;
            
            port = 0 ;            
            port = ~port;
            if ((r = DRV_VLAN_PROP_PORT_ENABLE_GET
                            (unit, DRV_VLAN_PROP_PROTOCOL2V_PORT, port,  
                            (uint32 *)&pbmp)) < 0) {
                goto bcm_err;
            }
            BCM_PBMP_CLEAR(arg_pbmp);
            BCM_PBMP_OR(arg_pbmp, pbmp);
            
            parse_table_init(unit, &pt);
            parse_table_add(&pt, "PortBitMap", PQ_DFL|PQ_PBMP,
                        (void *)(0), &arg_pbmp, NULL);

            if (!parseEndOk( a, &pt, &ret_code)) {
                return ret_code;
            }

            if ((r = DRV_VLAN_PROP_PORT_ENABLE_SET
                            (unit, DRV_VLAN_PROP_PROTOCOL2V_PORT, arg_pbmp,
                             (en_act ? TRUE :FALSE))) < 0) {
                goto bcm_err;
            }
            
            return CMD_OK;
        }

    if (sal_strcasecmp(subcmd, "add") == 0) {
        bcm_pbmp_t  pbmp;
        bcm_port_t  port;
        int     frame, ether, vlan, prio;

        parse_table_init(unit, &pt);
        parse_table_add(&pt, "PortBitMap", PQ_PBMP|PQ_BCM, 0, &pbmp, NULL);
        parse_table_add(&pt, "Frame", PQ_INT, 0, &frame, NULL);
        parse_table_add(&pt, "Ether", PQ_HEX, 0, &ether, NULL);
        parse_table_add(&pt, "VLan", PQ_INT, 0, &vlan, NULL);
        parse_table_add(&pt, "Prio", PQ_INT, 0, &prio, NULL);
        if (!parseEndOk( a, &pt, &ret_code)) {
        return ret_code;
        }

            /* port and frame are not used in BCM53242 */
            if (SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit)) {
                BCM_PBMP_CLEAR(pbmp);
                BCM_PBMP_PORT_ADD(pbmp, 0);
                frame = 0;
            }

           PBMP_ITER(pbmp, port) {
                if ((r = bcm_port_protocol_vlan_add(unit, port, frame,
                            ether, vlan)) < 0) {
                    goto bcm_err;
                }
        }
        return CMD_OK;
    }

    if (sal_strcasecmp(subcmd, "delete") == 0) {
            bcm_pbmp_t  pbmp;
            bcm_port_t  port;
            int     frame, ether;
            
            parse_table_init(unit, &pt);
            parse_table_add(&pt, "PortBitMap", PQ_PBMP|PQ_BCM, 0, &pbmp, NULL);
            parse_table_add(&pt, "Frame", PQ_INT, 0, &frame, NULL);
            parse_table_add(&pt, "Ether", PQ_HEX, 0, &ether, NULL);
            if (!parseEndOk( a, &pt, &ret_code)) {
                return ret_code;
            }

            /* port and frame are not used in BCM53242 */
            if (SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit)) {
                BCM_PBMP_CLEAR(pbmp);
                BCM_PBMP_PORT_ADD(pbmp, 0);
                frame = 0;
            }

            PBMP_ITER(pbmp, port) {
                if ((r = bcm_port_protocol_vlan_delete(unit, port, frame,
                                   ether)) < 0) {
                    goto bcm_err;
                }
            }
            return CMD_OK;
    }

    if (sal_strcasecmp(subcmd, "clear") == 0) {
           bcm_pbmp_t   pbmp;
           bcm_port_t   port;
           
           parse_table_init(unit, &pt);
           parse_table_add(&pt, "PortBitMap", PQ_PBMP|PQ_BCM, 0, &pbmp, NULL);
           if (!parseEndOk( a, &pt, &ret_code)) {
               cli_out("Usages: vlan protocol clear PortBitMap=<pbmp>\n");
               return ret_code;
           }

            /* port and frame are not used in BCM53242 */
            if (SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit)) {
                BCM_PBMP_CLEAR(pbmp);
                BCM_PBMP_PORT_ADD(pbmp, 0);
            }

            PBMP_ITER(pbmp, port) {
                if ((r = bcm_port_protocol_vlan_delete_all(unit,
                                   port)) < 0) {
                    goto bcm_err;
                }
            }
            return CMD_OK;
    }

    cli_out("%s: ERROR: unknown protocol subcommand: %s\n",
            ARG_CMD(a), subcmd);

    return CMD_FAIL;
    }
#endif

#ifdef BCM_MAC2V_SUPPORT
    /* MAC address vlan selection */
    if (sal_strcasecmp(subcmd, "mac") == 0) {

        bcm_port_t  port;
        int     en_act= 0;
        
        subcmd = ARG_GET(a);
        if (subcmd == NULL) {
            cli_out("%s: ERROR: missing mac subcommand\n", ARG_CMD(a));
            return CMD_FAIL;
        }

        if (sal_strcasecmp(subcmd, "enable") == 0 ) {
            en_act = 1;
        } else if (sal_strcasecmp(subcmd, "disable") == 0 ) {
            en_act = 0;
        } else {
            en_act = -1;
        }
        if (en_act != -1){
            
            parse_table_init(unit, &pt);
            parse_table_add(&pt, "PortBitMap", PQ_DFL|PQ_PBMP,
                        (void *)(0), &arg_pbmp, NULL);

            if (!parseEndOk( a, &pt, &ret_code)) {
                return ret_code;
            }

            if (BCM_PBMP_EQ(arg_pbmp, PBMP_ALL(unit))){
                if ((r = bcm_vlan_control_set(unit, bcmVlanPreferMAC, 
                                    (en_act) ? TRUE : FALSE)) < 0){
                    goto bcm_err;
                }
            } else {

                PBMP_ITER(arg_pbmp, port) {
                    if ((r = bcm_vlan_control_port_set(unit, port, 
                                        bcmVlanPortPreferMAC, 
                                        (en_act) ? TRUE : FALSE))< 0) {
                        goto bcm_err;
                    }
                }
            }

            return CMD_OK;
        }

        if (sal_strcasecmp(subcmd, "add") == 0) {
            bcm_mac_t   mac;
            int     vlan, prio;

            parse_table_init(unit, &pt);
            parse_table_add(&pt, "MACaddress", PQ_MAC, 0, &mac, NULL);
            parse_table_add(&pt, "VLan", PQ_INT, 0, &vlan, NULL);
            parse_table_add(&pt, "Prio", PQ_INT, 0, &prio, NULL);

            if (!parseEndOk( a, &pt, &ret_code)) {
                return ret_code;
            }

            if ((r = bcm_vlan_mac_add(unit, mac, vlan, prio)) < 0) {
                goto bcm_err;
            }

            return CMD_OK;
        }

        if (sal_strcasecmp(subcmd, "delete") == 0) {
            bcm_mac_t   mac;

            parse_table_init(unit, &pt);
            parse_table_add(&pt, "MACaddress", PQ_MAC, 0, &mac, NULL);

            if (!parseEndOk( a, &pt, &ret_code)) {
                return ret_code;
            }

            if ((r = bcm_vlan_mac_delete(unit, mac)) < 0) {
                goto bcm_err;
            }

            return CMD_OK;
        }

        if (sal_strcasecmp(subcmd, "clear") == 0) {
            if ((r = bcm_vlan_mac_delete_all(unit)) < 0) {
                goto bcm_err;
            }

            return CMD_OK;
        }

        if (sal_strcasecmp(subcmd, "show") == 0) {
            bcm_mac_t   mac, loc_mac_addr;
            mac2vlan_entry_t    mv_entry0, mv_entry1;
            uint32      loc_valid;
            uint64      mac_field;
            int         found, check_next, loc_prio;
            int   loc_vid = 0;
            
            

            parse_table_init(unit, &pt);
            parse_table_add(&pt, "MACaddress", PQ_MAC, 0, &mac, NULL);

            if (!parseEndOk( a, &pt, &ret_code)) {
                return ret_code;
            }


            sal_memset(&mv_entry0, 0, sizeof(mac2vlan_entry_t));
            sal_memset(&mv_entry1, 0, sizeof(mac2vlan_entry_t));
            if ((r = _mem_macvlan_get(unit, mac,
                    (uint64 *)&mv_entry0, (uint64 *)&mv_entry1)) < 0) {
                goto bcm_err;
            }

            check_next = 0;
            found = 0;
            /* get the data0 vlaid bit */
            if ((r =DRV_MEM_FIELD_GET
                            (unit, DRV_MEM_MACVLAN, DRV_MEM_FIELD_VALID,
                            (uint32 *)&mv_entry0, (uint32 *)&loc_valid)) < 0) {
                goto bcm_err;
            }
                            
            if (loc_valid){
                if ((r =DRV_MEM_FIELD_GET
                                (unit, DRV_MEM_MACVLAN, DRV_MEM_FIELD_MAC,
                                (uint32 *)&mv_entry0, (uint32 *)&mac_field)) < 0) {
                    goto bcm_err;
                }
                SAL_MAC_ADDR_FROM_UINT64(loc_mac_addr, mac_field);
                if (!SAL_MAC_ADDR_CMP(loc_mac_addr, mac)){
                    if ((r =DRV_MEM_FIELD_GET
                                    (unit, DRV_MEM_MACVLAN, DRV_MEM_FIELD_NEW_VLANID,
                                    (uint32 *)&mv_entry0, (uint32 *)&loc_vid)) < 0) {
                        goto bcm_err;
                    }
                    if ((r =DRV_MEM_FIELD_GET
                                    (unit, DRV_MEM_MACVLAN, DRV_MEM_FIELD_PRIORITY,
                                    (uint32 *)&mv_entry0, (uint32 *)&loc_prio)) < 0) {
                        goto bcm_err;
                    }

                    cli_out("mac=%02x:%02x:%02x:%02x:%02x:%02x vlan=%d priority=%d\n",
                            loc_mac_addr[0], loc_mac_addr[1], loc_mac_addr[2], 
                            loc_mac_addr[3], loc_mac_addr[4], loc_mac_addr[5], 
                            loc_vid, loc_prio);
                    found = 1;
                    
                } else {
                    /* assign to bin1 first for not found case */
                    check_next = 1;     
                }
            }

            if (check_next) {
                /* get the data1 vlaid bit */
                if ((r =DRV_MEM_FIELD_GET
                                (unit, DRV_MEM_MACVLAN, DRV_MEM_FIELD_VALID,
                                (uint32 *)&mv_entry1, (uint32 *)&loc_valid)) < 0) {
                    goto bcm_err;
                }
                                
                if (loc_valid){
                    if ((r =DRV_MEM_FIELD_GET
                                    (unit, DRV_MEM_MACVLAN, DRV_MEM_FIELD_MAC,
                                    (uint32 *)&mv_entry1, (uint32 *)&mac_field)) < 0) {
                        goto bcm_err;
                    }
                    SAL_MAC_ADDR_FROM_UINT64(loc_mac_addr, mac_field);
                    if (!SAL_MAC_ADDR_CMP(loc_mac_addr, mac)){
                        if ((r =DRV_MEM_FIELD_GET
                                        (unit, DRV_MEM_MACVLAN, DRV_MEM_FIELD_NEW_VLANID,
                                        (uint32 *)&mv_entry1, (uint32 *)&loc_vid)) < 0) {
                            goto bcm_err;
                        }
                        if ((r =DRV_MEM_FIELD_GET
                                        (unit, DRV_MEM_MACVLAN, DRV_MEM_FIELD_PRIORITY,
                                        (uint32 *)&mv_entry1, (uint32 *)&loc_prio)) < 0) {
                            goto bcm_err;
                        }
    
                        cli_out("mac=%02x:%02x:%02x:%02x:%02x:%02x vlan=%d priority=%d\n",
                                loc_mac_addr[0], loc_mac_addr[1], loc_mac_addr[2], 
                                loc_mac_addr[3], loc_mac_addr[4], loc_mac_addr[5], 
                                loc_vid, loc_prio);
                        found = 1;
                    }
                }

            }

            if (!found) {
                cli_out("mac=%02x:%02x:%02x:%02x:%02x:%02x not exist in MAC2VLAN table.\n",
                        mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
            }
            return CMD_OK;
        }

        cli_out("%s: ERROR: Unknown MAC subcommand: %s\n", ARG_CMD(a), subcmd);

        return CMD_FAIL;
    }
#endif

#ifdef  BCM_V2V_SUPPORT
    /* VLAN translate selection */
    if (sal_strcasecmp(subcmd, "translate") == 0) {
        int arg_vtmode;
        int     en_act= 0;
        bcm_port_t  port;

        subcmd = ARG_GET(a);
        
        if (subcmd == NULL) {
            cli_out("%s: ERROR: Missing translate subcommand\n", ARG_CMD(a));
            return CMD_USAGE;
        }

        if (sal_strcasecmp(subcmd, "enable") == 0 ) {
            en_act = 1;
        } else if (sal_strcasecmp(subcmd, "disable") == 0 ) {
            en_act = 0;
        } else {
            en_act = -1;
        }
        if (en_act != -1){
            bcm_pbmp_t  pbmp;
            
            port = 0 ;            
            port = ~port;
            BCM_IF_ERROR_RETURN(
                       DRV_VLAN_PROP_PORT_ENABLE_GET
                            (unit, DRV_VLAN_PROP_V2V_PORT, port,  
                            (uint32 *)&pbmp));
            BCM_PBMP_CLEAR(arg_pbmp);
            BCM_PBMP_OR(arg_pbmp, pbmp);
            
            parse_table_init(unit, &pt);
            parse_table_add(&pt, "PortBitMap", PQ_DFL|PQ_PBMP,
                        (void *)(0), &arg_pbmp, NULL);

            if (!parseEndOk( a, &pt, &ret_code)) {
                return ret_code;
            }

            if (BCM_PBMP_EQ(arg_pbmp, PBMP_ALL(unit))){
                if ((r = bcm_vlan_control_set(unit, bcmVlanTranslate, 
                                    (en_act) ? TRUE : FALSE)) < 0){
                    goto bcm_err;
                }
            } else {

                PBMP_ITER(arg_pbmp, port) {
                    if ((r = bcm_vlan_control_port_set(unit, port, 
                                        bcmVlanTranslateIngressEnable, 
                                        (en_act) ? TRUE : FALSE))< 0) {
                        goto bcm_err;
                    }
                }
            }

            return CMD_OK;
        }
        
        if (sal_strcasecmp(subcmd, "On") == 0) {
            arg_vtmode = 1;
        } else if (sal_strcasecmp(subcmd, "Off") == 0){
            arg_vtmode = 0;
        } else {
            arg_vtmode = -1;
        }

        if (arg_vtmode != -1) {
            if ((r = bcm_vlan_control_set(unit, bcmVlanTranslate,
                        (arg_vtmode) ? TRUE : FALSE)) < 0) {
                goto bcm_err;
            }
            return CMD_OK;
        }

        if (sal_strcasecmp(subcmd, "add") == 0) {
            int     port, old_vlan, new_vlan, prio;
            int     dtag = 0;
            static  char *dt_mode[] = {
                            "Flase","True",NULL}; /* double tagged mode */

            /* port and priority is not used in ROBO VLAN translation */
            port = prio = old_vlan = new_vlan = 0;
          
            parse_table_init(unit, &pt);
            parse_table_add(&pt, "Port", PQ_INT, 0, &port,   NULL);
            parse_table_add(&pt, "OldVLan", PQ_INT, 0, &old_vlan, NULL);
            parse_table_add(&pt, "NewVLan", PQ_INT, 0, &new_vlan, NULL);
            parse_table_add(&pt, "DTag", 
                                PQ_DFL|PQ_MULTI, 0, &dtag, dt_mode);

            if (!parseEndOk( a, &pt, &ret_code)) {
                return ret_code;
            }

            if (dtag == 1){ 
                if ((r = bcm_vlan_dtag_add(unit, port,
                                old_vlan, new_vlan, prio)) < 0) {
                    goto bcm_err;
                }
            } else if (dtag == 0){
                if ((r = bcm_vlan_translate_add(unit, port,
                                old_vlan, new_vlan, prio)) < 0) {
                    goto bcm_err;
                }
            }

            return CMD_OK;
        }

        if (sal_strcasecmp(subcmd, "get") == 0) {
            int port, prio = -1;
            int old_vlan;
            bcm_vlan_t new_vlan;

            parse_table_init(unit, &pt);
            parse_table_add(&pt, "Port", PQ_INT, 0, &port, NULL);
            parse_table_add(&pt, "OldVLan", PQ_INT, 0, &old_vlan, NULL);

            if (!parseEndOk( a, &pt, &ret_code)) {
                return ret_code;
            }

            if (!BCM_VLAN_VALID(old_vlan)) {
                goto bcm_err;
            }
            if ((r = bcm_vlan_translate_get(unit, port,
                                            (bcm_vlan_t)old_vlan, 
                                            &new_vlan, 
                                            &prio)) < 0) {
                goto bcm_err;
            }

            /* The priority key is ignored in ROBO vlan translation */
            cli_out("New Vlan ID = %d, Priority = %d\n", new_vlan, prio);
            return CMD_OK;
        }
    
        if (sal_strcasecmp(subcmd, "show") == 0) {
            
            parse_table_init(unit, &pt);
            
            if ((r = bcm_vlan_translate_traverse(unit, _robo_vlan_translate_print, 
                                                 NULL)) < 0) {
                goto bcm_err;
            }
            return CMD_OK;
        }
    
        if (sal_strcasecmp(subcmd, "delete") == 0) {
            int     port, old_vlan;

            /* port and priority is not used in ROBO VLAN translation */
            port = 0;
          
            parse_table_init(unit, &pt);
            parse_table_add(&pt, "Port", PQ_INT, 0, &port,   NULL);
            parse_table_add(&pt, "OldVLan", PQ_INT, 0, &old_vlan, NULL);

            if (!parseEndOk( a, &pt, &ret_code)) {
                return ret_code;
            }

            if ((r = bcm_vlan_translate_delete(unit, port, old_vlan)) < 0) {
                goto bcm_err;
            }

            return CMD_OK;
        }

        if (sal_strcasecmp(subcmd, "clear") == 0) {
            if ((r = bcm_vlan_translate_delete_all(unit)) < 0) {
                goto bcm_err;
            }

            return CMD_OK;
        }

        if (sal_strcasecmp(subcmd, "egress") == 0) {
            char * subsubcmd = ARG_GET(a);
            if (subsubcmd == NULL) {
                cli_out("%s: ERROR: Missing translate egress subcommand\n",
                        ARG_CMD(a));
                return CMD_FAIL;
            }

            if (sal_strcasecmp(subsubcmd, "add") == 0) {
                int port, old_vlan, new_vlan, prio, cng;

                parse_table_init(unit, &pt);
                parse_table_add(&pt, "Port", PQ_INT, 0, &port, NULL);
                parse_table_add(&pt, "OldVLan", PQ_INT, 0, &old_vlan, NULL);
                parse_table_add(&pt, "NewVLan", PQ_INT, 0, &new_vlan, NULL);
                parse_table_add(&pt, "Prio", PQ_INT, 0, &prio, NULL);
                parse_table_add(&pt, "Cng", PQ_INT, 0, &cng, NULL);

                if (!parseEndOk( a, &pt, &ret_code)) {
                    return ret_code;
                }

                if (cng) {
                    prio |= BCM_PRIO_DROP_FIRST;
                }

                if ((r = bcm_vlan_translate_egress_add(unit, port,
                                            old_vlan, new_vlan, prio)) < 0) {
                    goto bcm_err;
                }

                return CMD_OK;
            }
            if (sal_strcasecmp(subsubcmd, "get") == 0) {
                int port,  prio = -1;
                int old_vlan;
                bcm_vlan_t new_vlan;

                parse_table_init(unit, &pt);
                parse_table_add(&pt, "Port", PQ_INT, 0, &port, NULL);
                parse_table_add(&pt, "OldVLan", PQ_INT, 0, &old_vlan, NULL);

                if (!parseEndOk( a, &pt, &ret_code)) {
                    return ret_code;
                }

                if (!BCM_VLAN_VALID(old_vlan)) {
                    goto bcm_err;
                }
                if ((r = bcm_vlan_translate_egress_get(unit, port,
                                                       (bcm_vlan_t)old_vlan, 
                                                       &new_vlan, 
                                                       &prio)) < 0) {
                    goto bcm_err;
                }

                cli_out("New Vlan ID = %d, Priority = %d\n", new_vlan, prio);
                return CMD_OK;
            }

            if (sal_strcasecmp(subsubcmd, "delete") == 0) {
                int port, old_vlan;

                parse_table_init(unit, &pt);
                parse_table_add(&pt, "Port", PQ_INT, 0, &port, NULL);
                parse_table_add(&pt, "OldVLan", PQ_INT, 0, &old_vlan, NULL);

                if (!parseEndOk( a, &pt, &ret_code)) {
                    return ret_code;
                }

                if ((r = bcm_vlan_translate_egress_delete(unit, port,
                              old_vlan)) < 0) {
                    goto bcm_err;
                }

                return CMD_OK;
            }

            if (sal_strcasecmp(subsubcmd, "clear") == 0) {
                if ((r = bcm_vlan_translate_egress_delete_all(unit)) < 0) {
                    goto bcm_err;
                }

                return CMD_OK;
            }
        }

        if (sal_strcasecmp(subcmd, "dtag") == 0) {
            char * subsubcmd = ARG_GET(a);
            if (subsubcmd == NULL) {
                cli_out("%s: ERROR: Missing translate dtag subcommand\n",
                        ARG_CMD(a));
                return CMD_FAIL;
            }

            if (sal_strcasecmp(subsubcmd, "add") == 0) {
                int port, old_vlan, new_vlan, prio, cng;

                parse_table_init(unit, &pt);
                parse_table_add(&pt, "Port", PQ_INT, 0, &port, NULL);
                parse_table_add(&pt, "OldVLan", PQ_INT, 0, &old_vlan, NULL);
                parse_table_add(&pt, "NewVLan", PQ_INT, 0, &new_vlan, NULL);
                parse_table_add(&pt, "Prio", PQ_INT, 0, &prio, NULL);
                parse_table_add(&pt, "Cng", PQ_INT, 0, &cng, NULL);

                if (!parseEndOk( a, &pt, &ret_code)) {
                    return ret_code;
                }

                if (cng) {
                    prio |= BCM_PRIO_DROP_FIRST;
                }

                if ((r = bcm_vlan_dtag_add(unit, port,
                                           old_vlan, new_vlan, prio)) < 0) {
                    goto bcm_err;
                }

                return CMD_OK;
            }

            if (sal_strcasecmp(subsubcmd, "get") == 0) {
                int port, prio = -1;
                int old_vlan;
                bcm_vlan_t new_vlan;

                parse_table_init(unit, &pt);
                parse_table_add(&pt, "Port", PQ_INT, 0, &port, NULL);
                parse_table_add(&pt, "OldVLan", PQ_INT, 0, &old_vlan, NULL);

                if (!parseEndOk( a, &pt, &ret_code)) {
                    return ret_code;
                }

                if (!BCM_VLAN_VALID(old_vlan)) {
                    goto bcm_err;
                }
                if ((r = bcm_vlan_dtag_get(unit, port, (bcm_vlan_t)old_vlan, 
                                           &new_vlan, &prio)) < 0) {
                    goto bcm_err;
                }

                cli_out("New Vlan ID = %d, Priority = %d\n", new_vlan, prio);
                return CMD_OK;
            }

            if (sal_strcasecmp(subsubcmd, "show") == 0) {

                parse_table_init(unit, &pt);

                if ((r = bcm_vlan_dtag_traverse(unit, _robo_vlan_translate_print, 
                                                NULL)) < 0) {
                    goto bcm_err;
                }
                return CMD_OK;
            }

            if (sal_strcasecmp(subsubcmd, "delete") == 0) {
                int port, old_vlan;

                parse_table_init(unit, &pt);
                parse_table_add(&pt, "Port", PQ_INT, 0, &port, NULL);
                parse_table_add(&pt, "OldVLan", PQ_INT, 0, &old_vlan, NULL);

                if (!parseEndOk( a, &pt, &ret_code)) {
                    return ret_code;
                }

                if ((r = bcm_vlan_dtag_delete(unit, port, old_vlan)) < 0) {
                    goto bcm_err;
                }

                return CMD_OK;
            }

            if (sal_strcasecmp(subsubcmd, "clear") == 0) {
                if ((r = bcm_vlan_dtag_delete_all(unit)) < 0) {
                    goto bcm_err;
                }

                return CMD_OK;
            }

            if (sal_strcasecmp(subsubcmd, "range") == 0) {
                char * subsubsubcmd = ARG_GET(a);
                if (subsubsubcmd == NULL) {
                    cli_out("%s: ERROR: Missing translate dtag range subcommand\n",
                            ARG_CMD(a));
                    return CMD_FAIL;
                }
    
                if (sal_strcasecmp(subsubsubcmd, "add") == 0) {
                    int     port, old_vlan_lo, old_vlan_hi, new_vlan, prio, cng;
    
                    parse_table_init(unit, &pt);
                    parse_table_add(&pt, "Port", PQ_INT, 0, &port, NULL);
                    parse_table_add(&pt, "OldVLanLo", PQ_INT, 0, &old_vlan_lo, NULL);
                    parse_table_add(&pt, "OldVLanHi", PQ_INT, 0, &old_vlan_hi, NULL);
                    parse_table_add(&pt, "NewVLan", PQ_INT, 0, &new_vlan, NULL);
                    parse_table_add(&pt, "Prio", PQ_INT, 0, &prio, NULL);
                    parse_table_add(&pt, "Cng", PQ_INT, 0, &cng, NULL);

                    if (!parseEndOk( a, &pt, &ret_code)) {
                        return ret_code;
                    }
    
                    if (!BCM_VLAN_VALID(old_vlan_lo) || 
                        !BCM_VLAN_VALID(old_vlan_hi) ||
                        !BCM_VLAN_VALID(new_vlan)) {
                        goto bcm_err;
                    }

                    if (cng) {
                        prio |= BCM_PRIO_DROP_FIRST;
                    }

                    if ((r = bcm_vlan_dtag_range_add(unit, port, old_vlan_lo, 
                                                          old_vlan_hi, new_vlan, 
                                                          prio)) < 0) {
                        goto bcm_err;
                    }
    
                    return CMD_OK;
                }
    
                if (sal_strcasecmp(subsubsubcmd, "get") == 0) {
                    int     port, prio;
                    int     old_vlan_lo, old_vlan_hi;
                    bcm_vlan_t new_vlan;
    
                    parse_table_init(unit, &pt);
                    parse_table_add(&pt, "Port", PQ_INT, 0, &port, NULL);
                    parse_table_add(&pt, "OldVLanLo", PQ_INT, 0, &old_vlan_lo, NULL);
                    parse_table_add(&pt, "OldVLanHi", PQ_INT, 0, &old_vlan_hi, NULL);
    
                    if (!parseEndOk( a, &pt, &ret_code)) {
                        return ret_code;
                    }
    
                    if (!BCM_VLAN_VALID(old_vlan_lo) || 
                        !BCM_VLAN_VALID(old_vlan_hi)) {
                        goto bcm_err;
                    }
                    if ((r = bcm_vlan_dtag_range_get(unit, port, 
                                                          (bcm_vlan_t)old_vlan_lo,
                                                          (bcm_vlan_t)old_vlan_hi,
                                                          &new_vlan, &prio)) < 0) {
                        goto bcm_err;
                    }
    
                    cli_out("New Vlan ID = %d, Priority = %d\n", new_vlan, prio);
                    return CMD_OK;
                }
    
                if (sal_strcasecmp(subsubsubcmd, "show") == 0) {
                    if ((r = bcm_vlan_dtag_range_traverse(unit, 
                                                      _robo_vlan_translate_range_print, 
                                                               NULL)) < 0) {
                        goto bcm_err;
                    }
                    return CMD_OK;
                }
    
                if (sal_strcasecmp(subsubsubcmd, "delete") == 0) {
                    int     port, old_vlan_lo, old_vlan_hi;
    
                    parse_table_init(unit, &pt);
                    parse_table_add(&pt, "Port", PQ_INT, 0, &port, NULL);
                    parse_table_add(&pt, "OldVLanLo", PQ_INT, 0, &old_vlan_lo, NULL);
                    parse_table_add(&pt, "OldVLanHi", PQ_INT, 0, &old_vlan_hi, NULL);
    
                    if (!parseEndOk( a, &pt, &ret_code)) {
                        return ret_code;
                    }
    
                    if (!BCM_VLAN_VALID(old_vlan_lo) || 
                        !BCM_VLAN_VALID(old_vlan_hi)) {
                        goto bcm_err;
                    }
    
                    if ((r = bcm_vlan_dtag_range_delete(unit, port, old_vlan_lo, 
                                                             old_vlan_hi)) < 0) {
                        goto bcm_err;
                    }
    
                    return CMD_OK;
                }
    
                if (sal_strcasecmp(subsubsubcmd, "clear") == 0) {
                    if ((r = bcm_vlan_dtag_range_delete_all(unit)) < 0) {
                        goto bcm_err;
                    }
    
                    return CMD_OK;
                }
            }
            
        }

        if (sal_strcasecmp(subcmd, "range") == 0) {
            char * subsubcmd = ARG_GET(a);
            if (subsubcmd == NULL) {
                cli_out("%s: ERROR: Missing translate range subcommand\n",
                        ARG_CMD(a));
                return CMD_FAIL;
            }

            if (sal_strcasecmp(subsubcmd, "add") == 0) {
                int     port, old_vlan_lo, old_vlan_hi, new_vlan, prio, cng;

                parse_table_init(unit, &pt);
                parse_table_add(&pt, "Port", PQ_INT, 0, &port, NULL);
                parse_table_add(&pt, "OldVLanLo", PQ_INT, 0, &old_vlan_lo, NULL);
                parse_table_add(&pt, "OldVLanHi", PQ_INT, 0, &old_vlan_hi, NULL);
                parse_table_add(&pt, "NewVLan", PQ_INT, 0, &new_vlan, NULL);
                parse_table_add(&pt, "Prio", PQ_INT, 0, &prio, NULL);
                parse_table_add(&pt, "Cng", PQ_INT, 0, &cng, NULL);

                if (!parseEndOk( a, &pt, &ret_code)) {
                    return ret_code;
                }

                if (cng) {
                    prio |= BCM_PRIO_DROP_FIRST;
                }

                if (!BCM_VLAN_VALID(old_vlan_lo) || 
                    !BCM_VLAN_VALID(old_vlan_hi) ||
                    !BCM_VLAN_VALID(new_vlan)) {
                    goto bcm_err;
                }

                if ((r = bcm_vlan_translate_range_add(unit, port, old_vlan_lo, 
                                                      old_vlan_hi, new_vlan, 
                                                      prio)) < 0) {
                    goto bcm_err;
                }

                return CMD_OK;
            }

            if (sal_strcasecmp(subsubcmd, "get") == 0) {
                int     port, prio;
                int     old_vlan_lo, old_vlan_hi;
                bcm_vlan_t new_vlan;

                parse_table_init(unit, &pt);
                parse_table_add(&pt, "Port", PQ_INT, 0, &port, NULL);
                parse_table_add(&pt, "OldVLanLo", PQ_INT, 0, &old_vlan_lo, NULL);
                parse_table_add(&pt, "OldVLanHi", PQ_INT, 0, &old_vlan_hi, NULL);

                if (!parseEndOk( a, &pt, &ret_code)) {
                    return ret_code;
                }

                if (!BCM_VLAN_VALID(old_vlan_lo) || 
                    !BCM_VLAN_VALID(old_vlan_hi)) {
                    goto bcm_err;
                }
                if ((r = bcm_vlan_translate_range_get(unit, port, 
                                                      (bcm_vlan_t)old_vlan_lo,
                                                      (bcm_vlan_t)old_vlan_hi,
                                                      &new_vlan, &prio)) < 0) {
                    goto bcm_err;
                }

                cli_out("New Vlan ID = %d, Priority = %d\n", new_vlan, prio);
                return CMD_OK;
            }

            if (sal_strcasecmp(subsubcmd, "show") == 0) {
                if ((r = bcm_vlan_translate_range_traverse(unit, 
                                                  _robo_vlan_translate_range_print, 
                                                           NULL)) < 0) {
                    goto bcm_err;
                }
                return CMD_OK;
            }

            if (sal_strcasecmp(subsubcmd, "delete") == 0) {
                int     port, old_vlan_lo, old_vlan_hi;

                parse_table_init(unit, &pt);
                parse_table_add(&pt, "Port", PQ_INT, 0, &port, NULL);
                parse_table_add(&pt, "OldVLanLo", PQ_INT, 0, &old_vlan_lo, NULL);
                parse_table_add(&pt, "OldVLanHi", PQ_INT, 0, &old_vlan_hi, NULL);

                if (!parseEndOk( a, &pt, &ret_code)) {
                    return ret_code;
                }

                if (!BCM_VLAN_VALID(old_vlan_lo) || 
                    !BCM_VLAN_VALID(old_vlan_hi)) {
                    goto bcm_err;
                }

                if ((r = bcm_vlan_translate_range_delete(unit, port, old_vlan_lo, 
                                                         old_vlan_hi)) < 0) {
                    goto bcm_err;
                }

                return CMD_OK;
            }

            if (sal_strcasecmp(subsubcmd, "clear") == 0) {
                if ((r = bcm_vlan_translate_range_delete_all(unit)) < 0) {
                    goto bcm_err;
                }

                return CMD_OK;
            }
        }

        cli_out("%s: ERROR: Unknown translate subcommand: %s\n",
                ARG_CMD(a), subcmd);

        return CMD_FAIL;
    }
#endif      /* BCM_V2V_SUPPORT */

    /* Vlan port control */
    if (sal_strcasecmp(subcmd, "port") == 0) {
        char    *value, *tname;
        int ttype, i, varg, matched;
        bcm_port_t port;
        pbmp_t       pbm;
        static struct {         /* match enum from bcm/vlan.h */
            int     type;
            char    *name;
        } typenames[] = {
                { bcmVlanPortIgnorePktTag,         "ignorepkttag" },
                { bcmVlanPortPreferIP4,            "preferip4" },
                { bcmVlanPortPreferMAC,            "prefermac" },
                { bcmVlanTranslateIngressEnable,   "translateingress" },
                { bcmVlanTranslateIngressHitDrop, "translateingresshitdrop" },
                { bcmVlanTranslateIngressMissDrop, "translateingressmissdrop" },
                { bcmVlanTranslateEgressEnable,    "translateegress" },
                { bcmVlanTranslateEgressMissDrop,  "translateegressmissdrop" },
                { bcmVlanTranslateEgressMissUntaggedDrop, "translateegressmissuntagdrop" },
                { bcmVlanTranslateEgressMissUntag,        "translateegressmissuntag"},
                { bcmVlanLookupMACEnable,          "lookupmac" },
                { bcmVlanLookupIPEnable,           "lookupip" },
                { bcmVlanPortUseInnerPri,          "useinnerpri" },
                { bcmVlanPortUseOuterPri,          "useouterpri" },
                { bcmVlanPortVerifyOuterTpid,      "verifyoutertpid" }, 
                { bcmVlanPortOuterTpidSelect,      "outertpidselect"}, 
                { bcmVlanPortTranslateKeyFirst,    "translatekeyfirst"}, 
                { bcmVlanPortTranslateKeySecond,   "translatekeysecond"}, 
                { bcmVlanPortUntaggedDrop,   "untaggeddrop"}, 
                { bcmVlanPortPriTaggedDrop,   "pritaggeddrop"}, 
                { bcmVlanPortJoinAllVlan,   "joinallvlan"}, 
                { 0,                   NULL }   /* LAST ENTRY */
        };
    
        if ((c = ARG_GET(a)) == NULL) {
            BCM_PBMP_ASSIGN(pbm, PBMP_ALL(unit));
        } else if (parse_pbmp(unit, c, &pbm) < 0) {
            cli_out("%s: Error: unrecognized port bitmap: %s\n",
                    ARG_CMD(a), c);
            return CMD_FAIL;
        }
    
        BCM_PBMP_AND(pbm, PBMP_PORT_ALL(unit));
        if (BCM_PBMP_IS_NULL(pbm)) {
            cli_out("No ports specified.\n");
            return CMD_OK;
        }
    
        subcmd = ARG_GET(a);
        value = ARG_GET(a);
    
        matched = 0;
    
        BCM_PBMP_ITER(pbm, port) {
            cli_out("\nVlan Control on Port=%s\n", BCM_PORT_NAME(unit, port));
            for (i = 0; typenames[i].name != NULL; i++) {
                tname = typenames[i].name;
                if (subcmd == NULL || sal_strcasecmp(subcmd, tname) == 0) {
                    matched += 1;
                    ttype = typenames[i].type;
                    if (value == NULL) {
                        r = bcm_vlan_control_port_get(unit, port, ttype, &varg);
                        if (r < 0) {
                            cli_out("%-30s-\t%s\n", tname, bcm_errmsg(r));
                        } else {
                            cli_out("%-30s%d\n", tname, varg);
                        }  
                    } else {
                        varg = parse_integer(value);
                        r = bcm_vlan_control_port_set(unit, port, ttype, varg);
                        if (r < 0) {
                            cli_out("%s\tERROR: %s\n", tname, bcm_errmsg(r));
                        }
                    }
                }
            }
        }    
    
        if (matched == 0) {
            cli_out("%s: ERROR: Unknown port control name\n", subcmd);
            return CMD_FAIL;
        }
    
        return CMD_OK;
    }

    /* Vlan crossconnect */
    if (sal_strcasecmp(subcmd, "crossconnect") == 0) {
        subcmd = ARG_GET(a);

        if (subcmd == NULL) {
            cli_out("%s: ERROR: Missing 'vlan crossconnect' subcommand\n",
                    ARG_CMD(a));
            return CMD_FAIL;
        }
        
        if (sal_strcasecmp(subcmd, "add") == 0) {
            int ovid, ivid, port1, port2;
            parse_table_init(unit, &pt);
            parse_table_add(&pt, "OouterVLan", PQ_INT, 0, &ovid, NULL);
            parse_table_add(&pt, "InnerVLan", PQ_INT, 0, &ivid, NULL);
            parse_table_add(&pt, "Port1", PQ_INT, 0, &port1, NULL);
            parse_table_add(&pt, "Port2", PQ_INT, 0, &port2, NULL);

            if (!parseEndOk( a, &pt, &ret_code)) {
                return ret_code;
            }

            if ((r = bcm_vlan_cross_connect_add(unit,ovid,ivid,port1,port2)) < 0) {
                goto bcm_err;
            }
            return CMD_OK;
        }

        if (sal_strcasecmp(subcmd, "delete") == 0) {
            int ovid, ivid;
            parse_table_init(unit, &pt);
            parse_table_add(&pt, "OouterVLan", PQ_INT, 0, &ovid, NULL);
            parse_table_add(&pt, "InnerVLan", PQ_INT, 0, &ivid, NULL);

            if (!parseEndOk( a, &pt, &ret_code)) {
                return ret_code;
            }

            if ((r = bcm_vlan_cross_connect_delete(unit,ovid,ivid)) < 0) {
                goto bcm_err;
            }
            return CMD_OK;
        }

        if (sal_strcasecmp(subcmd, "clear") == 0) {
            if ((r = bcm_vlan_cross_connect_delete_all(unit)) < 0) {
                goto bcm_err;
            }
            return CMD_OK;
        }

        if (sal_strcasecmp(subcmd, "show") == 0) {
            if ((r = bcm_vlan_cross_connect_traverse(unit, 
                         _robo_vlan_cross_connect_print, NULL)) < 0) {
                goto bcm_err;
            }
            return CMD_OK;
        }

        return CMD_OK;
    }
        
    /* Vlan block */
    if (sal_strcasecmp(subcmd, "block") == 0) {
        bcm_vlan_block_t block;
        subcmd = ARG_GET(a);

        if (subcmd == NULL) {
            cli_out("%s: ERROR: Missing 'vlan block' subcommand\n",
                    ARG_CMD(a));
            return CMD_FAIL;
        }
        
        bcm_vlan_block_t_init(&block);
        if (sal_strcasecmp(subcmd, "add") == 0) {
            if ((c = ARG_GET(a)) != NULL) {
                id = parse_integer(c);
    
                /* Get old configuration */
                if ((r = bcm_vlan_block_get(unit, id, &block)) < 0) {
                    goto bcm_err;
                }
    
                /* Update if any */
                parse_table_init(unit, &pt);
                parse_table_add(&pt, "KnownMcast", PQ_DFL|PQ_PBMP,
                            (void *)(0), &block.known_multicast, NULL);
                parse_table_add(&pt, "UnknownMcast", PQ_DFL|PQ_PBMP,
                            (void *)(0), &block.unknown_multicast, NULL);
                parse_table_add(&pt, "UnknownUcast", PQ_DFL|PQ_PBMP,
                            (void *)(0), &block.unknown_unicast, NULL);
                parse_table_add(&pt, "Bcast", PQ_DFL|PQ_PBMP,
                            (void *)(0), &block.broadcast, NULL);
    
                if (!parseEndOk( a, &pt, &ret_code)) {
                    return ret_code;
                }
    
                if ((r = bcm_vlan_block_set(unit,id,&block)) < 0) {
                    goto bcm_err;
                }
            }
            return CMD_OK;
        }

        if (sal_strcasecmp(subcmd, "get") == 0) {
            bcm_vlan_data_t *list;
            int count, i;

            if ((c = ARG_GET(a)) != NULL) {
                id = parse_integer(c);
            }

            if (id != BCM_VLAN_INVALID) {
                if ((r = bcm_vlan_block_get(unit, id, &block)) < 0) {
                    goto bcm_err;
                }
                cli_out("VLAN ID = %d\n", id);
                cli_out("\tKnown Multicast PortBitmap   = 0x%x\n",SOC_PBMP_WORD_GET(block.known_multicast, 0));
                cli_out("\tUnknown Multicast PortBitmap = 0x%x\n",SOC_PBMP_WORD_GET(block.unknown_multicast, 0));
                cli_out("\tUnknown Unicast PortBitmap   = 0x%x\n",SOC_PBMP_WORD_GET(block.unknown_unicast, 0));
                cli_out("\tBroadcast PortBitmap         = 0x%x\n",SOC_PBMP_WORD_GET(block.broadcast, 0));
            } else {
                if ((r = bcm_vlan_list(unit, &list, &count)) < 0) {
                    goto bcm_err;
                }
    
                for (i = 0; i < count; i++) {
                    id = list[i].vlan_tag;
                    if ((r = bcm_vlan_block_get(unit, id, &block)) < 0) {
                        bcm_vlan_list_destroy(unit, list, count);
                        goto bcm_err;
                    }
                    cli_out("VLAN ID = %d\n", id);
                    cli_out("\tKnown Multicast PortBitmap   = 0x%x\n",SOC_PBMP_WORD_GET(block.known_multicast, 0));
                    cli_out("\tUnknown Multicast PortBitmap = 0x%x\n",SOC_PBMP_WORD_GET(block.unknown_multicast, 0));
                    cli_out("\tUnknown Unicast PortBitmap   = 0x%x\n",SOC_PBMP_WORD_GET(block.unknown_unicast, 0));
                    cli_out("\tBroadcast PortBitmap         = 0x%x\n",SOC_PBMP_WORD_GET(block.broadcast, 0));
                    cli_out("\n");
                }
    
                bcm_vlan_list_destroy(unit, list, count);
            }

            return CMD_OK;
        }

        return CMD_OK;
    }

    /* Vlan action */
    if (sal_strcasecmp(subcmd, "action") == 0) {
        subcmd = ARG_GET(a);
        if (subcmd == NULL) {
            return CMD_USAGE;
        }
        if (sal_strcasecmp(subcmd, "port") == 0) {
            return _robo_vlan_action_port(unit, a);
        }
        if (sal_strcasecmp(subcmd, "translate") == 0) {
            return _robo_vlan_action_translate(unit, a);
        }
        if (sal_strcasecmp(subcmd, "protocol") == 0 ||
            sal_strcasecmp(subcmd, "proto") == 0) {
            return _robo_vlan_action_protocol(unit, a);
        } 

        return CMD_USAGE;
    }

    /* Vlan Policer */
    if (sal_strcasecmp(subcmd, "policer") == 0) {
        return _robo_vlan_policer(unit, a);
    }

    /* DLF configuration */
    if (sal_strcasecmp(subcmd, "dlf") == 0) {
        return _robo_vlan_dlf(unit, a);
    }

    /* Set per Vlan property (Must be the last)*/
    {
        bcm_vlan_control_vlan_t vlan_control, default_control;
        int outer_tpid, learn_disable, unknown_ip6_mcast_to_cpu;
        int def_outer_tpid, def_learn_disable, def_unknown_ip6_mcast_to_cpu;
        int unknown_ip4_mcast_to_cpu, def_unknown_ip4_mcast_to_cpu;
        int def_ip4_disable, ip4_disable;
        int def_ip6_disable, ip6_disable;
        int def_ip4_mcast_disable, ip4_mcast_disable;
        int def_ip6_mcast_disable, ip6_mcast_disable;
        int def_ip4_mcast_l2_disable, ip4_mcast_l2_disable;
        int def_ip6_mcast_l2_disable, ip6_mcast_l2_disable;
        int def_mpls_disable, mpls_disable;
        int def_cosq_enable, cosq_enable;
        int def_l2_lookup_disable, l2_lookup_disable;
        int def_unknown_ucast_drop, unknown_ucast_drop;
        bcm_cos_queue_t def_cosq, cosq;

        id = parse_integer(subcmd);

        sal_memset(&default_control, 0, sizeof(default_control));
        if (VLAN_ID_VALID(id)) {
            r = bcm_vlan_control_vlan_get(unit, id, &default_control);
            if (r < 0) {
                goto bcm_err;
            }

            sal_memcpy(&vlan_control, &default_control, sizeof(vlan_control));

            def_outer_tpid    = default_control.outer_tpid;
            outer_tpid        = def_outer_tpid;
 
            def_cosq          = default_control.cosq;
            cosq              = def_cosq;  

            def_learn_disable = (default_control.flags & 
                                BCM_VLAN_LEARN_DISABLE) ? 1 : 0;
            learn_disable     = def_learn_disable;

            def_unknown_ip6_mcast_to_cpu = (default_control.flags &
                                      BCM_VLAN_UNKNOWN_IP6_MCAST_TOCPU) ? 1 : 0;
            unknown_ip6_mcast_to_cpu     =  def_unknown_ip6_mcast_to_cpu;

            def_unknown_ip4_mcast_to_cpu = (default_control.flags &
                                      BCM_VLAN_UNKNOWN_IP4_MCAST_TOCPU) ? 1 : 0;
            unknown_ip4_mcast_to_cpu     = def_unknown_ip4_mcast_to_cpu;

            def_ip4_disable = (default_control.flags & 
                                BCM_VLAN_IP4_DISABLE) ? 1 : 0;
            ip4_disable     = def_ip4_disable;

            def_ip6_disable = (default_control.flags & 
                                BCM_VLAN_IP6_DISABLE) ? 1 : 0;
            ip6_disable     = def_ip6_disable;

            def_ip4_mcast_disable = (default_control.flags & 
                                BCM_VLAN_IP4_MCAST_DISABLE) ? 1 : 0;
            ip4_mcast_disable     = def_ip4_mcast_disable;

            def_ip6_mcast_disable = (default_control.flags & 
                                BCM_VLAN_IP6_MCAST_DISABLE) ? 1 : 0;
            ip6_mcast_disable     = def_ip6_mcast_disable;

            def_ip4_mcast_l2_disable = (default_control.flags & 
                                BCM_VLAN_IP4_MCAST_L2_DISABLE) ? 1 : 0;
            ip4_mcast_l2_disable     = def_ip4_mcast_l2_disable;

            def_ip6_mcast_l2_disable = (default_control.flags & 
                                BCM_VLAN_IP6_MCAST_L2_DISABLE) ? 1 : 0;
            ip6_mcast_l2_disable     = def_ip6_mcast_l2_disable;

            def_mpls_disable = (default_control.flags & 
                                BCM_VLAN_MPLS_DISABLE) ? 1 : 0;
            mpls_disable     = def_mpls_disable;

            def_cosq_enable = (default_control.flags & 
                               BCM_VLAN_COSQ_ENABLE) ? 1 : 0;
            cosq_enable     = def_cosq_enable;

            def_l2_lookup_disable = (default_control.flags & 
                               BCM_VLAN_L2_LOOKUP_DISABLE) ? 1 : 0;
            l2_lookup_disable     = def_l2_lookup_disable;

            def_unknown_ucast_drop = (default_control.flags & 
                               BCM_VLAN_UNKNOWN_UCAST_DROP) ? 1 : 0;
            unknown_ucast_drop     = def_unknown_ucast_drop;

            parse_table_init(unit, &pt);
            parse_table_add(&pt, "VRF", PQ_INT | PQ_DFL, &default_control.vrf,
                            &vlan_control.vrf, NULL);
            parse_table_add(&pt, "OuterTPID", PQ_HEX | PQ_DFL, &def_outer_tpid, 
                            &outer_tpid, NULL);
            parse_table_add(&pt, "LearnDisable", PQ_INT | PQ_DFL, 
                            &def_learn_disable, &learn_disable, NULL);
            parse_table_add(&pt, "UnknownIp6McastToCpu", PQ_INT | PQ_DFL, 
                            &def_unknown_ip6_mcast_to_cpu, 
                            &unknown_ip6_mcast_to_cpu, NULL);
            parse_table_add(&pt, "UnknownIp4McastToCpu", PQ_INT | PQ_DFL, 
                            &def_unknown_ip4_mcast_to_cpu, 
                            &unknown_ip4_mcast_to_cpu, NULL);
            parse_table_add(&pt, "Ip4Disable", PQ_INT | PQ_DFL, 
                            &def_ip4_disable, &ip4_disable, NULL);
            parse_table_add(&pt, "Ip6Disable", PQ_INT | PQ_DFL, 
                            &def_ip6_disable, &ip6_disable, NULL);
            parse_table_add(&pt, "Ip4McastDisable", PQ_INT | PQ_DFL, 
                            &def_ip4_mcast_disable, &ip4_mcast_disable, NULL);
            parse_table_add(&pt, "Ip6McastDisable", PQ_INT | PQ_DFL, 
                            &def_ip6_mcast_disable, &ip6_mcast_disable, NULL);
            parse_table_add(&pt, "Ip4McastL2Disable", PQ_INT | PQ_DFL, 
                            &def_ip4_mcast_l2_disable, &ip4_mcast_l2_disable, NULL);
            parse_table_add(&pt, "Ip6McastL2Disable", PQ_INT | PQ_DFL, 
                            &def_ip6_mcast_l2_disable, &ip6_mcast_l2_disable, NULL);
            parse_table_add(&pt, "MplsDisable", PQ_INT | PQ_DFL, 
                            &def_mpls_disable, &mpls_disable, NULL);
            parse_table_add(&pt, "CosqEnable", PQ_INT | PQ_DFL, 
                            &def_cosq_enable, &cosq_enable, NULL);
            parse_table_add(&pt, "L2LookupDisable", PQ_INT | PQ_DFL, 
                            &def_l2_lookup_disable, &l2_lookup_disable, NULL);
            parse_table_add(&pt, "UnknownUcastDrop", PQ_INT | PQ_DFL, 
                            &def_unknown_ucast_drop, &unknown_ucast_drop, NULL);
            parse_table_add(&pt, "Cosq", PQ_INT | PQ_DFL, 
                            &def_cosq, &cosq, NULL);
            parse_table_add(&pt, "Ip6McastFloodMode", PQ_MULTI | PQ_DFL, 
                            &default_control.ip6_mcast_flood_mode, 
                            &vlan_control.ip6_mcast_flood_mode,
                            bcm_vlan_mcast_flood_str);
            parse_table_add(&pt, "Ip4McastFloodMode", PQ_MULTI | PQ_DFL, 
                            &default_control.ip4_mcast_flood_mode, 
                            &vlan_control.ip4_mcast_flood_mode,
                            bcm_vlan_mcast_flood_str);
            parse_table_add(&pt, "L2McastFloodMode", PQ_MULTI | PQ_DFL, 
                            &default_control.l2_mcast_flood_mode, 
                            &vlan_control.l2_mcast_flood_mode,
                            bcm_vlan_mcast_flood_str);
            parse_table_add(&pt, "IfClass", PQ_INT | PQ_DFL, 
                            &default_control.if_class, 
                            &vlan_control.if_class, NULL);

            if (!parseEndOk( a, &pt, &ret_code)) {
                return ret_code;
            }

            vlan_control.outer_tpid     = (uint16) outer_tpid;
            vlan_control.cosq           = cosq;
            vlan_control.flags = (learn_disable ? 
                                          BCM_VLAN_LEARN_DISABLE : 0);
            vlan_control.flags |= (unknown_ip6_mcast_to_cpu ? 
                                          BCM_VLAN_UNKNOWN_IP6_MCAST_TOCPU : 0);
            vlan_control.flags |= (unknown_ip4_mcast_to_cpu? 
                                          BCM_VLAN_UNKNOWN_IP4_MCAST_TOCPU : 0);
            vlan_control.flags |= (ip4_disable ? 
                                          BCM_VLAN_IP4_DISABLE : 0);
            vlan_control.flags |= (ip6_disable ? 
                                          BCM_VLAN_IP6_DISABLE : 0);
            vlan_control.flags |= (ip4_mcast_disable ? 
                                          BCM_VLAN_IP4_MCAST_DISABLE : 0);
            vlan_control.flags |= (ip6_mcast_disable ? 
                                          BCM_VLAN_IP6_MCAST_DISABLE : 0);
            vlan_control.flags |= (ip4_mcast_l2_disable ? 
                                          BCM_VLAN_IP4_MCAST_L2_DISABLE : 0);
            vlan_control.flags |= (ip6_mcast_l2_disable ? 
                                          BCM_VLAN_IP6_MCAST_L2_DISABLE : 0);
            vlan_control.flags |= (mpls_disable ? 
                                          BCM_VLAN_MPLS_DISABLE : 0);
            vlan_control.flags |= (cosq_enable ? 
                                          BCM_VLAN_COSQ_ENABLE : 0);
            vlan_control.flags |= (l2_lookup_disable? 
                                          BCM_VLAN_L2_LOOKUP_DISABLE : 0);
            vlan_control.flags |= (unknown_ucast_drop? 
                                          BCM_VLAN_UNKNOWN_UCAST_DROP : 0);
 
            if ((r = bcm_vlan_control_vlan_set(unit, id, vlan_control)) < 0) {
                goto bcm_err;
            }

            return CMD_OK;
        }
    }

    return CMD_USAGE;

 bcm_err:
    cli_out("%s: ERROR: %s\n", ARG_CMD(a), bcm_errmsg(r));
    return CMD_FAIL;
}

