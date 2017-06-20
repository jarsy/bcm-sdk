/*
 * $Id: switch.c,v 1.114 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * General Switch Control
 */

#include <shared/bsl.h>

#include <sal/core/libc.h>

#include <soc/debug.h>
#include <soc/drv.h>
#include <soc/types.h>
#include <soc/robo.h>

#include <shared/switch.h>
#include <bcm/types.h>
#include <bcm/error.h>
#include <bcm/field.h>
#include <bcm/switch.h>
#include <bcm/vlan.h>

#include <bcm_int/robo/port.h>
#include <bcm_int/robo/switch.h>
#include <bcm_int/robo/vlan.h>

#include <bcm_int/robo_dispatch.h>

#define DOS_ATTACK_TO_CPU         0x001
#define DOS_ATTACK_SIP_EQUAL_DIP  0x002
#define DOS_ATTACK_V4_FIRST_FRAG  0x004
#define DOS_ATTACK_TCP_FLAGS      0x008
#define DOS_ATTACK_L4_PORT        0x010
#define DOS_ATTACK_TCP_FRAG       0x020
#define DOS_ATTACK_PING_FLOOD 0x040
#define DOS_ATTACK_SYN_FLOOD  0x080
#define DOS_ATTACK_TCP_SMURF  0x100
#define DOS_ATTACK_TCP_FLAGS_SF  0x200
#define DOS_ATTACK_TCP_FLAGS_FUP  0x400
#define DOS_ATTACK_SYN_FRAG  0x800
#define DOS_ATTACK_FLAG_ZERO_SEQ_ZERO  0x1000

typedef enum dos_attack_case_e{
    DOS_ATTACK_CASE_EQUAL_IP,
    DOS_ATTACK_CASE_V4_FIRST_FRAG,
    DOS_ATTACK_CASE_TCP_FLAG_1,
    DOS_ATTACK_CASE_TCP_FLAG_2,
    DOS_ATTACK_CASE_TCP_FLAG_3,
    DOS_ATTACK_CASE_TCP_FLAG_4,
    DOS_ATTACK_CASE_L4_PORT,
    DOS_ATTACK_CASE_TCP_FRAG,
    DOS_ATTACK_CASE_PING_FLOOD,
    DOS_ATTACK_CASE_SYN_FLOOD,
    DOS_ATTACK_CASE_SYNACK_FLOOD,
    DOS_ATTACK_CASE_TCP_SMURF,

    MAX_NUM_DOS_ATTACK_CASES
} dos_attack_case_t;

/* Per port DoS Attack data */
typedef struct dos_attack_e {
    uint32  attacks; /* bitmap of enabled DoS Attack cases */
    uint32  params[2]; /* for Ping/SYN flood rate limit value */
    int entry_id[MAX_NUM_DOS_ATTACK_CASES];
    bcm_field_stat_t stat_arr[MAX_NUM_DOS_ATTACK_CASES];
    int stat_id[MAX_NUM_DOS_ATTACK_CASES];
    bcm_policer_t pol_id[MAX_NUM_DOS_ATTACK_CASES];
    int range_id;
} dos_attack_t;

dos_attack_t dos_attack_data[BCM_MAX_NUM_UNITS][SOC_MAX_NUM_PORTS];

static bcm_field_group_t dos_attack_group_id[BCM_MAX_NUM_UNITS];
static bcm_field_group_t dos_attack_tcp_group_id[BCM_MAX_NUM_UNITS];

#define TCP_FLAG_FIN 0x01
#define TCP_FLAG_SYN 0x02
#define TCP_FLAG_RST 0x04
#define TCP_FLAG_PSH 0x08
#define TCP_FLAG_ACK 0x10
#define TCP_FLAG_URG 0x20

#define TCP_FLAG_MASK 0x3f
#define TCP_SEQ_NUM_MASK 0xffffffff /* seq num is 32 bits */

#define IP_PROTOCOL_ID_ICMP 0x1

uint8 min_tcp_hdr_size = 20; /* 20 byetes */

#define DOS_ATTACK_ALL_PORT -1

#define BCM_SWITCH_HW_DOS_MONITOR_INTERVAL      500000

/*
 * Function:
 *      _bcm_robo_switch_control_gport_resolve
 * Description:
 *      Decodes local physical port from a gport
 * Parameters:
 *      unit - RoboSwitch PCI device unit number (driver internal).
 *      gport - a gport to decode
 *      port - (Out) Local physical port encoded in gport
 * Returns:
 *      BCM_E_xxxx
 * Note
 *      In case of gport contains other then local port error will be returned.
 */


STATIC int
_bcm_robo_switch_control_gport_resolve(
        int unit, bcm_gport_t gport, bcm_port_t *port)
{
    bcm_module_t    modid;
    bcm_trunk_t     tgid;
    bcm_port_t      local_port;
    int             id, isMymodid;

    if (NULL == port) {
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(
        _bcm_robo_gport_resolve(unit, gport, 
                &modid, &local_port, &tgid, &id));

    if ((BCM_TRUNK_INVALID != tgid) || (-1 != id)) {
        return BCM_E_PARAM;
    }
    /* Check if modid is local */
    BCM_IF_ERROR_RETURN(
        _bcm_robo_modid_is_local(unit, modid, &isMymodid));

    if (isMymodid != TRUE) {
        return BCM_E_PARAM;
    }

    return _bcm_robo_gport_modport_api2hw_map(
                   unit, modid, local_port, &modid, port);
}

static int
_bcm_robo_dos_attack_group_init(int unit)
{
    int rv = BCM_E_NONE;
    bcm_field_qset_t qset;
    bcm_field_group_t tmp_group;

    BCM_FIELD_QSET_INIT(qset);

    if (SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) ||
        SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
        SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
        BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyTcpControl);
        BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyInPorts);
        BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyIpProtocol);
        BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyDstIp);
        BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyIpFrag);
        BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyL4SrcPort);
    } else if (SOC_IS_TB(unit)) {
        BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyInPort);
        BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyIpProtocol);
        BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyDstIp);
        BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyIpFrag);
    }else if (SOC_IS_VO(unit)) {
        BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyInPort);
        BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyInPorts);
        BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyIpProtocol);
        BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyIpFrag);
        BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyRangeCheck);
    } else {
        BCM_FIELD_QSET_ADD(qset, bcmFieldQualifySrcIpEqualDstIp);
        BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyTcpControl);
        BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyRangeCheck);
        BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyEqualL4Port);
        BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyTcpHeaderSize);
        BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyIpProtocol);
        BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyDstIp);
        if (SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit)) {
            BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyInPorts);
        }
    }

    rv = bcm_robo_field_group_create(unit, qset, 
        BCM_FIELD_GROUP_PRIO_ANY, &tmp_group);
    if (rv < 0) {
        return rv;
    }

    dos_attack_group_id[unit] = tmp_group;

    return rv;
}

static int
_bcm_robo_dos_attack_group_get(int unit, int type, bcm_field_group_t *group)
{
    if (!dos_attack_group_id[unit]) {
        /* Get needed group ids */
        BCM_IF_ERROR_RETURN(_bcm_robo_dos_attack_group_init(unit));
    }

    switch(type) {
        case _BCM_SWITCH_DOS_TCP_GROUP:
            *group = dos_attack_tcp_group_id[unit];
            break;
        case _BCM_SWITCH_DOS_COMMON_GROUP:
        default:
            *group = dos_attack_group_id[unit];
            break;
    }
    return BCM_E_NONE;
}

int
_bcm_robo_dos_attack_set(int unit, bcm_port_t port, int dos_attack_type, int param)
{
    int rv = BCM_E_NONE;
    bcm_field_group_t group;
    bcm_field_entry_t entry;
    bcm_field_range_t range;
    int x = 0, y = 0;
    int *entry_id;
    pbmp_t pbm, pbm_mask;
    uint32 entry_size;
    int stat_id = 0;
    bcm_policer_t pol_id = 0;
    bcm_policer_config_t pol_cfg;

    BCM_PBMP_CLEAR(pbm);
    BCM_PBMP_ASSIGN(pbm_mask, PBMP_ALL(unit));

    rv =  _bcm_robo_dos_attack_group_get(unit,
                    _BCM_SWITCH_DOS_COMMON_GROUP, &group);
    if (rv < 0) {
        return rv;
    }

    switch (dos_attack_type) {
        case bcmSwitchDosAttackToCpu:
            /* SDK-33697 : 
             *  - this DOS supports for those chips which provide CFP basis
             *      DOS only. 
             */
            if (!(SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit))){
                return BCM_E_UNAVAIL;
            }
            
            /* Get maximum entry number by chips */
            BCM_IF_ERROR_RETURN(DRV_DEV_PROP_GET(unit, 
                    DRV_DEV_PROP_CFP_TCAM_SIZE, &entry_size));

            /* Dynamically allocate memory for entry_id */
            entry_id = (int *)sal_alloc(entry_size * sizeof(int), "entry_id");
            if (entry_id == NULL) {
                LOG_WARN(BSL_LS_BCM_COMMON,
                         (BSL_META_U(unit,
                                     "Insufficient memory.\n")));
                return BCM_E_MEMORY;
            }
            /* initialize to zero */
            sal_memset(entry_id, 0, entry_size * sizeof(int));

            if (param) {
                if (port != DOS_ATTACK_ALL_PORT) {
                    if (!dos_attack_data[unit][port].attacks) {
                        sal_free(entry_id); 
                        return BCM_E_UNAVAIL;
                    } else if (!(dos_attack_data[unit][port].attacks & DOS_ATTACK_TO_CPU)) {
                        for (y = 0; y < MAX_NUM_DOS_ATTACK_CASES; y++) {
                            if (dos_attack_data[unit][port].entry_id[y] != 0) {
                                entry = dos_attack_data[unit][port].entry_id[y];
                                /* Prevent to change action for the same entry id */
                                if (entry_id[entry] == 0) {
                                    entry_id[entry] = entry;
                                    if ((y == DOS_ATTACK_CASE_PING_FLOOD) || 
                                        (y == DOS_ATTACK_CASE_SYN_FLOOD) || 
                                        (y == DOS_ATTACK_CASE_SYNACK_FLOOD)) {
                                        rv = bcm_robo_field_action_remove(unit, entry, bcmFieldActionRpDrop);
                                    } else {
                                        rv = bcm_robo_field_action_remove(unit, entry, bcmFieldActionDrop);
                                    }
                                    if (rv == BCM_E_NOT_FOUND) {
                                        dos_attack_data[unit][port].entry_id[y] = 0;
                                    } else if (rv == BCM_E_NONE) {
                                        rv = bcm_robo_field_action_add(unit, entry, bcmFieldActionCopyToCpu, 0, 0);
                                        if (rv < 0) {
                                            sal_free(entry_id);
                                            return rv;
                                        }
                                        rv = bcm_robo_field_entry_install(unit, entry);
                                        if (rv < 0) {
                                            sal_free(entry_id);
                                            return rv;
                                        }
                                    } else {
                                        sal_free(entry_id); 
                                        return rv;
                                    }
                                }                                    
                            }
                        }
                        dos_attack_data[unit][port].attacks |= DOS_ATTACK_TO_CPU;
                    }
                    sal_free(entry_id);
                } else {
                    for (x = 0; x < SOC_MAX_NUM_PORTS; x++) {
                        if (!dos_attack_data[unit][x].attacks) {
                            sal_free(entry_id);
                            return BCM_E_UNAVAIL;
                        } else if (!(dos_attack_data[unit][x].attacks & DOS_ATTACK_TO_CPU)) {
                            for (y = 0; y < MAX_NUM_DOS_ATTACK_CASES; y++) {
                                if (dos_attack_data[unit][x].entry_id[y] != 0) {
                                    entry = dos_attack_data[unit][x].entry_id[y];
                                    /* Prevent to change action for the same entry id */
                                    if (entry_id[entry] == 0) {
                                        entry_id[entry] = entry;
                                         if ((y == DOS_ATTACK_CASE_PING_FLOOD) || 
                                             (y == DOS_ATTACK_CASE_SYN_FLOOD) || 
                                             (y == DOS_ATTACK_CASE_SYNACK_FLOOD)) {
                                            rv = bcm_robo_field_action_remove(unit, entry, bcmFieldActionRpDrop);
                                         } else {
                                            rv = bcm_robo_field_action_remove(unit, entry, bcmFieldActionDrop);
                                         }
                                         if (rv == BCM_E_NOT_FOUND) {
                                             dos_attack_data[unit][x].entry_id[y] = 0;
                                         }else if (rv == BCM_E_NONE) {
                                             rv = bcm_robo_field_action_add(unit, entry, bcmFieldActionCopyToCpu, 0, 0);
                                             if (rv < 0) {
                                                 sal_free(entry_id);
                                                 return rv;
                                             }
                                             rv = bcm_robo_field_entry_install(unit, entry);
                                             if (rv < 0) {
                                                 sal_free(entry_id);
                                                 return rv;
                                             }
                                         } else {
                                            sal_free(entry_id);
                                            return rv;
                                         }
                                    }
                                }
                            }
                            dos_attack_data[unit][x].attacks |= DOS_ATTACK_TO_CPU;
                        }
                    }
                sal_free(entry_id);
                }
            } else {
                if (port != DOS_ATTACK_ALL_PORT) {
                    if ((dos_attack_data[unit][port].attacks & ~DOS_ATTACK_TO_CPU) &&
                        (dos_attack_data[unit][port].attacks & DOS_ATTACK_TO_CPU)) {
                        for (y = 0; y < MAX_NUM_DOS_ATTACK_CASES; y++) {
                            if (dos_attack_data[unit][port].entry_id[y] != 0) {
                                entry = dos_attack_data[unit][port].entry_id[y];
                                /* Prevent to change action for the same entry id */
                                if (entry_id[entry] == 0) {
                                    entry_id[entry] = entry;
                                    rv = bcm_robo_field_action_remove(unit, entry, bcmFieldActionCopyToCpu);
                                    if (rv == BCM_E_NOT_FOUND) {
                                        dos_attack_data[unit][port].entry_id[y] = 0;
                                    } else if (rv == BCM_E_NONE) {
                                        if ((y == DOS_ATTACK_CASE_PING_FLOOD) || 
                                            (y == DOS_ATTACK_CASE_SYN_FLOOD) || 
                                            (y == DOS_ATTACK_CASE_SYNACK_FLOOD)) {
                                            rv = bcm_robo_field_action_add(unit, entry, bcmFieldActionRpDrop, 0, 0);
                                            if (rv < 0) {
                                                sal_free(entry_id);
                                                return rv;
                                            }
                                        } else {
                                            rv = bcm_robo_field_action_add(unit, entry, bcmFieldActionDrop, 0, 0);
                                            if (rv < 0) {
                                                sal_free(entry_id);
                                                return rv;
                                            }
                                        }
                                        rv = bcm_robo_field_entry_install(unit, entry);
                                        if (rv < 0) {
                                            sal_free(entry_id);
                                            return rv;
                                        }
                                    } else {
                                        sal_free(entry_id);
                                        return rv;
                                    }
                                }                                    
                            }
                        }
                        dos_attack_data[unit][port].attacks &= ~DOS_ATTACK_TO_CPU;
                    }
                    sal_free(entry_id);
                } else {
                    for (x = 0; x < SOC_MAX_NUM_PORTS; x++) {
                        if ((dos_attack_data[unit][x].attacks & ~DOS_ATTACK_TO_CPU) &&
                            (dos_attack_data[unit][x].attacks & DOS_ATTACK_TO_CPU)) {
                            for (y = 0; y < MAX_NUM_DOS_ATTACK_CASES; y++) {
                                if (dos_attack_data[unit][x].entry_id[y] != 0) {
                                    entry = dos_attack_data[unit][x].entry_id[y];
                                    /* Prevent to change action for the same entry id */
                                    if (entry_id[entry] == 0) {
                                        entry_id[entry] = entry;
                                        rv = bcm_robo_field_action_remove(unit, entry, bcmFieldActionCopyToCpu);
                                        if (rv == BCM_E_NOT_FOUND) {
                                            dos_attack_data[unit][x].entry_id[y] = 0;
                                        } else if (rv == BCM_E_NONE) {
                                            if ((y == DOS_ATTACK_CASE_PING_FLOOD) || 
                                                (y == DOS_ATTACK_CASE_SYN_FLOOD) || 
                                                (y == DOS_ATTACK_CASE_SYNACK_FLOOD)) {
                                                rv = bcm_robo_field_action_add(unit, entry, bcmFieldActionRpDrop, 0, 0);
                                                if (rv < 0) {
                                                    sal_free(entry_id);
                                                    return rv;
                                                }
                                            } else {
                                                rv = bcm_robo_field_action_add(unit, entry, bcmFieldActionDrop, 0, 0);
                                                if (rv < 0) {
                                                    sal_free(entry_id);
                                                    return rv;
                                                }
                                            }
                                            rv = bcm_robo_field_entry_install(unit, entry);
                                            if (rv < 0) {
                                                sal_free(entry_id);
                                                return rv;
                                            }
                                        } else {
                                            sal_free(entry_id);
                                            return rv;
                                        }
                                    }
                                }
                            }
                            dos_attack_data[unit][x].attacks &= ~DOS_ATTACK_TO_CPU;
                        }
                    }
                    sal_free(entry_id);
                }
            }
            break;
        case bcmSwitchDosAttackSipEqualDip:
            if (SOC_IS_VULCAN(unit) || SOC_IS_TBX(unit) || SOC_IS_STARFIGHTER(unit) ||
                SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) || 
                SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
                rv = BCM_E_UNAVAIL;
                return rv;
            }
            if (param) {
                if (port != DOS_ATTACK_ALL_PORT) {
                    entry = dos_attack_data[unit][port].entry_id[DOS_ATTACK_CASE_EQUAL_IP];
                } else {
                    entry = dos_attack_data[unit][0].entry_id[DOS_ATTACK_CASE_EQUAL_IP];
                }

                if (!entry) {

                    rv = bcm_robo_field_entry_create(unit, group, &entry);
                    if (rv < 0) {
                        return rv;
                    }
                    rv = bcm_robo_field_entry_prio_set(unit, entry, 
                        BCM_FIELD_ENTRY_PRIO_HIGHEST);
                    if (rv < 0) {
                        return rv;
                    }

                    if (port != DOS_ATTACK_ALL_PORT) {
                        dos_attack_data[unit][port].entry_id[DOS_ATTACK_CASE_EQUAL_IP] = entry;

                        if (SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit)) {
                            BCM_PBMP_PORT_ADD(pbm, port);
                            BCM_IF_ERROR_RETURN(
                                bcm_robo_field_qualify_InPorts
                                (unit, entry, pbm, pbm_mask));
                        }
                    } else {
                        for (x = 0; x < SOC_MAX_NUM_PORTS; x++) {
                            dos_attack_data[unit][x].entry_id[DOS_ATTACK_CASE_EQUAL_IP] = entry;
                        }
                    }
         
                    BCM_IF_ERROR_RETURN(
                        bcm_robo_field_qualify_SrcIpEqualDstIp(unit, entry, 0x1));
                    BCM_IF_ERROR_RETURN(
                        bcm_robo_field_action_add(unit, entry, bcmFieldActionDrop, 0, 0));
                    if (port != DOS_ATTACK_ALL_PORT) {
                        x = port;
                    } else {
                        x = 0; /* port 0 */
                    }
                    stat_id = dos_attack_data[unit][x].stat_id[DOS_ATTACK_CASE_EQUAL_IP];
                    if (stat_id == 0) {
                        dos_attack_data[unit][x].stat_arr[DOS_ATTACK_CASE_EQUAL_IP] = bcmFieldStatPackets;
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_field_stat_create(unit, group, 1, 
                                &dos_attack_data[unit][x].stat_arr[DOS_ATTACK_CASE_EQUAL_IP], 
                                &stat_id));
                        dos_attack_data[unit][x].stat_id[DOS_ATTACK_CASE_EQUAL_IP] = stat_id;
                    }
                    BCM_IF_ERROR_RETURN(
                        bcm_robo_field_entry_stat_attach(unit, entry, stat_id));
                    BCM_IF_ERROR_RETURN(
                        bcm_robo_field_entry_install(unit, entry));
                }

                if (port != DOS_ATTACK_ALL_PORT) {
                    dos_attack_data[unit][port].attacks |= DOS_ATTACK_SIP_EQUAL_DIP;
                    dos_attack_data[unit][port].attacks &= ~DOS_ATTACK_TO_CPU;
                } else {
                    for (x = 0; x < SOC_MAX_NUM_PORTS; x++) {
                        dos_attack_data[unit][x].attacks |= DOS_ATTACK_SIP_EQUAL_DIP;
                        dos_attack_data[unit][x].attacks &= ~DOS_ATTACK_TO_CPU;
                    }
                }
            } else {
                if (port != DOS_ATTACK_ALL_PORT) {
                    entry = 
                        dos_attack_data[unit][port].entry_id[DOS_ATTACK_CASE_EQUAL_IP];
                    stat_id = dos_attack_data[unit][port].stat_id[DOS_ATTACK_CASE_EQUAL_IP];
                    if ((stat_id != 0) && (entry != 0)) {
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_field_entry_stat_detach(unit, entry, stat_id));
                    }
                    if (stat_id != 0) {
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_field_stat_destroy(unit, stat_id));
                    }
                    dos_attack_data[unit][port].stat_id[DOS_ATTACK_CASE_EQUAL_IP] = 0;
                    if (entry) {
                        rv = bcm_robo_field_entry_destroy(unit, entry);
                    }
                    if ((rv == BCM_E_NONE) || (rv == BCM_E_NOT_FOUND)) {
                        dos_attack_data[unit][port].entry_id[DOS_ATTACK_CASE_EQUAL_IP] = 0;
                        dos_attack_data[unit][port].attacks &= ~DOS_ATTACK_SIP_EQUAL_DIP;
                    } else {
                        return rv;
                    }
                } else {
                    /*
                     * Since DOS_ATTACK_ALL_PORT set all port's data base same
                     * value, retrieve port 0's entry id to remove entry.
                     */
                    entry = 
                        dos_attack_data[unit][0].entry_id[DOS_ATTACK_CASE_EQUAL_IP];
                    stat_id = dos_attack_data[unit][0].stat_id[DOS_ATTACK_CASE_EQUAL_IP];
                    if ((stat_id != 0) && (entry != 0)) {
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_field_entry_stat_detach(unit, entry, stat_id));
                    }
                    if (stat_id != 0) {
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_field_stat_destroy(unit, stat_id));
                    }
                    if (entry) {
                        rv = bcm_robo_field_entry_destroy(unit, entry);
                    }
                    if ((rv == BCM_E_NONE) || (rv == BCM_E_NOT_FOUND)) {
                        for (x = 0; x < SOC_MAX_NUM_PORTS; x++) {
                            dos_attack_data[unit][x].entry_id[DOS_ATTACK_CASE_EQUAL_IP] = 0;
                            dos_attack_data[unit][x].stat_id[DOS_ATTACK_CASE_EQUAL_IP] = 0;
                            dos_attack_data[unit][x].attacks &= ~DOS_ATTACK_SIP_EQUAL_DIP;
                        }
                    } else {
                        return rv;
                    }
                }
            }
            break;
        case bcmSwitchDosAttackMinTcpHdrSize:
            /* This DoS type is used to work with TcpFrag DoS type, but the
             * TcpFrag DoS type through CFP solution is now unavailable.
             * Thus this DoS type become unavailable here.
             */
            return BCM_E_UNAVAIL;
            break;
        case bcmSwitchDosAttackV4FirstFrag:
            if (!(SOC_IS_VULCAN(unit) || SOC_IS_TBX(unit) ||
                SOC_IS_STARFIGHTER(unit) || SOC_IS_POLAR(unit) || 
                SOC_IS_NORTHSTAR(unit) || SOC_IS_NORTHSTARPLUS(unit) ||
                SOC_IS_STARFIGHTER3(unit))) {
                rv = BCM_E_UNAVAIL;
                return rv;
            }

            if (param) {
                if (port != DOS_ATTACK_ALL_PORT) {
                    entry = dos_attack_data[unit][port].entry_id[DOS_ATTACK_CASE_V4_FIRST_FRAG];
                } else {
                    entry = dos_attack_data[unit][0].entry_id[DOS_ATTACK_CASE_V4_FIRST_FRAG];
                }

                if (!entry) {

                    rv = bcm_robo_field_entry_create(unit, group, &entry);
                    if (rv < 0) {
                        return rv;
                    }
                    rv = bcm_robo_field_entry_prio_set(unit, entry, 
                        BCM_FIELD_ENTRY_PRIO_HIGHEST);
                    if (rv < 0) {
                        return rv;
                    }

                    if (port != DOS_ATTACK_ALL_PORT) {
                        dos_attack_data[unit][port].entry_id[DOS_ATTACK_CASE_V4_FIRST_FRAG] = entry;

                        if (SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) ||
                            SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) || 
                            SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
                            BCM_PBMP_PORT_ADD(pbm, port);
                            BCM_IF_ERROR_RETURN(bcm_robo_field_qualify_InPorts
                                    (unit, entry, pbm, pbm_mask));
                        } else if (SOC_IS_TBX(unit)){
                            BCM_IF_ERROR_RETURN(bcm_robo_field_qualify_InPort
                                    (unit, entry, port, 
                                    BCM_FIELD_EXACT_MATCH_MASK));
                        }
                    } else {
                        for (x = 0; x < SOC_MAX_NUM_PORTS; x++) {
                            dos_attack_data[unit][x].entry_id[DOS_ATTACK_CASE_V4_FIRST_FRAG] = entry;
                        }
                    }
         
                    BCM_IF_ERROR_RETURN(
                        bcm_robo_field_qualify_IpFrag(unit, entry, bcmFieldIpFragFirst));
                    BCM_IF_ERROR_RETURN(
                        bcm_robo_field_action_add(unit, entry, bcmFieldActionDrop, 0, 0));
                    if (port != DOS_ATTACK_ALL_PORT) {
                        x = port;
                    } else {
                        x = 0; /* port 0 */
                    }
                    stat_id = dos_attack_data[unit][x].stat_id[DOS_ATTACK_CASE_V4_FIRST_FRAG];
                    if (stat_id == 0) {
                        dos_attack_data[unit][x].stat_arr[DOS_ATTACK_CASE_V4_FIRST_FRAG] = bcmFieldStatPackets;
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_field_stat_create(unit, group, 1, 
                                &dos_attack_data[unit][x].stat_arr[DOS_ATTACK_CASE_V4_FIRST_FRAG], 
                                &stat_id));
                        dos_attack_data[unit][x].stat_id[DOS_ATTACK_CASE_V4_FIRST_FRAG] = stat_id;
                    }
                    BCM_IF_ERROR_RETURN(
                        bcm_robo_field_entry_stat_attach(unit, entry, stat_id));
                    BCM_IF_ERROR_RETURN(
                        bcm_robo_field_entry_install(unit, entry));
                }

                if (port != DOS_ATTACK_ALL_PORT) {
                    dos_attack_data[unit][port].attacks |= DOS_ATTACK_V4_FIRST_FRAG;
                    dos_attack_data[unit][port].attacks &= ~DOS_ATTACK_TO_CPU;
                } else {
                    for (x = 0; x < SOC_MAX_NUM_PORTS; x++) {
                        dos_attack_data[unit][x].attacks |= DOS_ATTACK_V4_FIRST_FRAG;
                        dos_attack_data[unit][x].attacks &= ~DOS_ATTACK_TO_CPU;
                    }
                }
            } else {
                if (port != DOS_ATTACK_ALL_PORT) {
                    entry = 
                        dos_attack_data[unit][port].entry_id[DOS_ATTACK_CASE_V4_FIRST_FRAG];
                    stat_id = dos_attack_data[unit][port].stat_id[DOS_ATTACK_CASE_V4_FIRST_FRAG];
                    if ((stat_id != 0) && (entry != 0)) {
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_field_entry_stat_detach(unit, entry, stat_id));
                    }
                    if (stat_id != 0) {
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_field_stat_destroy(unit, stat_id));
                    }
                    dos_attack_data[unit][port].stat_id[DOS_ATTACK_CASE_V4_FIRST_FRAG] = 0;
                    if (entry) {
                        rv = bcm_robo_field_entry_destroy(unit, entry);
                    }
                    if ((rv == BCM_E_NONE) || (rv == BCM_E_NOT_FOUND)) {
                        dos_attack_data[unit][port].entry_id[DOS_ATTACK_CASE_V4_FIRST_FRAG] = 0;
                        dos_attack_data[unit][port].attacks &= ~DOS_ATTACK_V4_FIRST_FRAG;
                    } else {
                        return rv;
                    }
                } else {
                    /*
                     * Since DOS_ATTACK_ALL_PORT set all port's data base same
                     * value, retrieve port 0's entry id to remove entry.
                     */
                    entry = 
                        dos_attack_data[unit][0].entry_id[DOS_ATTACK_CASE_V4_FIRST_FRAG];
                    stat_id = dos_attack_data[unit][0].stat_id[DOS_ATTACK_CASE_EQUAL_IP];
                    if ((stat_id != 0) && (entry != 0)) {
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_field_entry_stat_detach(unit, entry, stat_id));
                    }
                    if (stat_id != 0) {
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_field_stat_destroy(unit, stat_id));
                    }
                    if (entry) {
                        rv = bcm_robo_field_entry_destroy(unit, entry);
                    }
                    if ((rv == BCM_E_NONE) || (rv == BCM_E_NOT_FOUND)) {
                        for (x = 0; x < SOC_MAX_NUM_PORTS; x++) {
                            dos_attack_data[unit][x].entry_id[DOS_ATTACK_CASE_V4_FIRST_FRAG] = 0;
                            dos_attack_data[unit][x].stat_id[DOS_ATTACK_CASE_V4_FIRST_FRAG] = 0;
                            dos_attack_data[unit][x].attacks &= ~DOS_ATTACK_V4_FIRST_FRAG;
                        }
                    } else {
                        return rv;
                    }
                }
            }
            break;
        case bcmSwitchDosAttackTcpFlags:
            if (SOC_IS_TBX(unit)){
                /* TCP Control in TB is unavail */
                rv = BCM_E_UNAVAIL;
                return rv;
            }
            if (param) {
                /*
                 * Case 1: TCP SYN flag = 1 & source port < 1024
                 */
                if (port != DOS_ATTACK_ALL_PORT) {
                    entry = dos_attack_data[unit][port].entry_id[DOS_ATTACK_CASE_TCP_FLAG_1];
                } else {
                    entry = dos_attack_data[unit][0].entry_id[DOS_ATTACK_CASE_TCP_FLAG_1];
                }

                if (!entry) {
                    rv = bcm_robo_field_entry_create(unit, group, &entry);
                    if (rv < 0) {
                        return rv;
                    }
                    rv = bcm_robo_field_entry_prio_set(unit, entry, 
                        BCM_FIELD_ENTRY_PRIO_HIGHEST);
                    if (rv < 0) {
                        return rv;
                    }
    
                    if (port != DOS_ATTACK_ALL_PORT) {
                        dos_attack_data[unit][port].entry_id[DOS_ATTACK_CASE_TCP_FLAG_1] = entry;
    
                        if (SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit) || 
                            SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) || 
                            SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) || 
                            SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
                            BCM_PBMP_PORT_ADD(pbm, port);
                            BCM_IF_ERROR_RETURN(
                                bcm_robo_field_qualify_InPorts
                                (unit, entry, pbm, pbm_mask));
                        } else {
                            /* Can't configure source port < 1024 */
                            return BCM_E_UNAVAIL;
                        }
                    } else {
                        for (x = 0; x < SOC_MAX_NUM_PORTS; x++) {
                            dos_attack_data[unit][x].entry_id[DOS_ATTACK_CASE_TCP_FLAG_1] = entry;
                        }
                    }
    
                    BCM_IF_ERROR_RETURN(
                        bcm_robo_field_qualify_TcpControl(unit, entry, TCP_FLAG_SYN, TCP_FLAG_MASK));
                    
                    if (SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) ||
                        SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
                        SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_field_qualify_L4SrcPort(unit, 
                                entry, 0x0, 0xFC00));
                    } else {
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_field_range_create(unit, &range, BCM_FIELD_RANGE_SRCPORT, 0, 1023));
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_field_qualify_RangeCheck(unit, entry, range, 0));
        
                            if (port != DOS_ATTACK_ALL_PORT) {
                                    dos_attack_data[unit][port].range_id = range;
                            } else {
                                for (x = 0; x < SOC_MAX_NUM_PORTS; x++) {
                                    dos_attack_data[unit][x].range_id = range;
                                }
                        }
                            
                    }
    
                    BCM_IF_ERROR_RETURN(
                        bcm_robo_field_action_add(unit, entry, bcmFieldActionDrop, 0, 0));
                    if (port != DOS_ATTACK_ALL_PORT) {
                        x = port;
                    } else {
                        x = 0; /* port 0 */
                    }
                    stat_id = dos_attack_data[unit][x].stat_id[DOS_ATTACK_CASE_TCP_FLAG_1];
                    if (stat_id == 0) {
                        dos_attack_data[unit][x].stat_arr[DOS_ATTACK_CASE_TCP_FLAG_1] = bcmFieldStatPackets;
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_field_stat_create(unit, group, 1, 
                                &dos_attack_data[unit][x].stat_arr[DOS_ATTACK_CASE_TCP_FLAG_1], 
                                &stat_id));
                        dos_attack_data[unit][x].stat_id[DOS_ATTACK_CASE_TCP_FLAG_1] = stat_id;
                    }
                    BCM_IF_ERROR_RETURN(
                        bcm_robo_field_entry_stat_attach(unit, entry, stat_id));
                    if (port != DOS_ATTACK_ALL_PORT) {
                        dos_attack_data[unit][port].stat_id[DOS_ATTACK_CASE_TCP_FLAG_1] = stat_id;
                    } else {
                        for (x = 0; x < SOC_MAX_NUM_PORTS; x++) {
                            dos_attack_data[unit][x].stat_id[DOS_ATTACK_CASE_TCP_FLAG_1] = stat_id;
                        }
                    }
                    BCM_IF_ERROR_RETURN(
                        bcm_robo_field_entry_install(unit, entry));
                }

                /*
                 * Case 2: TCP flag = 0
                 */
                if (port != DOS_ATTACK_ALL_PORT) {
                    entry = dos_attack_data[unit][port].entry_id[DOS_ATTACK_CASE_TCP_FLAG_2];
                } else {
                    entry = dos_attack_data[unit][0].entry_id[DOS_ATTACK_CASE_TCP_FLAG_2];
                }

                if (!entry) {
                    rv = bcm_robo_field_entry_create(unit, group, &entry);
                    if (rv < 0) {
                        return rv;
                    }
                    rv = bcm_robo_field_entry_prio_set(unit, entry, 
                        BCM_FIELD_ENTRY_PRIO_HIGHEST);
                    if (rv < 0) {
                        return rv;
                    }
    
                    if (port != DOS_ATTACK_ALL_PORT) {
                        dos_attack_data[unit][port].entry_id[DOS_ATTACK_CASE_TCP_FLAG_2] = entry;
    
                        if (SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit) ||
                            SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) || 
                            SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
                            SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
                            BCM_PBMP_PORT_ADD(pbm, port);
                            BCM_IF_ERROR_RETURN(
                                bcm_robo_field_qualify_InPorts
                                (unit, entry, pbm, pbm_mask));
                        }
                    } else {
                        for (x = 0; x < SOC_MAX_NUM_PORTS; x++) {
                            dos_attack_data[unit][x].entry_id[DOS_ATTACK_CASE_TCP_FLAG_2] = entry;
                        }
                    }
    
                    BCM_IF_ERROR_RETURN(
                        bcm_robo_field_qualify_TcpControl(unit, entry, 0x0, TCP_FLAG_MASK));
                    BCM_IF_ERROR_RETURN(
                        bcm_robo_field_action_add(unit, entry, bcmFieldActionDrop, 0, 0));
                    if (port != DOS_ATTACK_ALL_PORT) {
                        x = port;
                    } else {
                        x = 0; /* port 0 */
                    }
                    stat_id = dos_attack_data[unit][x].stat_id[DOS_ATTACK_CASE_TCP_FLAG_2];
                    if (stat_id == 0) {
                        dos_attack_data[unit][x].stat_arr[DOS_ATTACK_CASE_TCP_FLAG_2] = bcmFieldStatPackets;
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_field_stat_create(unit, group, 1, 
                                &dos_attack_data[unit][x].stat_arr[DOS_ATTACK_CASE_TCP_FLAG_2], 
                                &stat_id));
                        dos_attack_data[unit][x].stat_id[DOS_ATTACK_CASE_TCP_FLAG_2] = stat_id;
                    }
                    BCM_IF_ERROR_RETURN(
                        bcm_robo_field_entry_stat_attach(unit, entry, stat_id));
                    if (port != DOS_ATTACK_ALL_PORT) {
                        dos_attack_data[unit][port].stat_id[DOS_ATTACK_CASE_TCP_FLAG_2] = stat_id;
                    } else {
                        for (x = 0; x < SOC_MAX_NUM_PORTS; x++) {
                            dos_attack_data[unit][x].stat_id[DOS_ATTACK_CASE_TCP_FLAG_2] = stat_id;
                        }
                    }
                    BCM_IF_ERROR_RETURN(
                        bcm_robo_field_entry_install(unit, entry));
                }

                /*
                 * Case 3: TCP FIN, URG, PSH = 1
                 */
                if (port != DOS_ATTACK_ALL_PORT) {
                    entry = dos_attack_data[unit][port].entry_id[DOS_ATTACK_CASE_TCP_FLAG_3];
                } else {
                    entry = dos_attack_data[unit][0].entry_id[DOS_ATTACK_CASE_TCP_FLAG_3];
                }

                if (!entry) {
                    rv = bcm_robo_field_entry_create(unit, group, &entry);
                    if (rv < 0) {
                        return rv;
                    }
                    rv = bcm_robo_field_entry_prio_set(unit, entry, 
                        BCM_FIELD_ENTRY_PRIO_HIGHEST);
                    if (rv < 0) {
                        return rv;
                    }
    
                    if (port != DOS_ATTACK_ALL_PORT) {
                        dos_attack_data[unit][port].entry_id[DOS_ATTACK_CASE_TCP_FLAG_3] = entry;
    
                        if (SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit) ||
                            SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) ||
                            SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
                            SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
                            BCM_PBMP_PORT_ADD(pbm, port);
                            BCM_IF_ERROR_RETURN(
                                bcm_robo_field_qualify_InPorts
                                (unit, entry, pbm, pbm_mask));
                        }
                    } else {
                        for (x = 0; x < SOC_MAX_NUM_PORTS; x++) {
                            dos_attack_data[unit][x].entry_id[DOS_ATTACK_CASE_TCP_FLAG_3] = entry;
                        }
                    }
    
                    BCM_IF_ERROR_RETURN(
                        bcm_robo_field_qualify_TcpControl(unit, entry, 
                                                 TCP_FLAG_FIN | TCP_FLAG_URG | TCP_FLAG_PSH, 
                                                 TCP_FLAG_MASK));
                    BCM_IF_ERROR_RETURN(
                        bcm_robo_field_action_add(unit, entry, bcmFieldActionDrop, 0, 0));
                    if (port != DOS_ATTACK_ALL_PORT) {
                        x = port;
                    } else {
                        x = 0; /* port 0 */
                    }
                    stat_id = dos_attack_data[unit][x].stat_id[DOS_ATTACK_CASE_TCP_FLAG_3];
                    if (stat_id == 0) {
                        dos_attack_data[unit][x].stat_arr[DOS_ATTACK_CASE_TCP_FLAG_3] = bcmFieldStatPackets;
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_field_stat_create(unit, group, 1, 
                                &dos_attack_data[unit][x].stat_arr[DOS_ATTACK_CASE_TCP_FLAG_3], 
                                &stat_id));
                        dos_attack_data[unit][x].stat_id[DOS_ATTACK_CASE_TCP_FLAG_3] = stat_id;
                    }
                    BCM_IF_ERROR_RETURN(
                        bcm_robo_field_entry_stat_attach(unit, entry, stat_id));
                    if (port != DOS_ATTACK_ALL_PORT) {
                        dos_attack_data[unit][port].stat_id[DOS_ATTACK_CASE_TCP_FLAG_3] = stat_id;
                    } else {
                        for (x = 0; x < SOC_MAX_NUM_PORTS; x++) {
                            dos_attack_data[unit][x].stat_id[DOS_ATTACK_CASE_TCP_FLAG_3] = stat_id;
                        }
                    }
                    BCM_IF_ERROR_RETURN(
                        bcm_robo_field_entry_install(unit, entry));
                }

                /*
                 * Case 4: TCP SYN and FIN = 1
                 */
                if (port != DOS_ATTACK_ALL_PORT) {
                    entry = dos_attack_data[unit][port].entry_id[DOS_ATTACK_CASE_TCP_FLAG_4];
                } else {
                    entry = dos_attack_data[unit][0].entry_id[DOS_ATTACK_CASE_TCP_FLAG_4];
                }

                if (!entry) {
                    rv = bcm_robo_field_entry_create(unit, group, &entry);
                    if (rv < 0) {
                        return rv;
                    }
                    rv = bcm_robo_field_entry_prio_set(unit, entry, 
                        BCM_FIELD_ENTRY_PRIO_HIGHEST);
                    if (rv < 0) {
                        return rv;
                    }
    
                    if (port != DOS_ATTACK_ALL_PORT) {
                        dos_attack_data[unit][port].entry_id[DOS_ATTACK_CASE_TCP_FLAG_4] = entry;
    
                        if (SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit) ||
                            SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) || 
                            SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) || 
                            SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
                            BCM_PBMP_PORT_ADD(pbm, port);
                            BCM_IF_ERROR_RETURN(
                                bcm_robo_field_qualify_InPorts
                                (unit, entry, pbm, pbm_mask));
                        }
                    } else {
                        for (x = 0; x < SOC_MAX_NUM_PORTS; x++) {
                            dos_attack_data[unit][x].entry_id[DOS_ATTACK_CASE_TCP_FLAG_4] = entry;
                        }
                    }
    
                    BCM_IF_ERROR_RETURN(
                        bcm_robo_field_qualify_TcpControl(unit, entry, 
                                                 TCP_FLAG_SYN | TCP_FLAG_FIN, 
                                                 TCP_FLAG_MASK));
                    BCM_IF_ERROR_RETURN(
                        bcm_robo_field_action_add(unit, entry, bcmFieldActionDrop, 0, 0));
                    if (port != DOS_ATTACK_ALL_PORT) {
                        x = port;
                    } else {
                        x = 0; /* port 0 */
                    }
                    stat_id = dos_attack_data[unit][x].stat_id[DOS_ATTACK_CASE_TCP_FLAG_4];
                    if (stat_id == 0) {
                        dos_attack_data[unit][x].stat_arr[DOS_ATTACK_CASE_TCP_FLAG_4] = bcmFieldStatPackets;
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_field_stat_create(unit, group, 1, 
                                &dos_attack_data[unit][x].stat_arr[DOS_ATTACK_CASE_TCP_FLAG_4], 
                                &stat_id));
                        dos_attack_data[unit][x].stat_id[DOS_ATTACK_CASE_TCP_FLAG_4] = stat_id;
                    }
                    
                    BCM_IF_ERROR_RETURN(
                        bcm_robo_field_entry_stat_attach(unit, entry, stat_id));
                    if (port != DOS_ATTACK_ALL_PORT) {
                        dos_attack_data[unit][port].stat_id[DOS_ATTACK_CASE_TCP_FLAG_4] = stat_id;
                    } else {
                        for (x = 0; x < SOC_MAX_NUM_PORTS; x++) {
                            dos_attack_data[unit][x].stat_id[DOS_ATTACK_CASE_TCP_FLAG_4] = stat_id;
                        }
                    }
                    BCM_IF_ERROR_RETURN(
                        bcm_robo_field_entry_install(unit, entry));
                }

                if (port != DOS_ATTACK_ALL_PORT) {
                    dos_attack_data[unit][port].attacks |= DOS_ATTACK_TCP_FLAGS;
                    dos_attack_data[unit][port].attacks &= ~DOS_ATTACK_TO_CPU;
                } else {
                    for (x = 0; x < SOC_MAX_NUM_PORTS; x++) {
                        dos_attack_data[unit][x].attacks |= DOS_ATTACK_TCP_FLAGS;
                        dos_attack_data[unit][x].attacks &= ~DOS_ATTACK_TO_CPU;
                    }
                }
            } else {
                if (port != DOS_ATTACK_ALL_PORT) {
                    range = dos_attack_data[unit][port].range_id;
                    rv = bcm_robo_field_range_destroy(unit, range);

                    entry = 
                        dos_attack_data[unit][port].entry_id[DOS_ATTACK_CASE_TCP_FLAG_1];
                    stat_id = dos_attack_data[unit][port].stat_id[DOS_ATTACK_CASE_TCP_FLAG_1];
                    if ((stat_id != 0) && (entry != 0)) {
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_field_entry_stat_detach(unit, entry, stat_id));
                    }
                    if (stat_id != 0) {
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_field_stat_destroy(unit, stat_id));
                    }
                    dos_attack_data[unit][port].stat_id[DOS_ATTACK_CASE_TCP_FLAG_1] = 0;
                    if (entry) {
                        rv = bcm_robo_field_entry_destroy(unit, entry);
                    }
                    if ((rv == BCM_E_NONE) || (rv == BCM_E_NOT_FOUND)) {
                        dos_attack_data[unit][port].entry_id[DOS_ATTACK_CASE_TCP_FLAG_1] = 0;
                    } else {
                        return rv;
                    }

                    entry = 
                        dos_attack_data[unit][port].entry_id[DOS_ATTACK_CASE_TCP_FLAG_2];
                    stat_id = dos_attack_data[unit][port].stat_id[DOS_ATTACK_CASE_TCP_FLAG_2];
                    if ((stat_id != 0) && (entry != 0)) {
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_field_entry_stat_detach(unit, entry, stat_id));
                    }
                    if (stat_id != 0) {
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_field_stat_destroy(unit, stat_id));
                    }
                    dos_attack_data[unit][port].stat_id[DOS_ATTACK_CASE_TCP_FLAG_2] = 0;
                    if (entry) {
                        rv = bcm_robo_field_entry_destroy(unit, entry);
                    }
                    if ((rv == BCM_E_NONE) || (rv == BCM_E_NOT_FOUND)) {
                        dos_attack_data[unit][port].entry_id[DOS_ATTACK_CASE_TCP_FLAG_2] = 0;
                    } else {
                        return rv;
                    }

                    entry = 
                        dos_attack_data[unit][port].entry_id[DOS_ATTACK_CASE_TCP_FLAG_3];
                    stat_id = dos_attack_data[unit][port].stat_id[DOS_ATTACK_CASE_TCP_FLAG_3];
                    if ((stat_id != 0) && (entry != 0)) {
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_field_entry_stat_detach(unit, entry, stat_id));
                    }
                    if (stat_id != 0) {
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_field_stat_destroy(unit, stat_id));
                    }
                    dos_attack_data[unit][port].stat_id[DOS_ATTACK_CASE_TCP_FLAG_3] = 0;
                    if (entry) {
                        rv = bcm_robo_field_entry_destroy(unit, entry);
                    }
                    if ((rv == BCM_E_NONE) || (rv == BCM_E_NOT_FOUND)) {
                        dos_attack_data[unit][port].entry_id[DOS_ATTACK_CASE_TCP_FLAG_3] = 0;
                    } else {
                        return rv;
                    }

                    entry = 
                        dos_attack_data[unit][port].entry_id[DOS_ATTACK_CASE_TCP_FLAG_4];
                    stat_id = dos_attack_data[unit][port].stat_id[DOS_ATTACK_CASE_TCP_FLAG_4];
                    if ((stat_id != 0) && (entry != 0)) {
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_field_entry_stat_detach(unit, entry, stat_id));
                    }
                    if (stat_id != 0) {
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_field_stat_destroy(unit, stat_id));
                    }
                    dos_attack_data[unit][port].stat_id[DOS_ATTACK_CASE_TCP_FLAG_4] = 0;
                    if (entry) {
                        rv = bcm_robo_field_entry_destroy(unit, entry);
                    }
                    if ((rv == BCM_E_NONE) || (rv == BCM_E_NOT_FOUND)) {
                        dos_attack_data[unit][port].entry_id[DOS_ATTACK_CASE_TCP_FLAG_4] = 0;
                    } else {
                        return rv;
                    }

                    dos_attack_data[unit][port].attacks &= ~DOS_ATTACK_TCP_FLAGS;
                } else {
                    /*
                     * Since DOS_ATTACK_ALL_PORT set all port's data base same
                     * value, retrieve port 0's entry id to remove entry.
                     */
                    range = dos_attack_data[unit][0].range_id;
                    rv = bcm_robo_field_range_destroy(unit, range);
                    entry = 
                        dos_attack_data[unit][0].entry_id[DOS_ATTACK_CASE_TCP_FLAG_1];
                    stat_id = dos_attack_data[unit][0].stat_id[DOS_ATTACK_CASE_TCP_FLAG_1];
                    if ((stat_id != 0) && (entry != 0)) {
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_field_entry_stat_detach(unit, entry, stat_id));
                    }
                    if (stat_id != 0) {
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_field_stat_destroy(unit, stat_id));
                    }
                    if (entry) {
                        rv = bcm_robo_field_entry_destroy(unit, entry);
                    }
                    if ((rv != BCM_E_NONE) && (rv != BCM_E_NOT_FOUND)) {
                        return rv;
                    }
                    entry = 
                        dos_attack_data[unit][0].entry_id[DOS_ATTACK_CASE_TCP_FLAG_2];
                    stat_id = dos_attack_data[unit][0].stat_id[DOS_ATTACK_CASE_TCP_FLAG_2];
                    if ((stat_id != 0) && (entry != 0)) {
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_field_entry_stat_detach(unit, entry, stat_id));
                    }
                    if (stat_id != 0) {
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_field_stat_destroy(unit, stat_id));
                    }
                    if (entry) {
                        rv = bcm_robo_field_entry_destroy(unit, entry);
                    }
                    if ((rv != BCM_E_NONE) && (rv != BCM_E_NOT_FOUND)) {
                        return rv;
                    }
                    entry = 
                        dos_attack_data[unit][0].entry_id[DOS_ATTACK_CASE_TCP_FLAG_3];
                    stat_id = dos_attack_data[unit][0].stat_id[DOS_ATTACK_CASE_TCP_FLAG_3];
                    if ((stat_id != 0) && (entry != 0)) {
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_field_entry_stat_detach(unit, entry, stat_id));
                    }
                    if (stat_id != 0) {
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_field_stat_destroy(unit, stat_id));
                    }
                    if (entry) {
                        rv = bcm_robo_field_entry_destroy(unit, entry);
                    }
                    if ((rv != BCM_E_NONE) && (rv != BCM_E_NOT_FOUND)) {
                        return rv;
                    }
                    entry = 
                        dos_attack_data[unit][0].entry_id[DOS_ATTACK_CASE_TCP_FLAG_4];
                    stat_id = dos_attack_data[unit][0].stat_id[DOS_ATTACK_CASE_TCP_FLAG_4];
                    if ((stat_id != 0) && (entry != 0)) {
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_field_entry_stat_detach(unit, entry, stat_id));
                    }
                    if (stat_id != 0) {
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_field_stat_destroy(unit, stat_id));
                    }
                    if (entry) {
                        rv = bcm_robo_field_entry_destroy(unit, entry);
                    }
                    if ((rv != BCM_E_NONE) && (rv != BCM_E_NOT_FOUND)) {
                        return rv;
                    }

                    for (x = 0; x < SOC_MAX_NUM_PORTS; x++) {
                        dos_attack_data[unit][x].entry_id[DOS_ATTACK_CASE_TCP_FLAG_1] = 0;
                        dos_attack_data[unit][x].entry_id[DOS_ATTACK_CASE_TCP_FLAG_2] = 0;
                        dos_attack_data[unit][x].entry_id[DOS_ATTACK_CASE_TCP_FLAG_3] = 0;
                        dos_attack_data[unit][x].entry_id[DOS_ATTACK_CASE_TCP_FLAG_4] = 0;

                        dos_attack_data[unit][x].stat_id[DOS_ATTACK_CASE_TCP_FLAG_1] = 0;
                        dos_attack_data[unit][x].stat_id[DOS_ATTACK_CASE_TCP_FLAG_2] = 0;
                        dos_attack_data[unit][x].stat_id[DOS_ATTACK_CASE_TCP_FLAG_3] = 0;
                        dos_attack_data[unit][x].stat_id[DOS_ATTACK_CASE_TCP_FLAG_4] = 0;

                        dos_attack_data[unit][x].attacks &= ~DOS_ATTACK_TCP_FLAGS;
                    }
                }
            }
            break;
        case bcmSwitchDosAttackTcpFlagsSF:
            if (SOC_IS_TBX(unit)){
                /* TCP Control in TB is unavail */
                rv = BCM_E_UNAVAIL;
                return rv;
            }
            if (param) {
                /*
                 * Case 4: TCP SYN and FIN = 1
                 */
                if (port != DOS_ATTACK_ALL_PORT) {
                    entry = dos_attack_data[unit][port].entry_id[DOS_ATTACK_CASE_TCP_FLAG_4];
                } else {
                    entry = dos_attack_data[unit][0].entry_id[DOS_ATTACK_CASE_TCP_FLAG_4];
                }

                if (!entry) {
                    rv = bcm_robo_field_entry_create(unit, group, &entry);
                    if (rv < 0) {
                        return rv;
                    }
                    rv = bcm_robo_field_entry_prio_set(unit, entry, 
                        BCM_FIELD_ENTRY_PRIO_HIGHEST);
                    if (rv < 0) {
                        return rv;
                    }
    
                    if (port != DOS_ATTACK_ALL_PORT) {
                        dos_attack_data[unit][port].entry_id[DOS_ATTACK_CASE_TCP_FLAG_4] = entry;
    
                        if (SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit) ||
                            SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) || 
                            SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
                            SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
                            BCM_PBMP_PORT_ADD(pbm, port);
                            BCM_IF_ERROR_RETURN(
                                bcm_robo_field_qualify_InPorts
                                (unit, entry, pbm, pbm_mask));
                        }
                    } else {
                        for (x = 0; x < SOC_MAX_NUM_PORTS; x++) {
                            dos_attack_data[unit][x].entry_id[DOS_ATTACK_CASE_TCP_FLAG_4] = entry;
                        }
                    }
    
                    BCM_IF_ERROR_RETURN(
                        bcm_robo_field_qualify_TcpControl(unit, entry, 
                                                 TCP_FLAG_SYN | TCP_FLAG_FIN, 
                                                 TCP_FLAG_MASK));
                    BCM_IF_ERROR_RETURN(
                        bcm_robo_field_action_add(unit, entry, bcmFieldActionDrop, 0, 0));
                    if (port != DOS_ATTACK_ALL_PORT) {
                        x = port;
                    } else {
                        x = 0; /* port 0 */
                    }
                    stat_id = dos_attack_data[unit][x].stat_id[DOS_ATTACK_CASE_TCP_FLAG_4];
                    if (stat_id == 0) {
                        dos_attack_data[unit][x].stat_arr[DOS_ATTACK_CASE_TCP_FLAG_4] = bcmFieldStatPackets;
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_field_stat_create(unit, group, 1, 
                                &dos_attack_data[unit][x].stat_arr[DOS_ATTACK_CASE_TCP_FLAG_4], 
                                &stat_id));
                        dos_attack_data[unit][x].stat_id[DOS_ATTACK_CASE_TCP_FLAG_4] = stat_id;
                    }
                    BCM_IF_ERROR_RETURN(
                        bcm_robo_field_entry_stat_attach(unit, entry, stat_id));
                    if (port != DOS_ATTACK_ALL_PORT) {
                        dos_attack_data[unit][port].stat_id[DOS_ATTACK_CASE_TCP_FLAG_4] = stat_id;
                    } else {
                        for (x = 0; x < SOC_MAX_NUM_PORTS; x++) {
                            dos_attack_data[unit][x].stat_id[DOS_ATTACK_CASE_TCP_FLAG_4] = stat_id;
                        }
                    }
                    BCM_IF_ERROR_RETURN(
                        bcm_robo_field_entry_install(unit, entry));
                }
                if (port != DOS_ATTACK_ALL_PORT) {
                    dos_attack_data[unit][port].attacks |= DOS_ATTACK_TCP_FLAGS_SF;
                    dos_attack_data[unit][port].attacks &= ~DOS_ATTACK_TO_CPU;
                } else {
                    for (x = 0; x < SOC_MAX_NUM_PORTS; x++) {
                        dos_attack_data[unit][x].attacks |= DOS_ATTACK_TCP_FLAGS_SF;
                        dos_attack_data[unit][x].attacks &= ~DOS_ATTACK_TO_CPU;
                    }
                }
            } else {
                if (port != DOS_ATTACK_ALL_PORT) {
                    entry = 
                        dos_attack_data[unit][port].entry_id[DOS_ATTACK_CASE_TCP_FLAG_4];
                    stat_id = dos_attack_data[unit][port].stat_id[DOS_ATTACK_CASE_TCP_FLAG_4];
                    if ((stat_id != 0) && (entry != 0)) {
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_field_entry_stat_detach(unit, entry, stat_id));
                    }
                    if (stat_id != 0) {
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_field_stat_destroy(unit, stat_id));
                    }
                    dos_attack_data[unit][port].stat_id[DOS_ATTACK_CASE_TCP_FLAG_4] = 0;
                    if (entry) {
                        rv = bcm_robo_field_entry_destroy(unit, entry);
                    }
                    if ((rv == BCM_E_NONE) || (rv == BCM_E_NOT_FOUND)) {
                        dos_attack_data[unit][port].entry_id[DOS_ATTACK_CASE_TCP_FLAG_4] = 0;

                        dos_attack_data[unit][port].attacks &= ~DOS_ATTACK_TCP_FLAGS_SF;
                    } else {
                        return rv;
                    }
                } else {
                    /*
                     * Since DOS_ATTACK_ALL_PORT set all port's data base same
                     * value, retrieve port 0's entry id to remove entry.
                     */
                    entry = 
                        dos_attack_data[unit][0].entry_id[DOS_ATTACK_CASE_TCP_FLAG_4];
                    stat_id = dos_attack_data[unit][0].stat_id[DOS_ATTACK_CASE_TCP_FLAG_4];
                    if ((stat_id != 0) && (entry != 0)) {
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_field_entry_stat_detach(unit, entry, stat_id));
                    }
                    if (stat_id != 0) {
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_field_stat_destroy(unit, stat_id));
                    }
                    if (entry) {
                        rv = bcm_robo_field_entry_destroy(unit, entry);
                    }
                    if ((rv == BCM_E_NONE) || (rv == BCM_E_NOT_FOUND)) {
                        for (x = 0; x < SOC_MAX_NUM_PORTS; x++) {
                            dos_attack_data[unit][x].entry_id[DOS_ATTACK_CASE_TCP_FLAG_4] = 0;
                            dos_attack_data[unit][x].stat_id[DOS_ATTACK_CASE_TCP_FLAG_4] = 0;
                            dos_attack_data[unit][x].attacks &= ~DOS_ATTACK_TCP_FLAGS_SF;
                        }
                    } else {
                        return rv;
                    }
                }
            }
            break;
        case bcmSwitchDosAttackTcpFlagsFUP:
            if (SOC_IS_TBX(unit)){
                /* TCP Control in TB is unavail */
                rv = BCM_E_UNAVAIL;
                return rv;
            }
            if (param) {
                /*
                 * Case 3: TCP FIN, URG, PSH = 1
                 */
                if (port != DOS_ATTACK_ALL_PORT) {
                    entry = dos_attack_data[unit][port].entry_id[DOS_ATTACK_CASE_TCP_FLAG_3];
                } else {
                    entry = dos_attack_data[unit][0].entry_id[DOS_ATTACK_CASE_TCP_FLAG_3];
                }

                if (!entry) {
                    rv = bcm_robo_field_entry_create(unit, group, &entry);
                    if (rv < 0) {
                        return rv;
                    }
                    rv = bcm_robo_field_entry_prio_set(unit, entry, 
                        BCM_FIELD_ENTRY_PRIO_HIGHEST);
                    if (rv < 0) {
                        return rv;
                    }
    
                    if (port != DOS_ATTACK_ALL_PORT) {
                        dos_attack_data[unit][port].entry_id[DOS_ATTACK_CASE_TCP_FLAG_3] = entry;
    
                        if (SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit) || 
                            SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) || 
                            SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) || 
                            SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
                            BCM_PBMP_PORT_ADD(pbm, port);
                            BCM_IF_ERROR_RETURN(
                                bcm_robo_field_qualify_InPorts
                                (unit, entry, pbm, pbm_mask));
                        }
                    } else {
                        for (x = 0; x < SOC_MAX_NUM_PORTS; x++) {
                            dos_attack_data[unit][x].entry_id[DOS_ATTACK_CASE_TCP_FLAG_3] = entry;
                        }
                    }
    
                    BCM_IF_ERROR_RETURN(
                        bcm_robo_field_qualify_TcpControl(unit, entry, 
                                                 TCP_FLAG_FIN | TCP_FLAG_URG | TCP_FLAG_PSH, 
                                                 TCP_FLAG_MASK));
                    BCM_IF_ERROR_RETURN(
                        bcm_robo_field_action_add(unit, entry, bcmFieldActionDrop, 0, 0));
                    if (port != DOS_ATTACK_ALL_PORT) {
                        x = port;
                    } else {
                        x = 0; /* port 0 */
                    }
                    stat_id = dos_attack_data[unit][x].stat_id[DOS_ATTACK_CASE_TCP_FLAG_3];
                    if (stat_id == 0) {
                        dos_attack_data[unit][x].stat_arr[DOS_ATTACK_CASE_TCP_FLAG_3] = bcmFieldStatPackets;
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_field_stat_create(unit, group, 1, 
                                &dos_attack_data[unit][x].stat_arr[DOS_ATTACK_CASE_TCP_FLAG_3], 
                                &stat_id));
                        dos_attack_data[unit][x].stat_id[DOS_ATTACK_CASE_TCP_FLAG_3] = stat_id;
                    }
                    BCM_IF_ERROR_RETURN(
                        bcm_robo_field_entry_stat_attach(unit, entry, stat_id));
                    if (port != DOS_ATTACK_ALL_PORT) {
                        dos_attack_data[unit][port].stat_id[DOS_ATTACK_CASE_TCP_FLAG_3] = stat_id;
                    } else {
                        for (x = 0; x < SOC_MAX_NUM_PORTS; x++) {
                            dos_attack_data[unit][x].stat_id[DOS_ATTACK_CASE_TCP_FLAG_3] = stat_id;
                        }
                    }
                    BCM_IF_ERROR_RETURN(
                        bcm_robo_field_entry_install(unit, entry));
                }
                if (port != DOS_ATTACK_ALL_PORT) {
                    dos_attack_data[unit][port].attacks |= DOS_ATTACK_TCP_FLAGS_FUP;
                    dos_attack_data[unit][port].attacks &= ~DOS_ATTACK_TO_CPU;
                } else {
                    for (x = 0; x < SOC_MAX_NUM_PORTS; x++) {
                        dos_attack_data[unit][x].attacks |= DOS_ATTACK_TCP_FLAGS_FUP;
                        dos_attack_data[unit][x].attacks &= ~DOS_ATTACK_TO_CPU;
                    }
                }
            } else {
                if (port != DOS_ATTACK_ALL_PORT) {
                    entry = 
                        dos_attack_data[unit][port].entry_id[DOS_ATTACK_CASE_TCP_FLAG_3];
                    stat_id = dos_attack_data[unit][port].stat_id[DOS_ATTACK_CASE_TCP_FLAG_3];
                    if ((stat_id != 0) && (entry != 0)) {
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_field_entry_stat_detach(unit, entry, stat_id));
                    }
                    if (stat_id != 0) {
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_field_stat_destroy(unit, stat_id));
                    }
                    dos_attack_data[unit][port].stat_id[DOS_ATTACK_CASE_TCP_FLAG_3] = 0;
                    if (entry) {
                        rv = bcm_robo_field_entry_destroy(unit, entry);
                    }
                    if ((rv == BCM_E_NONE) || (rv == BCM_E_NOT_FOUND)) {
                        dos_attack_data[unit][port].entry_id[DOS_ATTACK_CASE_TCP_FLAG_3] = 0;

                        dos_attack_data[unit][port].attacks &= ~DOS_ATTACK_TCP_FLAGS_FUP;
                    } else {
                        return rv;
                    }
                } else {
                    /*
                     * Since DOS_ATTACK_ALL_PORT set all port's data base same
                     * value, retrieve port 0's entry id to remove entry.
                     */
                    entry = 
                        dos_attack_data[unit][0].entry_id[DOS_ATTACK_CASE_TCP_FLAG_3];
                    stat_id = dos_attack_data[unit][0].stat_id[DOS_ATTACK_CASE_TCP_FLAG_3];
                    if ((stat_id != 0) && (entry != 0)) {
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_field_entry_stat_detach(unit, entry, stat_id));
                    }
                    if (stat_id != 0) {
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_field_stat_destroy(unit, stat_id));
                    }
                    if (entry) {
                        rv = bcm_robo_field_entry_destroy(unit, entry);
                    }
                    if ((rv == BCM_E_NONE) || (rv == BCM_E_NOT_FOUND)) {
                        for (x = 0; x < SOC_MAX_NUM_PORTS; x++) {
                            dos_attack_data[unit][x].entry_id[DOS_ATTACK_CASE_TCP_FLAG_3] = 0;
                            dos_attack_data[unit][x].stat_id[DOS_ATTACK_CASE_TCP_FLAG_3] = 0;
                            dos_attack_data[unit][x].attacks &= ~DOS_ATTACK_TCP_FLAGS_FUP;
                        }
                    } else {
                        return rv;
                    }
                }
            }
            break;
        case bcmSwitchDosAttackSynFrag:
            if (SOC_IS_TBX(unit)){
                /* TCP Control in TB is unavail */
                rv = BCM_E_UNAVAIL;
                return rv;
            }
            if (param) {
                /*
                 * Case 1: TCP SYN flag = 1 & source port < 1024
                 */
                if (port != DOS_ATTACK_ALL_PORT) {
                    entry = dos_attack_data[unit][port].entry_id[DOS_ATTACK_CASE_TCP_FLAG_1];
                } else {
                    entry = dos_attack_data[unit][0].entry_id[DOS_ATTACK_CASE_TCP_FLAG_1];
                }

                if (!entry) {
                    rv = bcm_robo_field_entry_create(unit, group, &entry);
                    if (rv < 0) {
                        return rv;
                    }
                    rv = bcm_robo_field_entry_prio_set(unit, entry, 
                        BCM_FIELD_ENTRY_PRIO_HIGHEST);
                    if (rv < 0) {
                        return rv;
                    }
    
                    if (port != DOS_ATTACK_ALL_PORT) {
                        dos_attack_data[unit][port].entry_id[DOS_ATTACK_CASE_TCP_FLAG_1] = entry;
    
                        if (SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit) || 
                            SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) || 
                            SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit)|| 
                            SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
                            BCM_PBMP_PORT_ADD(pbm, port);
                            BCM_IF_ERROR_RETURN(
                                bcm_robo_field_qualify_InPorts
                                (unit, entry, pbm, pbm_mask));
                        } else {
                            /* Can't configure source port < 1024 */
                            return BCM_E_UNAVAIL;
                        }
                    } else {
                        for (x = 0; x < SOC_MAX_NUM_PORTS; x++) {
                            dos_attack_data[unit][x].entry_id[DOS_ATTACK_CASE_TCP_FLAG_1] = entry;
                        }
                    }
    
                    /* SDK-33137 : 
                     *  - The qualifier for the DOS at TCP_SYN erorr are 
                     *    {SYN=1, ACK=0, L4_Src_Port<1024}
                     */
                    BCM_IF_ERROR_RETURN(
                        bcm_robo_field_qualify_TcpControl(unit, entry, 
                                TCP_FLAG_SYN, (TCP_FLAG_SYN | TCP_FLAG_ACK)));
                    
                    if (SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) ||
                        SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) || 
                        SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_field_qualify_L4SrcPort(unit, 
                                entry, 0x0, 0xFC00));
                    } else {
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_field_range_create(unit, &range, BCM_FIELD_RANGE_SRCPORT, 0, 1023));
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_field_qualify_RangeCheck(unit, entry, range, 0));
        
                            if (port != DOS_ATTACK_ALL_PORT) {
                                    dos_attack_data[unit][port].range_id = range;
                            } else {
                                for (x = 0; x < SOC_MAX_NUM_PORTS; x++) {
                                    dos_attack_data[unit][x].range_id = range;
                                }
                            }
                    }
                    BCM_IF_ERROR_RETURN(
                        bcm_robo_field_action_add(unit, entry, bcmFieldActionDrop, 0, 0));
                    if (port != DOS_ATTACK_ALL_PORT) {
                        x = port;
                    } else {
                        x = 0; /* port 0 */
                    }
                    stat_id = dos_attack_data[unit][x].stat_id[DOS_ATTACK_CASE_TCP_FLAG_1];
                    if (stat_id == 0) {
                        dos_attack_data[unit][x].stat_arr[DOS_ATTACK_CASE_TCP_FLAG_1] = bcmFieldStatPackets;
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_field_stat_create(unit, group, 1, 
                                &dos_attack_data[unit][x].stat_arr[DOS_ATTACK_CASE_TCP_FLAG_1], 
                                &stat_id));
                        dos_attack_data[unit][x].stat_id[DOS_ATTACK_CASE_TCP_FLAG_1] = stat_id;
                    }
                    BCM_IF_ERROR_RETURN(
                        bcm_robo_field_entry_stat_attach(unit, entry, stat_id));
                    if (port != DOS_ATTACK_ALL_PORT) {
                        dos_attack_data[unit][port].stat_id[DOS_ATTACK_CASE_TCP_FLAG_1] = stat_id;
                    } else {
                        for (x = 0; x < SOC_MAX_NUM_PORTS; x++) {
                            dos_attack_data[unit][x].stat_id[DOS_ATTACK_CASE_TCP_FLAG_1] = stat_id;
                        }
                    }
                    BCM_IF_ERROR_RETURN(
                        bcm_robo_field_entry_install(unit, entry));
                }
                if (port != DOS_ATTACK_ALL_PORT) {
                    dos_attack_data[unit][port].attacks |= DOS_ATTACK_SYN_FRAG;
                    dos_attack_data[unit][port].attacks &= ~DOS_ATTACK_TO_CPU;
                } else {
                    for (x = 0; x < SOC_MAX_NUM_PORTS; x++) {
                        dos_attack_data[unit][x].attacks |= DOS_ATTACK_SYN_FRAG;
                        dos_attack_data[unit][x].attacks &= ~DOS_ATTACK_TO_CPU;
                    }
                }
            } else {
                if (port != DOS_ATTACK_ALL_PORT) {
                    range = dos_attack_data[unit][port].range_id;
                    rv = bcm_robo_field_range_destroy(unit, range);
                    entry = 
                        dos_attack_data[unit][port].entry_id[DOS_ATTACK_CASE_TCP_FLAG_1];
                    stat_id = dos_attack_data[unit][port].stat_id[DOS_ATTACK_CASE_TCP_FLAG_1];
                    if ((stat_id != 0) && (entry != 0)) {
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_field_entry_stat_detach(unit, entry, stat_id));
                    }
                    if (stat_id != 0) {
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_field_stat_destroy(unit, stat_id));
                    }
                    dos_attack_data[unit][port].stat_id[DOS_ATTACK_CASE_TCP_FLAG_1] = 0;
                    if (entry) {
                        rv = bcm_robo_field_entry_destroy(unit, entry);
                    }
                    if ((rv == BCM_E_NONE) || (rv == BCM_E_NOT_FOUND)) {
                        dos_attack_data[unit][port].entry_id[DOS_ATTACK_CASE_TCP_FLAG_1] = 0;

                        dos_attack_data[unit][port].attacks &= ~DOS_ATTACK_SYN_FRAG;
                    } else {
                        return rv;
                    }
                } else {
                    /*
                     * Since DOS_ATTACK_ALL_PORT set all port's data base same
                     * value, retrieve port 0's entry id to remove entry.
                     */
                    range = dos_attack_data[unit][0].range_id;
                    rv = bcm_robo_field_range_destroy(unit, range);
                    entry = 
                        dos_attack_data[unit][0].entry_id[DOS_ATTACK_CASE_TCP_FLAG_1];
                    stat_id = dos_attack_data[unit][0].stat_id[DOS_ATTACK_CASE_TCP_FLAG_1];
                    if ((stat_id != 0) && (entry != 0)) {
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_field_entry_stat_detach(unit, entry, stat_id));
                    }
                    if (stat_id != 0) {
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_field_stat_destroy(unit, stat_id));
                    }
                    if (entry) {
                        rv = bcm_robo_field_entry_destroy(unit, entry);
                    }
                    if ((rv == BCM_E_NONE) || (rv == BCM_E_NOT_FOUND)) {
                        for (x = 0; x < SOC_MAX_NUM_PORTS; x++) {
                            dos_attack_data[unit][x].entry_id[DOS_ATTACK_CASE_TCP_FLAG_1] = 0;
                            dos_attack_data[unit][x].stat_id[DOS_ATTACK_CASE_TCP_FLAG_1] = 0;
                            dos_attack_data[unit][x].attacks &= ~DOS_ATTACK_SYN_FRAG;
                        }
                    } else {
                        return rv;
                    }
                }
            }
            break;
        case bcmSwitchDosAttackFlagZeroSeqZero:
            if (SOC_IS_TBX(unit)){
                /* TCP Control in TB is unavail */
                rv = BCM_E_UNAVAIL;
                return rv;
            }
            if (param) {
                /*
                 * Case 2: TCP flag = 0
                 */
                if (port != DOS_ATTACK_ALL_PORT) {
                    entry = dos_attack_data[unit][port].entry_id[DOS_ATTACK_CASE_TCP_FLAG_2];
                } else {
                    entry = dos_attack_data[unit][0].entry_id[DOS_ATTACK_CASE_TCP_FLAG_2];
                }

                if (!entry) {
                    rv = bcm_robo_field_entry_create(unit, group, &entry);
                    if (rv < 0) {
                        return rv;
                    }
                    rv = bcm_robo_field_entry_prio_set(unit, entry, 
                        BCM_FIELD_ENTRY_PRIO_HIGHEST);
                    if (rv < 0) {
                        return rv;
                    }
    
                    if (port != DOS_ATTACK_ALL_PORT) {
                        dos_attack_data[unit][port].entry_id[DOS_ATTACK_CASE_TCP_FLAG_2] = entry;
    
                        if (SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit) || 
                            SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) || 
                            SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) || 
                            SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
                            BCM_PBMP_PORT_ADD(pbm, port);
                            BCM_IF_ERROR_RETURN(
                                bcm_robo_field_qualify_InPorts
                                (unit, entry, pbm, pbm_mask));
                        }
                    } else {
                        for (x = 0; x < SOC_MAX_NUM_PORTS; x++) {
                            dos_attack_data[unit][x].entry_id[DOS_ATTACK_CASE_TCP_FLAG_2] = entry;
                        }
                    }
    
                    BCM_IF_ERROR_RETURN(
                        bcm_robo_field_qualify_TcpControl(unit, entry, 0x0, TCP_FLAG_MASK));
                    BCM_IF_ERROR_RETURN(
                        bcm_robo_field_action_add(unit, entry, bcmFieldActionDrop, 0, 0));
                    if (port != DOS_ATTACK_ALL_PORT) {
                        x = port;
                    } else {
                        x = 0; /* port 0 */
                    }
                    stat_id = dos_attack_data[unit][x].stat_id[DOS_ATTACK_CASE_TCP_FLAG_2];
                    if (stat_id == 0) {
                        dos_attack_data[unit][x].stat_arr[DOS_ATTACK_CASE_TCP_FLAG_2] = bcmFieldStatPackets;
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_field_stat_create(unit, group, 1, 
                                &dos_attack_data[unit][x].stat_arr[DOS_ATTACK_CASE_TCP_FLAG_2], 
                                &stat_id));
                        dos_attack_data[unit][x].stat_id[DOS_ATTACK_CASE_TCP_FLAG_2] = stat_id;
                    }
                    BCM_IF_ERROR_RETURN(
                        bcm_robo_field_entry_stat_attach(unit, entry, stat_id));
                    if (port != DOS_ATTACK_ALL_PORT) {
                        dos_attack_data[unit][port].stat_id[DOS_ATTACK_CASE_TCP_FLAG_2] = stat_id;
                    } else {
                        for (x = 0; x < SOC_MAX_NUM_PORTS; x++) {
                            dos_attack_data[unit][x].stat_id[DOS_ATTACK_CASE_TCP_FLAG_2] = stat_id;
                        }
                    }
                    BCM_IF_ERROR_RETURN(
                        bcm_robo_field_entry_install(unit, entry));
                }
                if (port != DOS_ATTACK_ALL_PORT) {
                    dos_attack_data[unit][port].attacks |= DOS_ATTACK_FLAG_ZERO_SEQ_ZERO;
                    dos_attack_data[unit][port].attacks &= ~DOS_ATTACK_TO_CPU;
                } else {
                    for (x = 0; x < SOC_MAX_NUM_PORTS; x++) {
                        dos_attack_data[unit][x].attacks |= DOS_ATTACK_FLAG_ZERO_SEQ_ZERO;
                        dos_attack_data[unit][x].attacks &= ~DOS_ATTACK_TO_CPU;
                    }
                }
            } else {
                if (port != DOS_ATTACK_ALL_PORT) {
                    entry = 
                        dos_attack_data[unit][port].entry_id[DOS_ATTACK_CASE_TCP_FLAG_2];
                    stat_id = dos_attack_data[unit][port].stat_id[DOS_ATTACK_CASE_TCP_FLAG_2];
                    if ((stat_id != 0) && (entry != 0)) {
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_field_entry_stat_detach(unit, entry, stat_id));
                    }
                    if (stat_id != 0) {
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_field_stat_destroy(unit, stat_id));
                    }
                    dos_attack_data[unit][port].stat_id[DOS_ATTACK_CASE_TCP_FLAG_2] = 0;
                    if (entry) {
                        rv = bcm_robo_field_entry_destroy(unit, entry);
                    }
                    if ((rv == BCM_E_NONE) || (rv == BCM_E_NOT_FOUND)) {
                        dos_attack_data[unit][port].entry_id[DOS_ATTACK_CASE_TCP_FLAG_2] = 0;

                        dos_attack_data[unit][port].attacks &= ~DOS_ATTACK_FLAG_ZERO_SEQ_ZERO;
                    } else {
                        return rv;
                    }
                } else {
                    /*
                     * Since DOS_ATTACK_ALL_PORT set all port's data base same
                     * value, retrieve port 0's entry id to remove entry.
                     */
                    entry = 
                        dos_attack_data[unit][0].entry_id[DOS_ATTACK_CASE_TCP_FLAG_2];
                    stat_id = dos_attack_data[unit][0].stat_id[DOS_ATTACK_CASE_TCP_FLAG_2];
                    if ((stat_id != 0) && (entry != 0)) {
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_field_entry_stat_detach(unit, entry, stat_id));
                    }
                    if (stat_id != 0) {
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_field_stat_destroy(unit, stat_id));
                    }
                    if (entry) {
                        rv = bcm_robo_field_entry_destroy(unit, entry);
                    }
                    if ((rv == BCM_E_NONE) || (rv == BCM_E_NOT_FOUND)) {
                        for (x = 0; x < SOC_MAX_NUM_PORTS; x++) {
                            dos_attack_data[unit][x].entry_id[DOS_ATTACK_CASE_TCP_FLAG_2] = 0;
                            dos_attack_data[unit][x].stat_id[DOS_ATTACK_CASE_TCP_FLAG_2] = 0;
                            dos_attack_data[unit][x].attacks &= ~DOS_ATTACK_FLAG_ZERO_SEQ_ZERO;
                        }
                    } else {
                        return rv;
                    }
                }
            }
            break;
        case bcmSwitchDosAttackL4Port:
            if (SOC_IS_VULCAN(unit) || SOC_IS_TBX(unit) || 
                SOC_IS_STARFIGHTER(unit) || SOC_IS_POLAR(unit) || 
                SOC_IS_NORTHSTAR(unit) || SOC_IS_NORTHSTARPLUS(unit) ||
                SOC_IS_STARFIGHTER3(unit)) {
                rv = BCM_E_UNAVAIL;
                return rv;
            }
            if (param) {
                if (port != DOS_ATTACK_ALL_PORT) {
                    entry = dos_attack_data[unit][port].entry_id[DOS_ATTACK_CASE_L4_PORT];
                } else {
                    entry = dos_attack_data[unit][0].entry_id[DOS_ATTACK_CASE_L4_PORT];
                }

                if (!entry) {
                    rv = bcm_robo_field_entry_create(unit, group, &entry);
                    if (rv < 0) {
                        return rv;
                    }
                    rv = bcm_robo_field_entry_prio_set(unit, entry, 
                        BCM_FIELD_ENTRY_PRIO_HIGHEST);
                    if (rv < 0) {
                        return rv;
                    }
    
                    if (port != DOS_ATTACK_ALL_PORT) {
                        dos_attack_data[unit][port].entry_id[DOS_ATTACK_CASE_L4_PORT] = entry;
    
                        if (SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit)) {
                            BCM_PBMP_PORT_ADD(pbm, port);
                            BCM_IF_ERROR_RETURN(
                                bcm_robo_field_qualify_InPorts
                                (unit, entry, pbm, pbm_mask));
                        }
                    } else {
                        for (x = 0; x < SOC_MAX_NUM_PORTS; x++) {
                            dos_attack_data[unit][x].entry_id[DOS_ATTACK_CASE_L4_PORT] = entry;
                        }
                    }
    
                    BCM_IF_ERROR_RETURN(
                        bcm_robo_field_qualify_EqualL4Port(unit, entry, 0x1));
                    BCM_IF_ERROR_RETURN(
                        bcm_robo_field_action_add(unit, entry, bcmFieldActionDrop, 0, 0));
                    if (port != DOS_ATTACK_ALL_PORT) {
                        x = port;
                    } else {
                        x = 0; /* port 0 */
                    }
                    stat_id = dos_attack_data[unit][x].stat_id[DOS_ATTACK_CASE_L4_PORT];
                    if (stat_id == 0) {
                        dos_attack_data[unit][x].stat_arr[DOS_ATTACK_CASE_L4_PORT] = bcmFieldStatPackets;
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_field_stat_create(unit, group, 1, 
                                &dos_attack_data[unit][x].stat_arr[DOS_ATTACK_CASE_L4_PORT], 
                                &stat_id));
                        dos_attack_data[unit][x].stat_id[DOS_ATTACK_CASE_L4_PORT] = stat_id;
                        
                    }
                    BCM_IF_ERROR_RETURN(
                        bcm_robo_field_entry_stat_attach(unit, entry, stat_id));
                    if (port != DOS_ATTACK_ALL_PORT) {
                        dos_attack_data[unit][port].stat_id[DOS_ATTACK_CASE_L4_PORT] = stat_id;
                    } else {
                        for (x = 0; x < SOC_MAX_NUM_PORTS; x++) {
                            dos_attack_data[unit][x].stat_id[DOS_ATTACK_CASE_L4_PORT] = stat_id;
                        }
                    }
                    BCM_IF_ERROR_RETURN(
                        bcm_robo_field_entry_install(unit, entry));
                }
                if (port != DOS_ATTACK_ALL_PORT) {
                    dos_attack_data[unit][port].attacks |= DOS_ATTACK_L4_PORT;
                    dos_attack_data[unit][port].attacks &= ~DOS_ATTACK_TO_CPU;
                } else {
                    for (x = 0; x < SOC_MAX_NUM_PORTS; x++) {
                        dos_attack_data[unit][x].attacks |= DOS_ATTACK_L4_PORT;
                        dos_attack_data[unit][x].attacks &= ~DOS_ATTACK_TO_CPU;
                    }
                }
            } else {
                if (port != DOS_ATTACK_ALL_PORT) {
                    entry = 
                        dos_attack_data[unit][port].entry_id[DOS_ATTACK_CASE_L4_PORT];
                    stat_id = dos_attack_data[unit][port].stat_id[DOS_ATTACK_CASE_L4_PORT];
                    if ((stat_id != 0) && (entry != 0)) {
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_field_entry_stat_detach(unit, entry, stat_id));
                    }
                    if (stat_id != 0) {
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_field_stat_destroy(unit, stat_id));
                    }
                    dos_attack_data[unit][port].stat_id[DOS_ATTACK_CASE_L4_PORT] = 0;
                    if (entry) {
                        rv = bcm_robo_field_entry_destroy(unit, entry);
                    }
                    if ((rv == BCM_E_NONE) || (rv == BCM_E_NOT_FOUND)) {
                        dos_attack_data[unit][port].entry_id[DOS_ATTACK_CASE_L4_PORT] = 0;
                        dos_attack_data[unit][port].attacks &= ~DOS_ATTACK_L4_PORT;
                    } else {
                        return rv;
                    }
                } else {
                    /*
                     * Since DOS_ATTACK_ALL_PORT set all port's data base same
                     * value, retrieve port 0's entry id to remove entry.
                     */
                    entry = 
                        dos_attack_data[unit][0].entry_id[DOS_ATTACK_CASE_L4_PORT];
                    stat_id = dos_attack_data[unit][0].stat_id[DOS_ATTACK_CASE_L4_PORT];
                    if ((stat_id != 0) && (entry != 0)) {
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_field_entry_stat_detach(unit, entry, stat_id));
                    }
                    if (stat_id != 0) {
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_field_stat_destroy(unit, stat_id));
                    }
                    if (entry) {
                        rv = bcm_robo_field_entry_destroy(unit, entry);
                    }
                    if ((rv == BCM_E_NONE) || (rv == BCM_E_NOT_FOUND)) {
                        for (x = 0; x < SOC_MAX_NUM_PORTS; x++) {
                            dos_attack_data[unit][x].entry_id[DOS_ATTACK_CASE_L4_PORT] = 0;
                            dos_attack_data[unit][x].stat_id[DOS_ATTACK_CASE_L4_PORT] = 0;
                            dos_attack_data[unit][x].attacks &= ~DOS_ATTACK_L4_PORT;
                        }
                    } else {
                        return rv;
                    }
                }
            }
            break;
        case bcmSwitchDosAttackTcpFrag:
            if (SOC_IS_VULCAN(unit) || SOC_IS_TBX(unit) || 
                SOC_IS_STARFIGHTER(unit) || SOC_IS_POLAR(unit) || 
                SOC_IS_NORTHSTAR(unit) || SOC_IS_NORTHSTARPLUS(unit) ||
                SOC_IS_STARFIGHTER3(unit)) {
                rv = BCM_E_UNAVAIL;
                return rv;
            }

            /* TcpFrag DoS on the chips which support HW DoS featur:
             * - The Fragment_Offset=1 in any fragment of a fragmented IP 
             *      datagram carrying part of TCP data.
             *
             * This function is for the DOS feature implementation through 
             *  CFP on ROBO chips. And the qualify set on TCP Fragment offset 
             *  field in current ROBO CFP is not supported.
             */
            return BCM_E_UNAVAIL;
            
            break;
        case bcmSwitchDosAttackPingFlood:
            if (param) {
                if (port != DOS_ATTACK_ALL_PORT) {
                    entry = dos_attack_data[unit][port].entry_id[DOS_ATTACK_CASE_PING_FLOOD];
                } else {
                    entry = dos_attack_data[unit][0].entry_id[DOS_ATTACK_CASE_PING_FLOOD];
                }

                if (!entry) {
                    rv = bcm_robo_field_entry_create(unit, group, &entry);
                    if (rv < 0) {
                        return rv;
                    }
                    rv = bcm_robo_field_entry_prio_set(unit, entry, 
                        BCM_FIELD_ENTRY_PRIO_HIGHEST);
                    if (rv < 0) {
                        return rv;
                    }
    
                    if (port != DOS_ATTACK_ALL_PORT) {
                        dos_attack_data[unit][port].entry_id[DOS_ATTACK_CASE_PING_FLOOD] = entry;
    
                        if (SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit) || 
                            SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) || 
                            SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
                            SOC_IS_STARFIGHTER3(unit)) {
                            BCM_PBMP_PORT_ADD(pbm, port);
                            BCM_IF_ERROR_RETURN(
                                bcm_robo_field_qualify_InPorts
                                (unit, entry, pbm, pbm_mask));
                        } else if (SOC_IS_TBX(unit)){
                            BCM_IF_ERROR_RETURN(bcm_robo_field_qualify_InPort
                                    (unit, entry, port, 
                                    BCM_FIELD_EXACT_MATCH_MASK));
                        }
                    } else {
                        for (x = 0; x < SOC_MAX_NUM_PORTS; x++) {
                            dos_attack_data[unit][x].entry_id[DOS_ATTACK_CASE_PING_FLOOD] = entry;
                        }
                    }
    
                    BCM_IF_ERROR_RETURN(
                        bcm_robo_field_qualify_IpProtocol(unit, entry, IP_PROTOCOL_ID_ICMP, 0xff));             
                    if (port != DOS_ATTACK_ALL_PORT) {
                        x = port;
                    } else {
                        x = 0; /* port 0 */
                    }

                    pol_id = dos_attack_data[unit][x].pol_id[DOS_ATTACK_CASE_PING_FLOOD];
                    if (pol_id == 0) {
                        sal_memset(&pol_cfg, 0, sizeof(bcm_policer_config_t));
                        pol_cfg.mode = bcmPolicerModeCommitted;
                        pol_cfg.ckbits_burst = 1024;
                        pol_cfg.ckbits_sec = 64;
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_policer_create(unit, &pol_cfg, &pol_id));
                        dos_attack_data[unit][x].pol_id[DOS_ATTACK_CASE_PING_FLOOD] = pol_id;
                    }
                    BCM_IF_ERROR_RETURN(
                        bcm_robo_field_entry_policer_attach(unit, 
                            entry, 0, pol_id));

                    stat_id = dos_attack_data[unit][x].stat_id[DOS_ATTACK_CASE_PING_FLOOD];
                    if (stat_id == 0) {
                        dos_attack_data[unit][x].stat_arr[DOS_ATTACK_CASE_PING_FLOOD] = 
                            bcmFieldStatPackets;
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_field_stat_create(unit, group, 1, 
                                &dos_attack_data[unit][x].stat_arr[DOS_ATTACK_CASE_PING_FLOOD], 
                                &stat_id));
                    }
                    BCM_IF_ERROR_RETURN(
                        bcm_robo_field_entry_stat_attach(unit, 
                            entry, stat_id));
                    if (port != DOS_ATTACK_ALL_PORT) {
                        dos_attack_data[unit][port].stat_id[DOS_ATTACK_CASE_PING_FLOOD] = stat_id;
                        dos_attack_data[unit][port].pol_id[DOS_ATTACK_CASE_PING_FLOOD] = pol_id;
                    } else {
                        for (x = 0; x < SOC_MAX_NUM_PORTS; x++) {
                            dos_attack_data[unit][x].stat_id[DOS_ATTACK_CASE_PING_FLOOD] = stat_id;
                            dos_attack_data[unit][x].pol_id[DOS_ATTACK_CASE_PING_FLOOD] = pol_id;
                        }
                    }
                    BCM_IF_ERROR_RETURN(
                        bcm_robo_field_action_add(unit, entry, bcmFieldActionRpDrop, 0, 0));
                    BCM_IF_ERROR_RETURN(
                        bcm_robo_field_entry_install(unit, entry));
                }
                if (port != DOS_ATTACK_ALL_PORT) {
                    dos_attack_data[unit][port].attacks |= DOS_ATTACK_PING_FLOOD;
                    dos_attack_data[unit][port].attacks &= ~DOS_ATTACK_TO_CPU;
                } else {
                    for (x = 0; x < SOC_MAX_NUM_PORTS; x++) {
                        dos_attack_data[unit][x].attacks |= DOS_ATTACK_PING_FLOOD;
                        dos_attack_data[unit][x].attacks &= ~DOS_ATTACK_TO_CPU;
                    }
                }
            } else {
                if (port != DOS_ATTACK_ALL_PORT) {
                    entry = 
                        dos_attack_data[unit][port].entry_id[DOS_ATTACK_CASE_PING_FLOOD];
                    pol_id = dos_attack_data[unit][port].pol_id[DOS_ATTACK_CASE_PING_FLOOD];
                    stat_id = dos_attack_data[unit][port].stat_id[DOS_ATTACK_CASE_PING_FLOOD];
                    if ((pol_id != 0) && (entry != 0)) {
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_field_entry_policer_detach(unit, entry, 0));
                    }
                    if ((stat_id != 0) && (entry != 0)) {
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_field_entry_stat_detach(unit, entry, stat_id));
                    }
                    if (pol_id != 0) {
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_policer_destroy(unit, pol_id));
                    }
                    if (stat_id != 0) {
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_field_stat_destroy(unit, stat_id));
                    }
                    if (entry) {
                        rv = bcm_robo_field_entry_destroy(unit, entry);
                    }
                    if ((rv == BCM_E_NONE) || (rv == BCM_E_NOT_FOUND)) {
                        dos_attack_data[unit][port].entry_id[DOS_ATTACK_CASE_PING_FLOOD] = 0;
                        dos_attack_data[unit][port].pol_id[DOS_ATTACK_CASE_PING_FLOOD] = 0;
                        dos_attack_data[unit][port].stat_id[DOS_ATTACK_CASE_PING_FLOOD] = 0;
                        dos_attack_data[unit][port].attacks &= ~DOS_ATTACK_PING_FLOOD;
                    } else {
                        return rv;
                    }
                } else {
                    /*
                     * Since DOS_ATTACK_ALL_PORT set all port's data base same
                     * value, retrieve port 0's entry id to remove entry.
                     */
                    entry = 
                        dos_attack_data[unit][0].entry_id[DOS_ATTACK_CASE_PING_FLOOD];
                    pol_id = dos_attack_data[unit][0].pol_id[DOS_ATTACK_CASE_PING_FLOOD];
                    stat_id = dos_attack_data[unit][0].stat_id[DOS_ATTACK_CASE_PING_FLOOD];
                    if ((pol_id != 0) && (entry != 0)) {
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_field_entry_policer_detach(unit, entry, 0));
                    }
                    if ((stat_id != 0) && (entry != 0)) {
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_field_entry_stat_detach(unit, entry, stat_id));
                    }
                    if (pol_id != 0) {
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_policer_destroy(unit, pol_id));
                    }
                    if (stat_id != 0) {
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_field_stat_destroy(unit, stat_id));
                    }
                    if (entry) {
                        rv = bcm_robo_field_entry_destroy(unit, entry);
                    }
                    if ((rv == BCM_E_NONE) || (rv == BCM_E_NOT_FOUND)) {
                        for (x = 0; x < SOC_MAX_NUM_PORTS; x++) {
                            dos_attack_data[unit][x].entry_id[DOS_ATTACK_CASE_PING_FLOOD] = 0;
                            dos_attack_data[unit][x].pol_id[DOS_ATTACK_CASE_PING_FLOOD] = 0;
                            dos_attack_data[unit][x].stat_id[DOS_ATTACK_CASE_PING_FLOOD] = 0;
                            dos_attack_data[unit][x].attacks &= ~DOS_ATTACK_PING_FLOOD;
                        }
                    } else {
                        return rv;
                    }
                }
            }
            break;
        case bcmSwitchDosAttackSynFlood:
            if (SOC_IS_TBX(unit)){
                /* TCP Control in TB is unavail */
                rv = BCM_E_UNAVAIL;
                return rv;
            }
            if (param) {
                /* SYN Flood */
                if (port != DOS_ATTACK_ALL_PORT) {
                    entry = dos_attack_data[unit][port].entry_id[DOS_ATTACK_CASE_SYN_FLOOD];
                } else {
                    entry = dos_attack_data[unit][0].entry_id[DOS_ATTACK_CASE_SYN_FLOOD];
                }

                if (!entry) {
                    rv = bcm_robo_field_entry_create(unit, group, &entry);
                    if (rv < 0) {
                        return rv;
                    }
                    rv = bcm_robo_field_entry_prio_set(unit, entry, 
                        BCM_FIELD_ENTRY_PRIO_HIGHEST);
                    if (rv < 0) {
                        return rv;
                    }
    
                    if (port != DOS_ATTACK_ALL_PORT) {
                        dos_attack_data[unit][port].entry_id[DOS_ATTACK_CASE_SYN_FLOOD] = entry;
    
                        if (SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit) || 
                            SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) || 
                            SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) || 
                            SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
                            BCM_PBMP_PORT_ADD(pbm, port);
                            BCM_IF_ERROR_RETURN(
                                bcm_robo_field_qualify_InPorts
                                (unit, entry, pbm, pbm_mask));
                        }
                    } else {
                        for (x = 0; x < SOC_MAX_NUM_PORTS; x++) {
                            dos_attack_data[unit][x].entry_id[DOS_ATTACK_CASE_SYN_FLOOD] = entry;
                        }
                    }
    
                    BCM_IF_ERROR_RETURN(
                        bcm_robo_field_qualify_TcpControl(unit, entry, TCP_FLAG_SYN, TCP_FLAG_MASK));
                    if (port != DOS_ATTACK_ALL_PORT) {
                        x = port;
                    } else {
                        x = 0; /* port 0 */
                    }

                    pol_id = dos_attack_data[unit][x].pol_id[DOS_ATTACK_CASE_SYN_FLOOD];
                    if (pol_id == 0) {
                        sal_memset(&pol_cfg, 0, sizeof(bcm_policer_config_t));
                        pol_cfg.mode = bcmPolicerModeCommitted;
                        pol_cfg.ckbits_burst = 1024;
                        pol_cfg.ckbits_sec = 64;
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_policer_create(unit, &pol_cfg, &pol_id));
                        dos_attack_data[unit][x].pol_id[DOS_ATTACK_CASE_SYN_FLOOD] = pol_id;
                    }
                    BCM_IF_ERROR_RETURN(
                        bcm_robo_field_entry_policer_attach(unit, 
                            entry, 0, pol_id));

                    stat_id = dos_attack_data[unit][x].stat_id[DOS_ATTACK_CASE_SYN_FLOOD];
                    if (stat_id == 0) {
                        dos_attack_data[unit][x].stat_arr[DOS_ATTACK_CASE_SYN_FLOOD] = 
                            bcmFieldStatPackets;
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_field_stat_create(unit, group, 1, 
                                &dos_attack_data[unit][x].stat_arr[DOS_ATTACK_CASE_SYN_FLOOD], 
                                &stat_id));
                    }
                    BCM_IF_ERROR_RETURN(
                        bcm_robo_field_entry_stat_attach(unit, 
                            entry, stat_id));
                    if (port != DOS_ATTACK_ALL_PORT) {
                        dos_attack_data[unit][port].stat_id[DOS_ATTACK_CASE_SYN_FLOOD] = stat_id;
                        dos_attack_data[unit][port].pol_id[DOS_ATTACK_CASE_SYN_FLOOD] = pol_id;
                    } else {
                        for (x = 0; x < SOC_MAX_NUM_PORTS; x++) {
                            dos_attack_data[unit][x].stat_id[DOS_ATTACK_CASE_SYN_FLOOD] = stat_id;
                            dos_attack_data[unit][x].pol_id[DOS_ATTACK_CASE_SYN_FLOOD] = pol_id;
                        }
                    }
                    BCM_IF_ERROR_RETURN(
                        bcm_robo_field_action_add(unit, entry, bcmFieldActionRpDrop, 0, 0));
                    BCM_IF_ERROR_RETURN(
                        bcm_robo_field_entry_install(unit, entry));
                }
                /* SYN-ACK Flood */
                if (port != DOS_ATTACK_ALL_PORT) {
                    entry = dos_attack_data[unit][port].entry_id[DOS_ATTACK_CASE_SYNACK_FLOOD];
                } else {
                    entry = dos_attack_data[unit][0].entry_id[DOS_ATTACK_CASE_SYNACK_FLOOD];
                }

                if (!entry) {
                    rv = bcm_robo_field_entry_create(unit, group, &entry);
                    if (rv < 0) {
                        return rv;
                    }
                    rv = bcm_robo_field_entry_prio_set(unit, entry, 
                        BCM_FIELD_ENTRY_PRIO_HIGHEST);
                    if (rv < 0) {
                        return rv;
                    }
    
                    if (port != DOS_ATTACK_ALL_PORT) {
                        dos_attack_data[unit][port].entry_id[DOS_ATTACK_CASE_SYNACK_FLOOD] = entry;
    
                        if (SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit) || 
                            SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) || 
                            SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) || 
                            SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
                            BCM_PBMP_PORT_ADD(pbm, port);
                            BCM_IF_ERROR_RETURN(
                                bcm_robo_field_qualify_InPorts
                                (unit, entry, pbm, pbm_mask));
                        }
                    } else {
                        for (x = 0; x < SOC_MAX_NUM_PORTS; x++) {
                            dos_attack_data[unit][x].entry_id[DOS_ATTACK_CASE_SYNACK_FLOOD] = entry;
                        }
                    }
    
                    BCM_IF_ERROR_RETURN(
                        bcm_robo_field_qualify_TcpControl(unit, entry, \
                            TCP_FLAG_SYN|TCP_FLAG_ACK, TCP_FLAG_MASK));
                    if (port != DOS_ATTACK_ALL_PORT) {
                        x = port;
                    } else {
                        x = 0; /* port 0 */
                    }

                    pol_id = dos_attack_data[unit][x].pol_id[DOS_ATTACK_CASE_SYNACK_FLOOD];
                    if (pol_id == 0) {
                        sal_memset(&pol_cfg, 0, sizeof(bcm_policer_config_t));
                        pol_cfg.mode = bcmPolicerModeCommitted;
                        pol_cfg.ckbits_burst = 1024;
                        pol_cfg.ckbits_sec = 64;
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_policer_create(unit, &pol_cfg, &pol_id));
                        dos_attack_data[unit][x].pol_id[DOS_ATTACK_CASE_SYNACK_FLOOD] = pol_id;
                    }
                    BCM_IF_ERROR_RETURN(
                        bcm_robo_field_entry_policer_attach(unit, 
                            entry, 0, pol_id));

                    stat_id = dos_attack_data[unit][x].stat_id[DOS_ATTACK_CASE_SYNACK_FLOOD];
                    if (stat_id == 0) {
                        dos_attack_data[unit][x].stat_arr[DOS_ATTACK_CASE_SYNACK_FLOOD] = 
                            bcmFieldStatPackets;
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_field_stat_create(unit, group, 1, 
                                &dos_attack_data[unit][x].stat_arr[DOS_ATTACK_CASE_SYNACK_FLOOD], 
                                &stat_id));
                    }
                    BCM_IF_ERROR_RETURN(
                        bcm_robo_field_entry_stat_attach(unit, 
                            entry, stat_id));
                    if (port != DOS_ATTACK_ALL_PORT) {
                        dos_attack_data[unit][port].stat_id[DOS_ATTACK_CASE_SYNACK_FLOOD] = stat_id;
                        dos_attack_data[unit][port].pol_id[DOS_ATTACK_CASE_SYNACK_FLOOD] = pol_id;
                    } else {
                        for (x = 0; x < SOC_MAX_NUM_PORTS; x++) {
                            dos_attack_data[unit][x].stat_id[DOS_ATTACK_CASE_SYNACK_FLOOD] = stat_id;
                            dos_attack_data[unit][x].pol_id[DOS_ATTACK_CASE_SYNACK_FLOOD] = pol_id;
                        }
                    }
                    BCM_IF_ERROR_RETURN(
                        bcm_robo_field_action_add(unit, entry, bcmFieldActionRpDrop, 0, 0));
                    BCM_IF_ERROR_RETURN(
                        bcm_robo_field_entry_install(unit, entry));
                }

                if (port != DOS_ATTACK_ALL_PORT) {
                    dos_attack_data[unit][port].attacks |= DOS_ATTACK_SYN_FLOOD;
                    dos_attack_data[unit][port].attacks &= ~DOS_ATTACK_TO_CPU;
                } else {
                    for (x = 0; x < SOC_MAX_NUM_PORTS; x++) {
                        dos_attack_data[unit][x].attacks |= DOS_ATTACK_SYN_FLOOD;
                        dos_attack_data[unit][x].attacks &= ~DOS_ATTACK_TO_CPU;
                    }
                }
            } else {
                if (port != DOS_ATTACK_ALL_PORT) {
                    entry = 
                        dos_attack_data[unit][port].entry_id[DOS_ATTACK_CASE_SYN_FLOOD];
                    pol_id = dos_attack_data[unit][port].pol_id[DOS_ATTACK_CASE_SYN_FLOOD];
                    stat_id = dos_attack_data[unit][port].stat_id[DOS_ATTACK_CASE_SYN_FLOOD];
                    if ((pol_id != 0) && (entry != 0)) {
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_field_entry_policer_detach(unit, entry, 0));
                    }
                    if ((stat_id != 0) && (entry != 0)) {
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_field_entry_stat_detach(unit, entry, stat_id));
                    }
                    if (pol_id != 0) {
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_policer_destroy(unit, pol_id));
                    }
                    if (stat_id != 0) {
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_field_stat_destroy(unit, stat_id));
                    }
                    if (entry) {
                        rv = bcm_robo_field_entry_destroy(unit, entry);
                    }
                    if ((rv == BCM_E_NONE) || (rv == BCM_E_NOT_FOUND)) {
                        dos_attack_data[unit][port].entry_id[DOS_ATTACK_CASE_SYN_FLOOD] = 0;
                        dos_attack_data[unit][port].pol_id[DOS_ATTACK_CASE_SYN_FLOOD] = 0;
                        dos_attack_data[unit][port].stat_id[DOS_ATTACK_CASE_SYN_FLOOD] = 0;
                    } else {
                        return rv;
                    }

                    entry = 
                        dos_attack_data[unit][port].entry_id[DOS_ATTACK_CASE_SYNACK_FLOOD];
                    pol_id = dos_attack_data[unit][port].pol_id[DOS_ATTACK_CASE_SYNACK_FLOOD];
                    stat_id = dos_attack_data[unit][port].stat_id[DOS_ATTACK_CASE_SYNACK_FLOOD];
                    if ((pol_id != 0) && (entry != 0)) {
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_field_entry_policer_detach(unit, entry, 0));
                    }
                    if ((stat_id != 0) && (entry != 0)) {
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_field_entry_stat_detach(unit, entry, stat_id));
                    }
                    if (pol_id != 0) {
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_policer_destroy(unit, pol_id));
                    }
                    if (stat_id != 0) {
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_field_stat_destroy(unit, stat_id));
                    }
                    if (entry) {
                        rv = bcm_robo_field_entry_destroy(unit, entry);
                    }
                    if ((rv == BCM_E_NONE) || (rv == BCM_E_NOT_FOUND)) {
                        dos_attack_data[unit][port].entry_id[DOS_ATTACK_CASE_SYNACK_FLOOD] = 0;
                        dos_attack_data[unit][port].pol_id[DOS_ATTACK_CASE_SYNACK_FLOOD] = 0;
                        dos_attack_data[unit][port].stat_id[DOS_ATTACK_CASE_SYNACK_FLOOD] = 0;
                    } else {
                        return rv;
                    }
                    dos_attack_data[unit][port].attacks &= ~DOS_ATTACK_SYN_FLOOD;
                } else {
                    /*
                     * Since DOS_ATTACK_ALL_PORT set all port's data base same
                     * value, retrieve port 0's entry id to remove entry.
                     */
                    entry = 
                        dos_attack_data[unit][0].entry_id[DOS_ATTACK_CASE_SYN_FLOOD];
                    pol_id = dos_attack_data[unit][0].pol_id[DOS_ATTACK_CASE_SYN_FLOOD];
                    stat_id = dos_attack_data[unit][0].stat_id[DOS_ATTACK_CASE_SYN_FLOOD];
                    if ((pol_id != 0) && (entry != 0)) {
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_field_entry_policer_detach(unit, entry, 0));
                    }
                    if ((stat_id != 0) && (entry != 0)) {
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_field_entry_stat_detach(unit, entry, stat_id));
                    }
                    if (pol_id != 0) {
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_policer_destroy(unit, pol_id));
                    }
                    if (stat_id != 0) {
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_field_stat_destroy(unit, stat_id));
                    }
                    if (entry) {
                        rv = bcm_robo_field_entry_destroy(unit, entry);
                    }
                    if ((rv != BCM_E_NONE) && (rv != BCM_E_NOT_FOUND)) {
                        return rv;
                    }

                    entry = 
                        dos_attack_data[unit][0].entry_id[DOS_ATTACK_CASE_SYNACK_FLOOD];
                    pol_id = dos_attack_data[unit][0].pol_id[DOS_ATTACK_CASE_SYNACK_FLOOD];
                    stat_id = dos_attack_data[unit][0].stat_id[DOS_ATTACK_CASE_SYNACK_FLOOD];
                    if ((pol_id != 0) && (entry != 0)) {
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_field_entry_policer_detach(unit, entry, 0));
                    }
                    if ((stat_id != 0) && (entry != 0)) {
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_field_entry_stat_detach(unit, entry, stat_id));
                    }
                    if (pol_id != 0) {
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_policer_destroy(unit, pol_id));
                    }
                    if (stat_id != 0) {
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_field_stat_destroy(unit, stat_id));
                    }
                    if (entry) {
                        rv = bcm_robo_field_entry_destroy(unit, entry);
                    }
                    if ((rv != BCM_E_NONE) && (rv != BCM_E_NOT_FOUND)) {
                        return rv;
                    }

                    for (x = 0; x < SOC_MAX_NUM_PORTS; x++) {
                        dos_attack_data[unit][x].entry_id[DOS_ATTACK_CASE_SYN_FLOOD] = 0;
                        dos_attack_data[unit][x].entry_id[DOS_ATTACK_CASE_SYNACK_FLOOD] = 0;
                        dos_attack_data[unit][x].pol_id[DOS_ATTACK_CASE_SYN_FLOOD] = 0;
                        dos_attack_data[unit][x].pol_id[DOS_ATTACK_CASE_SYNACK_FLOOD] = 0;
                        dos_attack_data[unit][x].stat_id[DOS_ATTACK_CASE_SYN_FLOOD] = 0;
                        dos_attack_data[unit][x].stat_id[DOS_ATTACK_CASE_SYNACK_FLOOD] = 0;
                        dos_attack_data[unit][x].attacks &= ~DOS_ATTACK_SYN_FLOOD;
                    }
                }
            }
            break;
        case bcmSwitchDosAttackTcpSmurf:
            if (SOC_IS_VO(unit)) {
                /* DSTIP in VO is unavail */
                rv = BCM_E_UNAVAIL;
                return rv;
            }
            
            if (param) {
                if (port != DOS_ATTACK_ALL_PORT) {
                    entry = dos_attack_data[unit][port].entry_id[DOS_ATTACK_CASE_TCP_SMURF];
                } else {
                    entry = dos_attack_data[unit][0].entry_id[DOS_ATTACK_CASE_TCP_SMURF];
                }

                if (!entry) {
                    rv = bcm_robo_field_entry_create(unit, group, &entry);
                    if (rv < 0) {
                        return rv;
                    }
                    rv = bcm_robo_field_entry_prio_set(unit, entry, 
                        BCM_FIELD_ENTRY_PRIO_HIGHEST);
                    if (rv < 0) {
                        return rv;
                    }
    
                    if (port != DOS_ATTACK_ALL_PORT) {
                        dos_attack_data[unit][port].entry_id[DOS_ATTACK_CASE_TCP_SMURF] = entry;
    
                        if (SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit) || 
                            SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) || 
                            SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) || 
                            SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
                            BCM_PBMP_PORT_ADD(pbm, port);
                            BCM_IF_ERROR_RETURN(
                                bcm_robo_field_qualify_InPorts
                                (unit, entry, pbm, pbm_mask));
                        } else if (SOC_IS_TB(unit)){
                            BCM_IF_ERROR_RETURN(bcm_robo_field_qualify_InPort
                                    (unit, entry, port, 
                                    BCM_FIELD_EXACT_MATCH_MASK));
                        }
                    } else {
                        for (x = 0; x < SOC_MAX_NUM_PORTS; x++) {
                            dos_attack_data[unit][x].entry_id[DOS_ATTACK_CASE_TCP_SMURF] = entry;
                        }
                    }
    
                    BCM_IF_ERROR_RETURN(
                        bcm_robo_field_qualify_IpProtocol(unit, entry, IP_PROTOCOL_ID_ICMP, 0xff));
                    BCM_IF_ERROR_RETURN(
                        bcm_robo_field_qualify_DstIp(unit, entry, 0xff, 0xff));
                    BCM_IF_ERROR_RETURN(
                        bcm_robo_field_action_add(unit, entry, bcmFieldActionDrop, 0, 0));
                    if (port != DOS_ATTACK_ALL_PORT) {
                        x = port;
                    } else {
                        x = 0; /* port 0 */
                    }
                    stat_id = dos_attack_data[unit][x].stat_id[DOS_ATTACK_CASE_TCP_SMURF];
                    if (stat_id == 0) {
                        dos_attack_data[unit][x].stat_arr[DOS_ATTACK_CASE_TCP_SMURF] = bcmFieldStatPackets;
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_field_stat_create(unit, group, 1, 
                                &dos_attack_data[unit][x].stat_arr[DOS_ATTACK_CASE_TCP_SMURF], 
                                &stat_id));
                    }
                    BCM_IF_ERROR_RETURN(
                        bcm_robo_field_entry_stat_attach(unit, entry, stat_id));
                    if (port != DOS_ATTACK_ALL_PORT) {
                        dos_attack_data[unit][port].stat_id[DOS_ATTACK_CASE_TCP_SMURF] = stat_id;
                    } else {
                        for (x = 0; x < SOC_MAX_NUM_PORTS; x++) {
                            dos_attack_data[unit][x].stat_id[DOS_ATTACK_CASE_TCP_SMURF] = stat_id;
                        }
                    }
                    BCM_IF_ERROR_RETURN(
                        bcm_robo_field_entry_install(unit, entry));
                }
                if (port != DOS_ATTACK_ALL_PORT) {
                    dos_attack_data[unit][port].attacks |= DOS_ATTACK_TCP_SMURF;
                    dos_attack_data[unit][port].attacks &= ~DOS_ATTACK_TO_CPU;
                } else {
                    for (x = 0; x < SOC_MAX_NUM_PORTS; x++) {
                        dos_attack_data[unit][x].attacks |= DOS_ATTACK_TCP_SMURF;
                        dos_attack_data[unit][x].attacks &= ~DOS_ATTACK_TO_CPU;
                    }
                }
            } else {
                if (port != DOS_ATTACK_ALL_PORT) {
                    entry = 
                        dos_attack_data[unit][port].entry_id[DOS_ATTACK_CASE_TCP_SMURF];
                    stat_id = dos_attack_data[unit][port].stat_id[DOS_ATTACK_CASE_TCP_SMURF];
                    if ((stat_id != 0) && (entry != 0)) {
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_field_entry_stat_detach(unit, entry, stat_id));
                    }
                    if (stat_id != 0) {
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_field_stat_destroy(unit, stat_id));
                    }
                    dos_attack_data[unit][port].stat_id[DOS_ATTACK_CASE_TCP_SMURF] = 0;
                    if (entry) {
                        rv = bcm_robo_field_entry_destroy(unit, entry);
                    }
                    if ((rv == BCM_E_NONE) || (rv == BCM_E_NOT_FOUND)) {
                        dos_attack_data[unit][port].entry_id[DOS_ATTACK_CASE_TCP_SMURF] = 0;
                        dos_attack_data[unit][port].attacks &= ~DOS_ATTACK_TCP_SMURF;
                    } else {
                        return rv;
                    }
                } else {
                    /*
                     * Since DOS_ATTACK_ALL_PORT set all port's data base same
                     * value, retrieve port 0's entry id to remove entry.
                     */
                    entry = 
                        dos_attack_data[unit][0].entry_id[DOS_ATTACK_CASE_TCP_SMURF];
                    stat_id = dos_attack_data[unit][0].stat_id[DOS_ATTACK_CASE_TCP_SMURF];
                    if ((stat_id != 0) && (entry != 0)) {
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_field_entry_stat_detach(unit, entry, stat_id));
                    }
                    if (stat_id != 0) {
                        BCM_IF_ERROR_RETURN(
                            bcm_robo_field_stat_destroy(unit, stat_id));
                    }
                    if (entry) {
                        rv = bcm_robo_field_entry_destroy(unit, entry);
                    }
                    if ((rv == BCM_E_NONE) || (rv == BCM_E_NOT_FOUND)) {
                        for (x = 0; x < SOC_MAX_NUM_PORTS; x++) {
                            dos_attack_data[unit][x].entry_id[DOS_ATTACK_CASE_TCP_SMURF] = 0;
                            dos_attack_data[unit][x].stat_id[DOS_ATTACK_CASE_TCP_SMURF] = 0;
                            dos_attack_data[unit][x].attacks &= ~DOS_ATTACK_TCP_SMURF;
                        }
                    } else {
                        return rv;
                    }
                }
            }
            break;
        default:
            return BCM_E_UNAVAIL;
    } 

    return BCM_E_NONE;
}

int
_bcm_robo_dos_attack_get(int unit, bcm_port_t port, int dos_attack_type, int *param)
{
    bcm_port_t query_port;

    if (port == DOS_ATTACK_ALL_PORT) {
        /* if query all ports' status, return status of port #0 */
        query_port = 0;
    } else {
        query_port = port;
    }

    switch(dos_attack_type) {
        case bcmSwitchDosAttackToCpu:

            /* SDK-33697 : 
             *  - this DOS supports for those chips which provide CFP basis
             *      DOS only. 
             */
            if (!(SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit))){
                return BCM_E_UNAVAIL;
            }
            
            /* 
             * A primary check point for DoS Attack?
             */
            if (dos_attack_data[unit][query_port].attacks & DOS_ATTACK_TO_CPU) {
                *param = 1;
            } else {
                *param = 0;
            }
            break;
        case bcmSwitchDosAttackSipEqualDip:
            if (SOC_IS_VULCAN(unit) || SOC_IS_TBX(unit) || 
                SOC_IS_STARFIGHTER(unit) || SOC_IS_POLAR(unit) ||
                SOC_IS_NORTHSTAR(unit) || SOC_IS_NORTHSTARPLUS(unit) ||
                SOC_IS_STARFIGHTER3(unit)) {
                return BCM_E_UNAVAIL;
            }
            if (dos_attack_data[unit][query_port].attacks & DOS_ATTACK_SIP_EQUAL_DIP) {
                *param = 1;
            } else {
                *param = 0;
            }
            break;
        case bcmSwitchDosAttackMinTcpHdrSize:
            /* This DoS type is used to work with TcpFrag DoS type, but the 
             * TcpFrag DoS type through CFP solution is now unavailable. 
             * Thus this DoS type become unavailable here.
             */
            return BCM_E_UNAVAIL;            
            break;
        case bcmSwitchDosAttackV4FirstFrag:
            if (!(SOC_IS_VULCAN(unit) || SOC_IS_TBX(unit) ||
                SOC_IS_STARFIGHTER(unit) || SOC_IS_POLAR(unit) ||
                SOC_IS_NORTHSTAR(unit)|| SOC_IS_NORTHSTARPLUS(unit) ||
                SOC_IS_STARFIGHTER3(unit))) {
                return BCM_E_UNAVAIL;
            }
            if (dos_attack_data[unit][query_port].attacks & DOS_ATTACK_V4_FIRST_FRAG) {
                *param = 1;
            } else {
                *param = 0;
            }
            break;
        case bcmSwitchDosAttackTcpFlags:
            if (SOC_IS_TBX(unit)){
                /* TCP Control in TB is unavail */
                return BCM_E_UNAVAIL;
            }
            if (dos_attack_data[unit][query_port].attacks & DOS_ATTACK_TCP_FLAGS) {
                *param = 1;
            } else {
                *param = 0;
            }
            break;
        case bcmSwitchDosAttackTcpFlagsSF:
            if (SOC_IS_TBX(unit)){
                /* TCP Control in TB is unavail */
                return BCM_E_UNAVAIL;
            }
            if (dos_attack_data[unit][query_port].attacks & DOS_ATTACK_TCP_FLAGS_SF) {
                *param = 1;
            } else {
                *param = 0;
            }
            break;
        case bcmSwitchDosAttackTcpFlagsFUP:
            if (SOC_IS_TBX(unit)){
                /* TCP Control in TB is unavail */
                return BCM_E_UNAVAIL;
            }
            if (dos_attack_data[unit][query_port].attacks & DOS_ATTACK_TCP_FLAGS_FUP) {
                *param = 1;
            } else {
                *param = 0;
            }
            break;
        case bcmSwitchDosAttackSynFrag:
            if (SOC_IS_TBX(unit)){
                /* TCP Control in TB is unavail */
                return BCM_E_UNAVAIL;
            }
            if (dos_attack_data[unit][query_port].attacks & DOS_ATTACK_SYN_FRAG) {
                *param = 1;
            } else {
                *param = 0;
            }
            break;
        case bcmSwitchDosAttackFlagZeroSeqZero:
            if (SOC_IS_TBX(unit)){
                /* TCP Control in TB is unavail */
                return BCM_E_UNAVAIL;
            }
            if (dos_attack_data[unit][query_port].attacks & DOS_ATTACK_FLAG_ZERO_SEQ_ZERO) {
                *param = 1;
            } else {
                *param = 0;
            }
            break;
        case bcmSwitchDosAttackL4Port:
            if (SOC_IS_VULCAN(unit) || SOC_IS_TBX(unit) || 
                SOC_IS_STARFIGHTER(unit) || SOC_IS_POLAR(unit) ||
                SOC_IS_NORTHSTAR(unit) || SOC_IS_NORTHSTARPLUS(unit) ||
                SOC_IS_STARFIGHTER3(unit)) {
                return BCM_E_UNAVAIL;
            }
            if (dos_attack_data[unit][query_port].attacks & DOS_ATTACK_L4_PORT) {
                *param = 1;
            } else {
                *param = 0;
            }
            break;
        case bcmSwitchDosAttackTcpFrag:
            if (SOC_IS_VULCAN(unit) || SOC_IS_TBX(unit) || 
                SOC_IS_STARFIGHTER(unit) || SOC_IS_POLAR(unit) ||
                SOC_IS_NORTHSTAR(unit) || SOC_IS_NORTHSTARPLUS(unit) ||
                SOC_IS_STARFIGHTER3(unit)) {
                return BCM_E_UNAVAIL;
            }
            /* TcpFrag DoS on the chips which support HW DoS featur:
             * - The Fragment_Offset=1 in any fragment of a fragmented IP 
             *      datagram carrying part of TCP data.
             *
             * This function is for the DOS feature implementation through 
             *  CFP on ROBO chips. And the qualify set on TCP Fragment offset 
             *  field in current ROBO CFP is not supported.
             */
            return BCM_E_UNAVAIL;
            break;
        case bcmSwitchDosAttackPingFlood:
            if (dos_attack_data[unit][query_port].attacks & DOS_ATTACK_PING_FLOOD) {
                *param = 1;
            } else {
                *param = 0;
            }
            break;
        case bcmSwitchDosAttackSynFlood:
            if (SOC_IS_TBX(unit)){
                /* TCP Control in TB is unavail */
                return BCM_E_UNAVAIL;
            }
            if (dos_attack_data[unit][query_port].attacks & DOS_ATTACK_SYN_FLOOD) {
                *param = 1;
            } else {
                *param = 0;
            }
            break;
        case bcmSwitchDosAttackTcpSmurf:
            if (SOC_IS_VO(unit)){
                /* DSTIP  in VO is unavail */
                return BCM_E_UNAVAIL;
            }

            if (dos_attack_data[unit][query_port].attacks & DOS_ATTACK_TCP_SMURF) {
                *param = 1;
            } else {
                *param = 0;
            }
            break;
        default:
            return SOC_E_UNAVAIL;
    }
    
    return SOC_E_NONE;
}

int
_bcm_robo_hw_dos_attack_set(int unit, bcm_switch_control_t type, int arg)
{
    int     rv = BCM_E_NONE;
    uint32  dos_type = 0, temp = 0;
    
    switch(type) {
        case bcmSwitchDosAttackMinTcpHdrSize:
            dos_type = DRV_DOS_MIN_TCP_HDR_SZ;
            min_tcp_hdr_size = arg;
            temp = min_tcp_hdr_size; /* The unit expected in arg is byte */
            rv = DRV_DOS_ENABLE_SET(unit, dos_type, temp);
            break;
        case bcmSwitchDosAttackSipEqualDip:
            dos_type = DRV_DOS_LAND;
            temp = arg;
            rv = DRV_DOS_ENABLE_SET(unit, dos_type, temp);
            break;
        case bcmSwitchDosAttackTcpFlags:
            /* 4 cases */
            dos_type = DRV_DOS_SYN_WITH_SP_LT1024;
            temp = arg;
            rv = DRV_DOS_ENABLE_SET(unit, dos_type, temp);
            dos_type = DRV_DOS_SYN_FIN_SCAN;
            rv |= DRV_DOS_ENABLE_SET(unit, dos_type, temp);
            dos_type = DRV_DOS_XMASS_WITH_TCP_SEQ_ZERO;
            rv |= DRV_DOS_ENABLE_SET(unit, dos_type, temp);
            dos_type = DRV_DOS_NULL_WITH_TCP_SEQ_ZERO;
            rv |= DRV_DOS_ENABLE_SET(unit, dos_type, temp);
            break;
        case bcmSwitchDosAttackSynFrag:
            dos_type = DRV_DOS_SYN_WITH_SP_LT1024;
            temp = arg;
            rv = DRV_DOS_ENABLE_SET(unit, dos_type, temp);
            break;
        case bcmSwitchDosAttackTcpFlagsSF:
            dos_type = DRV_DOS_SYN_FIN_SCAN;
            temp = arg;
            rv = DRV_DOS_ENABLE_SET(unit, dos_type, temp);
            break;
        case bcmSwitchDosAttackTcpFlagsFUP:
            dos_type = DRV_DOS_XMASS_WITH_TCP_SEQ_ZERO;
            temp = arg;
            rv = DRV_DOS_ENABLE_SET(unit, dos_type, temp);
            break;
        case bcmSwitchDosAttackFlagZeroSeqZero:
            dos_type = DRV_DOS_NULL_WITH_TCP_SEQ_ZERO;
            temp = arg;
            rv = DRV_DOS_ENABLE_SET(unit, dos_type, temp);
            break;
        case bcmSwitchDosAttackL4Port:
            dos_type = DRV_DOS_BLAT;
            temp = arg;
            rv = DRV_DOS_ENABLE_SET(unit, dos_type, temp);
            break;
        case bcmSwitchDosAttackTcpPortsEqual:
            dos_type = DRV_DOS_TCP_BLAT;
            temp = arg;
            rv = DRV_DOS_ENABLE_SET(unit, dos_type, temp);
            break;
        case bcmSwitchDosAttackUdpPortsEqual:
            dos_type = DRV_DOS_UDP_BLAT;
            temp = arg;
            rv = DRV_DOS_ENABLE_SET(unit, dos_type, temp);
            break;
        case bcmSwitchDosAttackTcpFrag:
            /* Apply to set global value min_tcp_hdr_size to register first */
            dos_type = DRV_DOS_MIN_TCP_HDR_SZ;
            temp = min_tcp_hdr_size; /* The unit is byte */
            rv = DRV_DOS_ENABLE_SET(unit, dos_type, temp);
            dos_type = DRV_DOS_TCP_FRAG;
            temp = arg;
            rv |= DRV_DOS_ENABLE_SET(unit, dos_type, temp);
            break;
        case bcmSwitchDosAttackTcpOffset:
            dos_type = DRV_DOS_TCP_FRAG_OFFSET;
            temp = arg;
            rv = DRV_DOS_ENABLE_SET(unit, dos_type, temp);
            break;
        case bcmSwitchDosAttackTcpSmurf:
            dos_type = DRV_DOS_SMURF;
            temp = arg;
            rv = DRV_DOS_ENABLE_SET(unit, dos_type, temp);
            break;
        case bcmSwitchDosAttackIcmpFragments:
            /* 2 cases */
            dos_type = DRV_DOS_ICMPV4_FRAGMENTS;
            temp = arg;
            rv = DRV_DOS_ENABLE_SET(unit, dos_type, temp);
            dos_type = DRV_DOS_ICMPV6_FRAGMENTS;
            rv |= DRV_DOS_ENABLE_SET(unit, dos_type, temp);
            break;
        case bcmSwitchDosAttackIcmpPktOversize:
            dos_type = DRV_DOS_MAX_ICMPV4_SIZE;
            temp = arg;
            rv = DRV_DOS_ENABLE_SET(unit, dos_type, temp);
            break;
        case bcmSwitchDosAttackIcmpV6PingSize:
            dos_type = DRV_DOS_MAX_ICMPV6_SIZE;
            temp = arg;
            rv = DRV_DOS_ENABLE_SET(unit, dos_type, temp);
            break;
        case bcmSwitchDosAttackIcmp:
            /* 2 cases */
            /* Apply to set global value max_icmpv4_size to register first */
            dos_type = DRV_DOS_ICMPV4_LONG_PING;
            temp = arg;
            rv = DRV_DOS_ENABLE_SET(unit, dos_type, temp);
            /* Apply to set global value max_icmpv6_size to register first */
            dos_type = DRV_DOS_ICMPV6_LONG_PING;
            temp = arg;
            rv = DRV_DOS_ENABLE_SET(unit, dos_type, temp);
            break;
        case bcmSwitchDosAttackTcpHdrPartial:
            dos_type = DRV_DOS_TCP_SHORT_HDR;
            temp = arg;
            rv = DRV_DOS_ENABLE_SET(unit, dos_type, temp);
            break;
        default:
            rv = BCM_E_UNAVAIL;
    }

    if (soc_feature(unit, soc_feature_hw_dos_report)) {
        uint32  events_bmp = 0, us = 0;
        
        if (rv == SOC_E_NONE) {
            BCM_IF_ERROR_RETURN(DRV_DOS_EVENT_BITMAP_GET(unit, 
                    DRV_DOS_EVT_OP_ENABLED, &events_bmp));
            BCM_IF_ERROR_RETURN(soc_robo_dos_monitor_enable_get(unit, &us));
            if (events_bmp){
                /* start the HW DOS monitor if any HW DOS is enabled */
                if (us == 0){
                    BCM_IF_ERROR_RETURN(soc_robo_dos_monitor_enable_set(
                            unit, BCM_SWITCH_HW_DOS_MONITOR_INTERVAL));
                }
                
            } else {
                /* stop the HW DOS monitor if any HW DOS is enabled */
                if (us > 0){
                    BCM_IF_ERROR_RETURN(
                            soc_robo_dos_monitor_enable_set(
                            unit, 0));
                }
            }
        }
    }
    
    return rv;
}

int
_bcm_robo_hw_dos_attack_get(int unit, bcm_switch_control_t type, int *arg)
{
    int rv = BCM_E_NONE;
    uint32 dos_type = 0, temp = 0;
    
    switch(type) {
        case bcmSwitchDosAttackMinTcpHdrSize:
            dos_type = DRV_DOS_MIN_TCP_HDR_SZ;
            rv = DRV_DOS_ENABLE_GET(unit, dos_type, &temp);
            *arg = temp;
            break;
        case bcmSwitchDosAttackSipEqualDip:
            dos_type = DRV_DOS_LAND;
            rv = DRV_DOS_ENABLE_GET(unit, dos_type, &temp);
            *arg = temp;
            break;
        case bcmSwitchDosAttackTcpFlags:
            /* 4 cases */
            /* Just need to get one of the cases */
            dos_type = DRV_DOS_SYN_WITH_SP_LT1024;
            rv = DRV_DOS_ENABLE_GET(unit, dos_type, &temp);
            *arg = temp;
            break;
        case bcmSwitchDosAttackSynFrag:
            dos_type = DRV_DOS_SYN_WITH_SP_LT1024;
            rv = DRV_DOS_ENABLE_GET(unit, dos_type, &temp);
            *arg = temp;
            break;
        case bcmSwitchDosAttackTcpFlagsSF:
            dos_type = DRV_DOS_SYN_FIN_SCAN;
            rv = DRV_DOS_ENABLE_GET(unit, dos_type, &temp);
            *arg = temp;
            break;
        case bcmSwitchDosAttackTcpFlagsFUP:
            dos_type = DRV_DOS_XMASS_WITH_TCP_SEQ_ZERO;
            rv = DRV_DOS_ENABLE_GET(unit, dos_type, &temp);
            *arg = temp;
            break;
        case bcmSwitchDosAttackFlagZeroSeqZero:
            dos_type = DRV_DOS_NULL_WITH_TCP_SEQ_ZERO;
            rv = DRV_DOS_ENABLE_GET(unit, dos_type, &temp);
            *arg = temp;
            break;
        case bcmSwitchDosAttackL4Port:
            dos_type = DRV_DOS_BLAT;
            rv = DRV_DOS_ENABLE_GET(unit, dos_type, &temp);
             *arg = temp;
            break;
        case bcmSwitchDosAttackTcpPortsEqual:
            dos_type = DRV_DOS_TCP_BLAT;
            rv = DRV_DOS_ENABLE_GET(unit, dos_type, &temp);
             *arg = temp;
            break;
        case bcmSwitchDosAttackUdpPortsEqual:
            dos_type = DRV_DOS_UDP_BLAT;
            rv = DRV_DOS_ENABLE_GET(unit, dos_type, &temp);
             *arg = temp;
            break;
        case bcmSwitchDosAttackTcpFrag:
            dos_type = DRV_DOS_TCP_FRAG;
            rv = DRV_DOS_ENABLE_GET(unit, dos_type, &temp);
             *arg = temp;
            break;
        case bcmSwitchDosAttackTcpOffset:
            dos_type = DRV_DOS_TCP_FRAG_OFFSET;
            rv = DRV_DOS_ENABLE_GET(unit, dos_type, &temp);
             *arg = temp;
            break;
        case bcmSwitchDosAttackTcpSmurf:
            dos_type = DRV_DOS_SMURF;
            rv = DRV_DOS_ENABLE_GET(unit, dos_type, &temp);
             *arg = temp;
            break;
        case bcmSwitchDosAttackIcmpFragments:
            /* 2 cases */
            /* Just need to get one of the cases */
            dos_type = DRV_DOS_ICMPV4_FRAGMENTS;
            rv = DRV_DOS_ENABLE_GET(unit, dos_type, &temp);
            *arg = temp;
            break;
        case bcmSwitchDosAttackIcmpPktOversize:
            dos_type = DRV_DOS_MAX_ICMPV4_SIZE;
            rv = DRV_DOS_ENABLE_GET(unit, dos_type, &temp);
            *arg = temp;
            break;
        case bcmSwitchDosAttackIcmpV6PingSize:
            dos_type = DRV_DOS_MAX_ICMPV6_SIZE;
            rv = DRV_DOS_ENABLE_GET(unit, dos_type, &temp);
            *arg = temp;
            break;
        case bcmSwitchDosAttackIcmp:
            /* 2 cases */
            /* Just need to get one of the cases */
            dos_type = DRV_DOS_ICMPV4_LONG_PING;
            rv = DRV_DOS_ENABLE_GET(unit, dos_type, &temp);
            *arg = temp;
            break;
        case bcmSwitchDosAttackTcpHdrPartial:
            dos_type = DRV_DOS_TCP_SHORT_HDR;
            rv = DRV_DOS_ENABLE_GET(unit, dos_type, &temp);
            *arg = temp;
            break;
        default:
            rv = BCM_E_UNAVAIL;
    }

    return rv;
}

/*
 * Function:
 *      _bcm_robo_switch_eav_mac_ethertype_set
 * Description:
 *      Set the upper 24 bits of MAC address or EtherType field 
 *      for timeSync protocol
 * Parameters:
 *      unit        - unit number.
 *      port        - port number.
 *      type        - The required switch control type to get MAC for
 *      arg         - arg to set the upper MAC address
 * Returns:
 *      BCM_E_xxxx
 */
 
STATIC int
_bcm_robo_switch_eav_mac_ethertype_set(int unit, bcm_switch_control_t type, int arg)
{
    bcm_mac_t mac;
    bcm_port_ethertype_t etype;
    int rv = BCM_E_NONE;

    if (!soc_feature(unit, soc_feature_eav_support)) {
        return BCM_E_UNAVAIL;
    }

    sal_memset(mac, 0, sizeof(bcm_mac_t));
    etype = 0;

    rv = DRV_EAV_TIME_SYNC_MAC_GET(unit, mac, &etype);
    if ((rv < 0) && (rv != BCM_E_DISABLED)) {
        return rv;
    }

    switch(type) {
        case bcmSwitchTimeSyncEthertype:
            etype = arg;
            break;
        case bcmSwitchTimeSyncDestMacOui:
            mac[0] = (arg & 0xff0000) >> 16; 
            mac[1] = (arg & 0xff00) >> 8; 
            mac[2] = (arg & 0xff); 
            break;
        case bcmSwitchTimeSyncDestMacNonOui:
            mac[3] = (arg & 0xff0000) >> 16; 
            mac[4] = (arg & 0xff00) >> 8; 
            mac[5] = (arg & 0xff); 
            break;
        default:
            return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(
        DRV_EAV_TIME_SYNC_MAC_SET(unit, mac, etype));

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_robo_switch_eav_mac_ethertype_get
 * Description:
 *      Get the upper 24 bits of MAC address or EtherType field 
 *      for timeSync protocol
 * Parameters:
 *      unit        - unit number.
 *      port        - port number.
 *      type        - The required switch control type to get MAC for
 *      arg         - arg to get the upper MAC address
 * Returns:
 *      BCM_E_xxxx
 */

STATIC int
_bcm_robo_switch_eav_mac_ethertype_get(int unit, bcm_switch_control_t type, int *arg)
{
    bcm_mac_t mac;
    bcm_port_ethertype_t etype;
    int rv = BCM_E_NONE;

    if (!soc_feature(unit, soc_feature_eav_support)) {
        return BCM_E_UNAVAIL;
    }

    sal_memset(mac, 0, sizeof(bcm_mac_t));
    etype = 0;

    rv = DRV_EAV_TIME_SYNC_MAC_GET(unit, mac, &etype);
    if ((rv < 0) && (rv != BCM_E_DISABLED)) {
        return rv;
    }

    switch(type) {
        case bcmSwitchTimeSyncEthertype:
            *arg = etype;
            break;
        case bcmSwitchTimeSyncDestMacOui:
            *arg = (mac[0] << 16) | (mac[1] << 8) | mac[2]; 
            break;
        case bcmSwitchTimeSyncDestMacNonOui:
            *arg = (mac[3] << 16) | (mac[4] << 8) | mac[5]; 
            break;
        default:
            return BCM_E_PARAM;
    }

    return BCM_E_NONE;
}


/*
 * Function:
 *      _bcm_robo_switch_eav_action_set
 * Description:
 *      Set the EAV action for timeSync protocol
 * Parameters:
 *      unit        - unit number.
 *      port        - port number.
 *      type        - The required switch control type to get MAC for
 *      arg         - arg to set action
 * Returns:
 *      BCM_E_xxxx
 */


STATIC int
_bcm_robo_switch_eav_action_set(int unit, bcm_switch_control_t type, int arg)
{
    uint32 reg_value = 0, temp = 0;

    if (!soc_feature(unit, soc_feature_eav_support)) {
        return BCM_E_UNAVAIL;
    }

    BCM_IF_ERROR_RETURN(
        REG_READ_MPORTVEC0r(unit, &reg_value));

    /* If the type is Drop or Flood and the arg is zero, change the action to Copy to CPU */
    switch (type) {
        case bcmSwitchTimeSyncPktToCpu:
            temp  = SOC_PBMP_WORD_GET(PBMP_CMIC(unit), 0);
            break;
        case bcmSwitchTimeSyncPktDrop:
            temp = (arg) ? 0 : SOC_PBMP_WORD_GET(PBMP_CMIC(unit), 0);
            break;
        case bcmSwitchTimeSyncPktFlood:
            temp  = (arg) ? SOC_PBMP_WORD_GET(PBMP_ALL(unit), 0) : 
                SOC_PBMP_WORD_GET(PBMP_CMIC(unit), 0);
            break;
        default:
            return BCM_E_PARAM;
    }

    soc_MPORTVEC0r_field_set(unit, &reg_value, 
        PORT_VCTRf, &temp);

    BCM_IF_ERROR_RETURN(
        REG_WRITE_MPORTVEC0r(unit, &reg_value));

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_robo_switch_eav_action_get
 * Description:
 *      Get the EAV action for timeSync protocol
 * Parameters:
 *      unit        - unit number.
 *      port        - port number.
 *      type        - The required switch control type to get MAC for
 *      arg         - arg to get action
 * Returns:
 *      BCM_E_xxxx
 */

STATIC int
_bcm_robo_switch_eav_action_get(int unit, bcm_switch_control_t type, int *arg)
{
    uint32 reg_value = 0, temp = 0;
    uint32 vec_value = 0;

    if (!soc_feature(unit, soc_feature_eav_support)) {
        return BCM_E_UNAVAIL;
    }

    BCM_IF_ERROR_RETURN(
        REG_READ_MPORTVEC0r(unit, &reg_value));

    soc_MPORTVEC0r_field_get(unit, &reg_value, 
        PORT_VCTRf, &temp);

    switch (type) {
        case bcmSwitchTimeSyncPktToCpu:
            vec_value = SOC_PBMP_WORD_GET(PBMP_CMIC(unit),0);
            break;
        case bcmSwitchTimeSyncPktDrop:
            vec_value = 0;
            break;
        case bcmSwitchTimeSyncPktFlood:
            vec_value = SOC_PBMP_WORD_GET(PBMP_ALL(unit), 0);
            break;
        default:
            return BCM_E_UNAVAIL;
    }
    
    *arg = (vec_value == temp) ? 1 : 0;

    return BCM_E_NONE;
}

/*
 * Function:
 *  _bcm_robo_bypass_modes_get
 * Description:
 *  Set the Enable/Disable status of the system based bypass mode.
 *      
 */
STATIC int
_bcm_robo_bypass_modes_get(int unit, int type, uint32 *modes)
{ 
#ifdef  BCM_TB_SUPPORT
    int     enable = 0, ctrl_val[BCM_FILTER_CONTROL_COUNT];
    uint32  ctrl_cnt = 0;
    uint32  ctrl_type[BCM_FILTER_CONTROL_COUNT];
    uint32  acting_modes = 0, support_modes = 0;
    
    /* prepare the target types to get */
    sal_memset(ctrl_type, 0, sizeof(uint32) * BCM_FILTER_CONTROL_COUNT);
    sal_memset(ctrl_val, 0, sizeof(uint32) * BCM_FILTER_CONTROL_COUNT);

    /* SwitchFiltersControlList is used to report the filter bypass control
     *  supporting items with bitmap format.
     */
    ctrl_type[0] = DRV_DEV_CTRL_FL_BYPASS_SUPPORT_LIST;
    ctrl_cnt = 1;
    BCM_IF_ERROR_RETURN(DRV_DEV_CONTROL_GET(unit, 
            &ctrl_cnt, ctrl_type, ctrl_val));
    support_modes = ctrl_val[0];
    ctrl_val[0] = 0;
    
    if (type == bcmSwitchFiltersControlList){
        *modes = support_modes;
    } else if (type == bcmSwitchFiltersControlModes){
        
        /* check the supported modes */
        if (support_modes == BCM_FILTER_NONE){
            *modes = BCM_FILTER_NONE;
            return BCM_E_UNAVAIL;
        }
        
        /* assigning the bypass mode type for get */
        ctrl_type[0] = DRV_DEV_CTRL_RX_BYPASS_CRCCHK;
        ctrl_type[1] = DRV_DEV_CTRL_DOS_BYPASS_TOCPU;
        ctrl_type[2] = DRV_DEV_CTRL_STP_BYPASS_USERADDR;
        ctrl_type[3] = DRV_DEV_CTRL_STP_BYPASS_MAC0X;
        ctrl_type[4] = DRV_DEV_CTRL_EAP_BYPASS_USERADDR;
        ctrl_type[5] = DRV_DEV_CTRL_EAP_BYPASS_DHCP;
        ctrl_type[6] = DRV_DEV_CTRL_EAP_BYPASS_ARP;
        ctrl_type[7] = DRV_DEV_CTRL_EAP_BYPASS_MAC_22_2F;
        ctrl_type[8] = DRV_DEV_CTRL_EAP_BYPASS_MAC_21;
        ctrl_type[9] = DRV_DEV_CTRL_EAP_BYPASS_MAC_20;
        ctrl_type[10] = DRV_DEV_CTRL_EAP_BYPASS_MAC_11_1F;
        ctrl_type[11] = DRV_DEV_CTRL_EAP_BYPASS_MAC_10;
        ctrl_type[12] = DRV_DEV_CTRL_EAP_BYPASS_MAC_0X;
        ctrl_cnt = 13;
        BCM_IF_ERROR_RETURN(DRV_DEV_CONTROL_GET(unit, 
                    &ctrl_cnt, ctrl_type, ctrl_val));

        acting_modes |= (ctrl_val[0]) ? BCM_FILTER_RX_CRCCHK : 0;
        acting_modes |= (ctrl_val[1]) ? BCM_FILTER_DOS_TOCPU : 0;
        acting_modes |= (ctrl_val[2]) ? BCM_FILTER_STP_USERADDR : 0;
        acting_modes |= (ctrl_val[3]) ? BCM_FILTER_STP_MAC0X : 0;
        acting_modes |= (ctrl_val[4]) ? BCM_FILTER_EAP_USERADDR : 0;
        acting_modes |= (ctrl_val[5]) ? BCM_FILTER_EAP_DHCP : 0;
        acting_modes |= (ctrl_val[6]) ? BCM_FILTER_EAP_ARP : 0;
        acting_modes |= (ctrl_val[7]) ? BCM_FILTER_EAP_MAC_22_2F : 0;
        acting_modes |= (ctrl_val[8]) ? BCM_FILTER_EAP_MAC_21 : 0;
        acting_modes |= (ctrl_val[9]) ? BCM_FILTER_EAP_MAC_20 : 0;
        acting_modes |= (ctrl_val[10]) ? BCM_FILTER_EAP_MAC_11_1F : 0;
        acting_modes |= (ctrl_val[11]) ? BCM_FILTER_EAP_MAC_10 : 0;
        acting_modes |= (ctrl_val[12]) ? BCM_FILTER_EAP_MAC_0X : 0;

        /* vlan bypass sections */
        BCM_IF_ERROR_RETURN(bcm_robo_vlan_control_get(unit, 
                    bcmVlanBypassIgmpMld, &enable));
        acting_modes |= (enable) ? BCM_FILTER_VLAN_IGMP_MLD : 0;
        BCM_IF_ERROR_RETURN(bcm_robo_vlan_control_get(unit, 
                    bcmVlanBypassArpDhcp, &enable));
        acting_modes |= (enable) ? BCM_FILTER_VLAN_ARP_DHCP : 0;
        BCM_IF_ERROR_RETURN(bcm_robo_vlan_control_get(unit, 
                    bcmVlanBypassMiim, &enable));
        acting_modes |= (enable) ? BCM_FILTER_VLAN_MIIM : 0;
        BCM_IF_ERROR_RETURN(bcm_robo_vlan_control_get(unit, 
                    bcmVlanBypassMcast, &enable));
        acting_modes |= (enable) ? BCM_FILTER_VLAN_MCAST : 0;
        BCM_IF_ERROR_RETURN(bcm_robo_vlan_control_get(unit, 
                    bcmVlanBypassRsvdMcast, &enable));
        acting_modes |= (enable) ? BCM_FILTER_VLAN_RSV_MCAST : 0;
        BCM_IF_ERROR_RETURN(bcm_robo_vlan_control_get(unit, 
                    bcmVlanBypassL2UserAddr, &enable));
        acting_modes |= (enable) ? BCM_FILTER_VLAN_USERADDR : 0;
      
        *modes = acting_modes;
    
    } else {
        return BCM_E_PARAM;
    }

    return BCM_E_NONE;
#else   /* BCM_TB_SUPPORT */
        COMPILER_REFERENCE(unit);
        COMPILER_REFERENCE(modes);
        
        return BCM_E_UNAVAIL;
#endif  /* BCM_TB_SUPPORT */
}

/*
 * Function:
 *  _bcm_robo_bypass_modes_set
 * Description:
 *  Set the Enable/Disable status of the system based bypass mode.
 *      
 */
STATIC int
_bcm_robo_bypass_modes_set(int unit, uint32 modes)
{
#ifdef  BCM_TB_SUPPORT
    uint32  support_modes = BCM_FILTER_NONE;
    uint32  vlan_ctrl_type = 0, ctrl_cnt = 0;
    uint32  ctrl_type;
    int     ctrl_val;
    int     i, temp_type = 0;

    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "%s, set bypass control items(0x%x) into this chip!\n"),
                 FUNCTION_NAME(), modes));
    /* retrieve the chip specific supported bypass control items */
    BCM_IF_ERROR_RETURN( _bcm_robo_bypass_modes_get(unit, 
            bcmSwitchFiltersControlList, &support_modes));
    if (support_modes == BCM_FILTER_NONE){
        if (modes != BCM_FILTER_NONE){
            LOG_VERBOSE(BSL_LS_BCM_COMMON,
                        (BSL_META_U(unit,
                                    "%s, No bypass control item in this chip!\n"),
                         FUNCTION_NAME()));
            return BCM_E_UNAVAIL;
        }
    } else {
        if (modes & (~support_modes)){
            LOG_VERBOSE(BSL_LS_BCM_COMMON,
                        (BSL_META_U(unit,
                                    "%s, Invalid bypass control item(0x%x) in this chip!\n"),
                         FUNCTION_NAME(), (modes & (~support_modes))));
            return BCM_E_UNAVAIL;
        }
        
        /* get the support items count */
        ctrl_cnt = 1;
        assert(BCM_FILTER_CONTROL_COUNT <= 32);

        for (i = 0; i < BCM_FILTER_CONTROL_COUNT; i++){
            temp_type = (uint32)0x1 << i;
            if (support_modes & ((uint32)0x1 << i)){
                /* each supported item will be set to enable or disable */
                ctrl_val = (modes & temp_type) ? TRUE : FALSE;
                
                DRV_FILTER_BYPASS_TYPE_GET(temp_type, i);

                /* VLAN related bypass mode is designed in VLAN API */
                vlan_ctrl_type = -1;
                if (temp_type == BCM_FILTER_VLAN_IGMP_MLD){
                    vlan_ctrl_type = bcmVlanBypassIgmpMld;
                } else if (temp_type == BCM_FILTER_VLAN_ARP_DHCP){
                    vlan_ctrl_type = bcmVlanBypassArpDhcp;
                } else if (temp_type == BCM_FILTER_VLAN_MIIM){
                    vlan_ctrl_type = bcmVlanBypassMiim;
                } else if (temp_type == BCM_FILTER_VLAN_MCAST){
                    vlan_ctrl_type = bcmVlanBypassMcast;
                } else if (temp_type == BCM_FILTER_VLAN_RSV_MCAST){
                    vlan_ctrl_type = bcmVlanBypassRsvdMcast;
                } else if (temp_type == BCM_FILTER_VLAN_USERADDR){
                    vlan_ctrl_type = bcmVlanBypassL2UserAddr;
                }

                if (vlan_ctrl_type != -1){
                    /* use VLAN APIs to set VLAN related bypass control */
                    BCM_IF_ERROR_RETURN(
                            bcm_robo_vlan_control_set(unit, 
                            vlan_ctrl_type, ctrl_val));
                } else {
                    /* set for non-vlan related bypass mode configuration */
                    ctrl_type = DRV_TYPE_FROM_FILTER_BYPASS_TO_PROP(temp_type);
                    BCM_IF_ERROR_RETURN(
                            DRV_DEV_CONTROL_SET(unit, 
                            &ctrl_cnt, &ctrl_type, &ctrl_val));
                }
                
            } else {
                if (modes & temp_type){
                    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                                (BSL_META_U(unit,
                                            "%s, Bypass control item in this chip "
                                            "at bit%d is not supported\n"),
                                 FUNCTION_NAME(), i));
                    return BCM_E_UNAVAIL;
                }
            }
        }

    }
    
    return BCM_E_NONE;
#else   /* BCM_TB_SUPPORT */
    COMPILER_REFERENCE(unit);
    COMPILER_REFERENCE(modes);
    
    return BCM_E_UNAVAIL;
#endif  /* BCM_TB_SUPPORT */
}

/*
 * Function:
 *  _bcm_robo_dev_control_set
 * Description:
 *  Set the Enable/Disable on device misc control(not easy to be classed into
 *  an existed API group or the existed API group have no proper function for 
 *  this control type.
 *
 */
STATIC int
_bcm_robo_dev_control_set(int unit, uint32 type, int arg){

    int     ctrl_val = 0;
    uint32  ctrl_type = 0, ctrl_cnt = 0;

    ctrl_cnt = 1;
    ctrl_val = arg;
    switch(type){
        case bcmSwitchJumboFrameDrop :
            ctrl_type = DRV_DEV_CTRL_JUMBO_FRAME_DROP;
            break;
        case bcmSwitchDroppedJumboFrameDisLearn :
            ctrl_type = DRV_DEV_CTRL_DROPPED_JUMBO_FRM_NO_LEARN;
            break;
        case bcmSwitchRangeErrDrop :
            ctrl_type = DRV_DEV_CTRL_RANGE_ERROR_DROP;
            break;
        case bcmSwitchDroppedRangeErrDisLearn :
            ctrl_type = DRV_DEV_CTRL_DROPPED_RANGE_ERR_NO_LEARN;
            break;
        case bcmSwitchMcastSaDrop :
            ctrl_type = DRV_DEV_CTRL_MCAST_SA_DROP;
            break;
        case bcmSwitchIgmpMldToCpu : 
            ctrl_type = DRV_DEV_CTRL_IGMP_MLD_SNOOP_MODE;
            break;
        case bcmSwitchIngressRateLimitIncludeIFG :
            ctrl_type = DRV_DEV_CTRL_RATE_METER_PLUS_IPG;
            break;
        case bcmSwitchDestMacZeroDrop :
            ctrl_type = DRV_DEV_CTRL_DA_ALL0_DROP;
            break;
        case bcmSwitchArpDhcpToCpu :
            ctrl_type = DRV_DEV_CTRL_ARPDHCP_TOCPU;
            break;
        case bcmSwitchReservedMcastLearn:
            ctrl_type = DRV_DEV_CTRL_RESERVED_MCAST_SA_LEARN;
            break;
        case bcmSwitchUnknownUcastToCpu:
            ctrl_type = DRV_DEV_CTRL_CPU_RXULF;
            break;
        case bcmSwitchUnknownMcastToCpu:
            ctrl_type = DRV_DEV_CTRL_CPU_RXMLF;
            break;
        case bcmSwitchL2LearnLimitToCpu :
            ctrl_type = DRV_DEV_CTRL_ARLBINFULL_TOCPU;
            break;
        case bcmSwitchPPPoESessionPktDscpRemarkEnable :
            ctrl_type = DRV_DEV_CTRL_EGRESS_PPPOEDSCP_REMARK;
            break;
        default:
            return BCM_E_UNAVAIL;
            break;
    }

    BCM_IF_ERROR_RETURN(DRV_DEV_CONTROL_SET(unit, 
                &ctrl_cnt, &ctrl_type, &ctrl_val));
    
    return BCM_E_NONE;
}

/*
 * Function:
 *  _bcm_robo_dev_control_get
 * Description:
 *  Get the Enable/Disable on device misc control(not easy to be classed into
 *  an existed API group or the existed API group have no proper function for 
 *  this control type.
 *
 */
STATIC int
_bcm_robo_dev_control_get(int unit, uint32 type, int *arg){

    int     ctrl_val = 0;
    uint32  ctrl_type = 0, ctrl_cnt = 0;

    ctrl_cnt = 1;
    switch(type){
        case bcmSwitchJumboFrameDrop :
            ctrl_type = DRV_DEV_CTRL_JUMBO_FRAME_DROP;
            break;
        case bcmSwitchDroppedJumboFrameDisLearn :
            ctrl_type = DRV_DEV_CTRL_DROPPED_JUMBO_FRM_NO_LEARN;
            break;
        case bcmSwitchRangeErrDrop :
            ctrl_type = DRV_DEV_CTRL_RANGE_ERROR_DROP;
            break;
        case bcmSwitchDroppedRangeErrDisLearn :
            ctrl_type = DRV_DEV_CTRL_DROPPED_RANGE_ERR_NO_LEARN;
            break;
        case bcmSwitchMcastSaDrop :
            ctrl_type = DRV_DEV_CTRL_MCAST_SA_DROP;
            break;
        case bcmSwitchIgmpMldToCpu : 
            ctrl_type = DRV_DEV_CTRL_IGMP_MLD_SNOOP_MODE;
            break;
        case bcmSwitchIngressRateLimitIncludeIFG :
            ctrl_type = DRV_DEV_CTRL_RATE_METER_PLUS_IPG;
            break;
        case bcmSwitchDestMacZeroDrop :
            ctrl_type = DRV_DEV_CTRL_DA_ALL0_DROP;
            break;
        case bcmSwitchArpDhcpToCpu :
            ctrl_type = DRV_DEV_CTRL_ARPDHCP_TOCPU;
            break;
        case bcmSwitchReservedMcastLearn:
            ctrl_type = DRV_DEV_CTRL_RESERVED_MCAST_SA_LEARN;
            break;
        case bcmSwitchUnknownUcastToCpu:
            ctrl_type = DRV_DEV_CTRL_CPU_RXULF;
            break;
        case bcmSwitchUnknownMcastToCpu:
            ctrl_type = DRV_DEV_CTRL_CPU_RXMLF;
            break;
        case bcmSwitchL2LearnLimitToCpu :
            ctrl_type = DRV_DEV_CTRL_ARLBINFULL_TOCPU;
            break;
        case bcmSwitchPPPoESessionPktDscpRemarkEnable :
            ctrl_type = DRV_DEV_CTRL_EGRESS_PPPOEDSCP_REMARK;
            break;
        default:
            return BCM_E_UNAVAIL;
            break;
    }

    BCM_IF_ERROR_RETURN(DRV_DEV_CONTROL_GET(unit, 
                &ctrl_cnt, &ctrl_type, &ctrl_val));
    *arg = ctrl_val;
    
    return BCM_E_NONE;
}

/* GNATS 40759 : redesign IGMP/MLD trap/snoop 
 *
 *  - the desiging matrix for ToCPU and Drop is 
 *       ToCpu  Drop  => [Mode]
 *  ------------------------------------
 *  (1):   F     F    => [Disable]
 *  (2):   F     T    => [Discard] (No forwarding to front port and CPU)
 *  (3):   T     F    => [Snoop] (Forwarding to front port and COPY to CPU)
 *  (4):   T     T    => [Trap] (Redirect to CPU only )
 *  ------------------------------------
 */

/*
 * Function:
 *  _bcm_robo_igmp_mld_control_set
 * Description:
 *  Set the IGMP/MLD related configuration.
 * Parameters:
 *      unit -  RoboSwitch PCI device unit number (driver internal).
 *      type -  Requesting setting type for IMGP/MLD control.
 *      arg -   value to the control set.
 * Returns:
 *      BCM_E_xxxx 
 */
static int
_bcm_robo_igmp_mld_control_set(int unit, bcm_switch_control_t type, int arg)
{
    int     snoop_type = -1, snoop_mode = -1, tocpu_old = 0, old_mode = -1;
    bcm_switch_control_t    to_cpu_bcm_type = bcmSwitch__Count;

    if ((type == bcmSwitchIgmpPktToCpu) || 
            (type == bcmSwitchIgmpPktDrop)){
        to_cpu_bcm_type = bcmSwitchIgmpPktToCpu;
        snoop_type = DRV_IGMP_MLD_TYPE_IGMP;
    } else if ((type == bcmSwitchIgmpQueryToCpu) || 
            (type == bcmSwitchIgmpQueryDrop)){
        to_cpu_bcm_type = bcmSwitchIgmpQueryToCpu;
        snoop_type = DRV_IGMP_MLD_TYPE_IGMP_QUERY;
    } else if ((type == bcmSwitchIgmpReportLeaveToCpu) || 
            (type == bcmSwitchIgmpReportLeaveDrop)){
        to_cpu_bcm_type = bcmSwitchIgmpReportLeaveToCpu;
        snoop_type = DRV_IGMP_MLD_TYPE_IGMP_REPLEV;
    } else if ((type == bcmSwitchIgmpUnknownToCpu) || 
            (type == bcmSwitchIgmpUnknownDrop)){
        to_cpu_bcm_type = bcmSwitchIgmpUnknownToCpu;
        snoop_type = DRV_IGMP_MLD_TYPE_IGMP_UNKNOWN;
    } else if ((type == bcmSwitchMldPktToCpu) || 
            (type == bcmSwitchMldPktDrop)){
        to_cpu_bcm_type = bcmSwitchMldPktToCpu;
        snoop_type = DRV_IGMP_MLD_TYPE_MLD;
    } else if ((type == bcmSwitchMldQueryToCpu) || 
            (type == bcmSwitchMldQueryDrop)){
        to_cpu_bcm_type = bcmSwitchMldQueryToCpu;
        snoop_type = DRV_IGMP_MLD_TYPE_MLD_QUERY;
    } else if ((type == bcmSwitchMldReportDoneToCpu) || 
            (type == bcmSwitchMldReportDoneDrop)){
        to_cpu_bcm_type = bcmSwitchMldReportDoneToCpu;
        snoop_type = DRV_IGMP_MLD_TYPE_MLD_REPDONE;
    } else if ((type == bcmSwitchIgmpQueryFlood) || \
            (type == bcmSwitchIgmpReportLeaveFlood) || \
            (type == bcmSwitchIgmpUnknownFlood) || \
            (type == bcmSwitchMldQueryFlood) || \
            (type == bcmSwitchMldReportDoneFlood)){
        /* ROBO chips have no flood design */
        return BCM_E_UNAVAIL;
    }

    /* check the currnet ToCpu configuration */
    BCM_IF_ERROR_RETURN(DRV_IGMP_MLD_SNOOP_MODE_GET(unit, 
            snoop_type, &old_mode));

    tocpu_old = (old_mode == DRV_IGMP_MLD_MODE_DISABLE) ? FALSE : TRUE;

    /* assigning the forwarding mode :
     *  
     *  Working Rule :
     *       ToCpu  Drop  => [Mode]
     *  ------------------------------------
     *  (1):   F     F    => [Disable]
     *  (2):   F     T    => [Discard] (No forwarding to front port and CPU)
     *  (3):   T     F    => [Snoop] (Forwarding to front port and COPY to CPU)
     *  (4):   T     T    => [Trap] (Redirect to CPU only )
     *  ------------------------------------
     *
     *  1. If ToCpu's new value is no change, reutrn BCM_E_NONE directly.
     *  2. If the the case is (2) in above, return BCM_E_CONFIG directly.
     */
    if (tocpu_old == TRUE){
        if (type == to_cpu_bcm_type){
            /* for ToCpu process */
            if (arg == TRUE){
                /* the same configuration, no change! */
                return BCM_E_NONE;
            } else {
                snoop_mode = DRV_IGMP_MLD_MODE_DISABLE;
            }
        } else {
            /* for Drop process */

            snoop_mode = (arg == TRUE) ? 
                    DRV_IGMP_MLD_MODE_TRAP : DRV_IGMP_MLD_MODE_SNOOP;
        }
    } else {
        if (type == to_cpu_bcm_type){
            /* for ToCpu process */
            if (arg == TRUE){
                snoop_mode = DRV_IGMP_MLD_MODE_ENABLE;
            } else {
                /* the same configuration, no change! */
                return BCM_E_NONE;
            }
        } else {
            /* for Drop process */
            if (arg == TRUE){
                /* Special case for the pure drop action is not supported in 
                 *  ROBO chips 
                 */
                return BCM_E_CONFIG;
            } else {
                /* the same configuration, no change! */
                return BCM_E_NONE;
            }
        }
    }

    assert((snoop_type != -1) && (snoop_mode != -1));

    /* set the proper configuration for IGMP/MLD snooping feature */
    return DRV_IGMP_MLD_SNOOP_MODE_SET(unit, snoop_type, snoop_mode);
}

/*
 * Function:
 *  _bcm_robo_igmp_mld_control_get
 * Description:
 *  Get the IGMP/MLD related configuration.
 * Parameters:
 *      unit -  RoboSwitch PCI device unit number (driver internal).
 *      type -  Requesting setting type for IMGP/MLD control.
 *      arg -   (OUT)value to the control get.
 * Returns:
 *      BCM_E_xxxx 
 */
static int
_bcm_robo_igmp_mld_control_get(int unit, bcm_switch_control_t type, int *arg)
{
    int     snoop_type = -1, snoop_mode = -1;
    bcm_switch_control_t    to_cpu_bcm_type = bcmSwitch__Count;

    if ((type == bcmSwitchIgmpPktToCpu) || 
            (type == bcmSwitchIgmpPktDrop)){
        to_cpu_bcm_type = bcmSwitchIgmpPktToCpu;
        snoop_type = DRV_IGMP_MLD_TYPE_IGMP;
    } else if ((type == bcmSwitchIgmpQueryToCpu) || 
            (type == bcmSwitchIgmpQueryDrop)){
        to_cpu_bcm_type = bcmSwitchIgmpQueryToCpu;
        snoop_type = DRV_IGMP_MLD_TYPE_IGMP_QUERY;
    } else if ((type == bcmSwitchIgmpReportLeaveToCpu) || 
            (type == bcmSwitchIgmpReportLeaveDrop)){
        to_cpu_bcm_type = bcmSwitchIgmpReportLeaveToCpu;
        snoop_type = DRV_IGMP_MLD_TYPE_IGMP_REPLEV;
    } else if ((type == bcmSwitchIgmpUnknownToCpu) || 
            (type == bcmSwitchIgmpUnknownDrop)){
        to_cpu_bcm_type = bcmSwitchIgmpUnknownToCpu;
        snoop_type = DRV_IGMP_MLD_TYPE_IGMP_UNKNOWN;
    } else if ((type == bcmSwitchMldPktToCpu) || 
            (type == bcmSwitchMldPktDrop)){
        to_cpu_bcm_type = bcmSwitchMldPktToCpu;
        snoop_type = DRV_IGMP_MLD_TYPE_MLD;
    } else if ((type == bcmSwitchMldQueryToCpu) || 
            (type == bcmSwitchMldQueryDrop)){
        to_cpu_bcm_type = bcmSwitchMldQueryToCpu;
        snoop_type = DRV_IGMP_MLD_TYPE_MLD_QUERY;
    } else if ((type == bcmSwitchMldReportDoneToCpu) || 
            (type == bcmSwitchMldReportDoneDrop)){
        to_cpu_bcm_type = bcmSwitchMldReportDoneToCpu;
        snoop_type = DRV_IGMP_MLD_TYPE_MLD_REPDONE;
    } else if ((type == bcmSwitchIgmpQueryFlood) || \
            (type == bcmSwitchIgmpReportLeaveFlood) || \
            (type == bcmSwitchIgmpUnknownFlood) || \
            (type == bcmSwitchMldQueryFlood) || \
            (type == bcmSwitchMldReportDoneFlood)){
        /* ROBO chips have no flood design */
        return BCM_E_UNAVAIL;
    }

    /* check the currnet ToCpu configuration */
    BCM_IF_ERROR_RETURN(DRV_IGMP_MLD_SNOOP_MODE_GET(unit, 
            snoop_type, &snoop_mode));

    if (snoop_mode == DRV_IGMP_MLD_MODE_DISABLE) {
        /* for both ToCpu and Drop type, the value is FALSE when the current 
         *  mode is disabled.
         */
        *arg = FALSE;
    } else {
        if (to_cpu_bcm_type == type){
            *arg = TRUE;
        } else {
            *arg = (snoop_mode == DRV_IGMP_MLD_MODE_TRAP) ? TRUE : FALSE;
        }
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *  bcm_robo_switch_control_set
 * Description:
 *  Set traffic-to-CPU configuration parameters for the switch.
 *      All switch chip ports will be configured with the same settings.
 * Parameters:
 *  unit - RoboSwitch PCI device unit number (driver internal).
 *      type - The desired configuration parameter to modify.
 *      arg - The value with which to set the parameter.
 * Returns:
 *      BCM_E_xxxx 
 */
int 
bcm_robo_switch_control_set(int unit,
                  bcm_switch_control_t type,
                  int arg)
{
    uint8   tmp_state = 0;
    uint32  type_mask = 0;
    uint32  snoop_type_mask = 0;
    uint32  bypass_modes = 0, temp_modes = 0;
    int     en = 0, rv = BCM_E_UNAVAIL;
    soc_port_t port = 0;
    soc_pbmp_t bmp;

    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "BCM API : %s(),type=%d,arg=%d\n"), 
                 FUNCTION_NAME(), type, arg));

    BCM_PBMP_CLEAR(bmp);

    /* special case indicated in API guide that this control type is defined
     *  as READ only to report HW's trunk failover detection on Ether port.
     */
    if (type == bcmSwitchFailoverEtherTrunk){
        return BCM_E_UNAVAIL;
    }

    /* Process DoS Attack cases */
    switch(type) {
        /* global flags and variable setting */
        case bcmSwitchDosAttackV4FirstFrag:
            if (soc_feature(unit, soc_feature_field)) {
                return (_bcm_robo_dos_attack_set(unit, DOS_ATTACK_ALL_PORT, type, arg));
            } else if (soc_feature(unit, soc_feature_hw_dos_prev)) {
                return (_bcm_robo_hw_dos_attack_set(unit, type, arg));
            } else {
                return BCM_E_UNAVAIL;
            }
            break;
        /* apply to all front ports */
        case bcmSwitchDosAttackMinTcpHdrSize:
        case bcmSwitchDosAttackToCpu:
        case bcmSwitchDosAttackSipEqualDip:
        case bcmSwitchDosAttackTcpFlags:
        case bcmSwitchDosAttackTcpFlagsSF:
        case bcmSwitchDosAttackTcpFlagsFUP:
        case bcmSwitchDosAttackSynFrag:
        case bcmSwitchDosAttackFlagZeroSeqZero:
        case bcmSwitchDosAttackL4Port:
        case bcmSwitchDosAttackTcpPortsEqual:
        case bcmSwitchDosAttackUdpPortsEqual:
        case bcmSwitchDosAttackTcpFrag:
        case bcmSwitchDosAttackTcpOffset:
        case bcmSwitchDosAttackIcmpFragments:
        case bcmSwitchDosAttackIcmpPktOversize:
        case bcmSwitchDosAttackIcmpV6PingSize:
        case bcmSwitchDosAttackIcmp:
        case bcmSwitchDosAttackPingFlood:
        case bcmSwitchDosAttackSynFlood:
        case bcmSwitchDosAttackTcpSmurf:
        case bcmSwitchDosAttackTcpHdrPartial:
            if (soc_feature(unit, soc_feature_hw_dos_prev)) {
                rv = _bcm_robo_hw_dos_attack_set(unit, type, arg);
            }
            if ((rv == BCM_E_UNAVAIL) && 
                (soc_feature(unit, soc_feature_field))) {
                rv = _bcm_robo_dos_attack_set(unit, DOS_ATTACK_ALL_PORT, type, arg);
            }
            return rv;
            break;
        case bcmSwitchTimeSyncEthertype:
        case bcmSwitchTimeSyncDestMacOui:
        case bcmSwitchTimeSyncDestMacNonOui:
            rv = _bcm_robo_switch_eav_mac_ethertype_set(unit, type, arg);
            return rv;
            break;
        case bcmSwitchTimeSyncPktToCpu:
        case bcmSwitchTimeSyncPktDrop:
        case bcmSwitchTimeSyncPktFlood:
            rv = _bcm_robo_switch_eav_action_set(unit, type, arg);
            return rv;
            break;
        default:
            break;
    }

    /* Process vlan related cases */
    switch(type) {
        case bcmSwitchGvrpToCpu:
            return (DRV_VLAN_PROP_SET(unit, DRV_VLAN_PROP_GVRP_TO_CPU, arg));
            break;
        case bcmSwitchGmrpToCpu:
            return (DRV_VLAN_PROP_SET(unit, DRV_VLAN_PROP_GMRP_TO_CPU, arg));
            break;
        case bcmSwitchUnknownVlanToCpu:
            return (bcm_robo_vlan_control_set(unit, 
                    bcmVlanUnknownToCpu, arg));
            break;
            
        case bcmSwitchSharedVlanEnable:
            return (bcm_robo_vlan_control_set(unit, 
                    bcmVlanShared, arg));
            break;
        case bcmSwitchMcastFloodDefault:
            return(_bcm_robo_vlan_flood_default_set(unit, arg));
            break;
        default:
            break;
    }

    /* Process the EEE related cases */
    switch (type) {
        case bcmSwitchEEEPipelineTime:
            if (soc_feature (unit, soc_feature_eee)) {
                rv = DRV_DEV_PROP_SET(unit, 
                    DRV_DEV_PROP_EEE_PIPELINE_TIMER, arg);
            } else {
                rv = BCM_E_UNAVAIL;
            }
            return rv;
        case bcmSwitchEEEGlobalCongestionThreshold:
            if (soc_feature (unit, soc_feature_eee)) {
                rv = DRV_DEV_PROP_SET(unit, 
                    DRV_DEV_PROP_EEE_GLOBAL_CONG_THRESH, arg);
            } else {
                rv = BCM_E_UNAVAIL;
            }
            return rv;
        default:
            break;
    }

    /* system basis bypass mode process : 
     *  - Set bypass mode on the indicated mode bits only.
     *
     * Note :
     *  1. 'bcmSwitchFiltersControlModes' is for setting on filter bypass
     *      control, but this symbol definition in semantic view is opposite
     *      to the controling target. (i.e. filter v.s. filter bypass)
     *      >> To reverted the setting value to prevent user been confused.
     */
    if (type == bcmSwitchFiltersControlModes){
        BCM_IF_ERROR_RETURN(_bcm_robo_bypass_modes_get(
                unit, bcmSwitchFiltersControlList, &bypass_modes));

        /* check if any invalid bypass mode set */
        if (arg & (~bypass_modes)){
            rv = BCM_E_PARAM;
        } else {
            /* to revert the user setting value */
            temp_modes = (~arg) & bypass_modes;
            rv = _bcm_robo_bypass_modes_set(unit, temp_modes);
        }
        return rv;
    } else if (type == bcmSwitchFiltersControlList){
        /* 'bcmSwitchFiltersControlList' is for get only */
        return BCM_E_UNAVAIL;
    }

    /* system basis LED configuration */
    if (type == bcmSwitchLedFuncGrp0 || type == bcmSwitchLedFuncGrp1){
        uint32  led_group = 0;

        led_group = (type == bcmSwitchLedFuncGrp0) ? 
                BCM_LED_FUNCGRP_0 : BCM_LED_FUNCGRP_1;
        rv = DRV_LED_FUNC_SET(unit, led_group, arg);

        return rv;
    }

    /* Mac low power mode */
    if (type == bcmSwitchMacLowPower) {
        
        rv = DRV_DEV_PROP_SET(unit, 
            DRV_DEV_PROP_LOW_POWER_ENABLE, arg);

        return rv;
    }

    /* AutoVoIP wildcard rule */
    switch (type) {
        case bcmSwitchUdpPktActionEnable:
            /* Enable AutoVoIP wildcard rule */
            if (SOC_IS_BLACKBIRD2(unit)) {
                temp_modes = (arg)? TRUE : FALSE;
                rv = DRV_CFP_CONTROL_SET(unit, 
                    DRV_CFP_UDP_DEFAULT_ACTION_ENABLE, 0, temp_modes);
                return rv;
            } else {
                return BCM_E_UNAVAIL;
            }
            break;
        case bcmSwitchUdpPktRedirectPbmp:
             if (SOC_IS_BLACKBIRD2(unit)) {
                temp_modes = arg;
                rv = DRV_CFP_CONTROL_SET(unit, 
                        DRV_CFP_UDP_DEFAULT_ACTION, 
                        DRV_CFP_ACT_OB_REDIRECT, temp_modes);
                return rv;
            } else {
                return BCM_E_UNAVAIL;
            }
            break;
        case bcmSwitchUdpPktCopyToPbmp:
            if (SOC_IS_BLACKBIRD2(unit)) {
                temp_modes = arg;
                rv = DRV_CFP_CONTROL_SET(unit, 
                        DRV_CFP_UDP_DEFAULT_ACTION, 
                        DRV_CFP_ACT_OB_APPEND, temp_modes);
                return rv;
            } else {
                return BCM_E_UNAVAIL;
            }
            break;
        case bcmSwitchUdpPktNewTc:
            if (SOC_IS_BLACKBIRD2(unit)) {
                temp_modes = arg;
                rv = DRV_CFP_CONTROL_SET(unit, 
                        DRV_CFP_UDP_DEFAULT_ACTION, 
                        DRV_CFP_ACT_CHANGE_TC, temp_modes);
                return rv;
            } else {
                return BCM_E_UNAVAIL;
            }
            break;
        default:
            break;
    }

    /* device misc control realted */
    switch(type) {
        case bcmSwitchPortBasedQos :
            tmp_state = (arg) ? TRUE : FALSE;
            BCM_IF_ERROR_RETURN(DRV_QUEUE_MAPPING_TYPE_SET(
                    unit, PBMP_ALL(unit), DRV_QUEUE_MAP_PORT, tmp_state));
            return BCM_E_NONE;
            break;
        case bcmSwitchL2McastAllRouterPortsAddEnable:
            SOC_MCAST_ADD_ALL_ROUTER_PORTS(unit) = arg ? 1 : 0;
            return BCM_E_NONE;
            break;
        case bcmSwitchUseGport :
            SOC_USE_GPORT_SET(unit, arg);
            return BCM_E_NONE;
            break;
        case bcmSwitchIgmpMldToCpu : 
            if (soc_feature(unit, soc_feature_igmp_mld_support)) {
                if (arg < 0 || arg > _BCM_IGMPMLD_MODE_MAXID){
                    return BCM_E_PARAM;
                } else {
                    en = (arg == _BCM_IGMPMLD_MODE_CPU_TRAP) ? 
                                DRV_SNOOP_MODE_CPU_TRAP : 
                            ((arg == _BCM_IGMPMLD_MODE_CPU_SNOOP) ? 
                                DRV_SNOOP_MODE_CPU_SNOOP : 
                                DRV_SNOOP_MODE_NONE);
                    return (_bcm_robo_dev_control_set(unit, type, en));
                    break;
                }
            } else {
                return BCM_E_UNAVAIL;
            }
            break;
        case bcmSwitchJumboFrameDrop :
        case bcmSwitchDroppedJumboFrameDisLearn :
        case bcmSwitchRangeErrDrop :
        case bcmSwitchDroppedRangeErrDisLearn :
        case bcmSwitchMcastSaDrop :
        case bcmSwitchIngressRateLimitIncludeIFG :
        case bcmSwitchDestMacZeroDrop :
        case bcmSwitchArpDhcpToCpu :
        case bcmSwitchReservedMcastLearn:
        case bcmSwitchUnknownUcastToCpu:
        case bcmSwitchUnknownMcastToCpu:
        case bcmSwitchL2LearnLimitToCpu :
        case bcmSwitchPPPoESessionPktDscpRemarkEnable :
            return (_bcm_robo_dev_control_set(unit, type, arg));
            break;
        case bcmSwitchPPPoESessionParseEnable:
            type_mask = (arg)? TRUE : FALSE;
            bmp = PBMP_ALL(unit);
            return (DRV_PORT_SET(unit, bmp,
                DRV_PORT_PROP_PPPOE_PARSE_EN, type_mask));
            break;
        case bcmSwitchPPPoESessionEthertype:
            type_mask = (uint32)arg;
            return (DRV_DEV_PROP_SET(unit, 
                DRV_DEV_PROP_PPPOE_SESSION_ETYPE, type_mask));
            break;
        default:
            break;
    }

    /* GNATS 40759 : redesign IGMP/MLD trap/snoop */
    if (_BCM_SWITCH_IGMP_MLD_TYPES(type)){

        rv = _bcm_robo_igmp_mld_control_set(unit, type, arg);
        return rv;
    }
        
    /* get non-igmp/non-mld trap configuring status */
    BCM_PBMP_ASSIGN(bmp, PBMP_ALL(unit));
    BCM_IF_ERROR_RETURN(DRV_TRAP_GET
        (unit, port, &type_mask));
    
    /* bcm53115, bcm53118 new feature on supporting different CPU snooping/traping 
     * behavior per separated the IGMP/MLD packet types.
     *
     * The designed process for this new feature are :
     *  1. set TRUE at bcmSwitchxxxxToCPU in bcm53115, bcm53118 :
     *      - Do nothing if "toCPU" is set already!(can be snoop||trap)
     *      - else set to SNOOPING by default behavior.
     *  2. set FALSE at bcmSwitchxxxxToCPU in bcm53115, bcm53118 :
     *      - set to disable on SNOOPING and TRAPING by default behavior.
     *  3. set TRUE at bcmSwitchxxxxDrop in bcm53115, bcm53118 :
     *      - Return BCM_E_UNAVAIL if "toCPU" is not set for chip limitation!
     *          (i.e. neither snooping nor traping)
     *      - Do nothing if "TRAPING" is set already!
     *      - else set to "TRAPING" by default behavior.
     *  4. set FALSE at bcmSwitchxxxxDrop in bcm53115, bcm53118 :
     *      - Do nothing if "toCPU" is reset or if "SNOOPING" is set already!
     *      - else set to SNOOPING by default behavior.
     */ 
    if (SOC_IS_ROBO_ARCH_VULCAN(unit)){
        int     snoop_action = FALSE;
        int     trap_action = FALSE;
        
        /* get snoop configuring status */
        BCM_IF_ERROR_RETURN(DRV_SNOOP_GET
            (unit, &snoop_type_mask));
        
        switch(type) {
        /* non-igmp & non-mld request */
        case bcmSwitchArpReplyToCpu:    /* ARP */
            if (arg){
                if (snoop_type_mask & DRV_SNOOP_ARP){
                    /* enabled already */
                    return  BCM_E_NONE;
                } else {
                    snoop_type_mask |= DRV_SNOOP_ARP;
                }
            } else {
                snoop_type_mask &= ~DRV_SNOOP_ARP;
            }
            snoop_action = TRUE;
            break;

        case bcmSwitchDhcpPktToCpu:     /* DHCP */
            if (arg){
                if (snoop_type_mask & DRV_SNOOP_DHCP){
                    /* enabled already */
                    return  BCM_E_NONE;
                } else {
                    snoop_type_mask |= DRV_SNOOP_DHCP;
                }
            } else {
                snoop_type_mask &= ~DRV_SNOOP_DHCP;
            }
            snoop_action = TRUE;
            break;

        case bcmSwitchIcmpRedirectToCpu:
            /* bcm53115, bcm53118 support redirect to CPU about ICMP is ICMPv6 only. 
             *  The ICMPv4 can only work at snoop but not trap feature.
             */
            if (arg){
                if (type_mask & DRV_SWITCH_TRAP_ICMPV6){
                    /* enabled already */
                    return  BCM_E_NONE;
                } else {
                    type_mask |= DRV_SWITCH_TRAP_ICMPV6;
                    type_mask &= ~DRV_SWITCH_TRAP_ICMPV6_DISABLE;
                    trap_action = TRUE;
                }
            } else {
                if (type_mask & DRV_SWITCH_TRAP_ICMPV6){
                    
                    type_mask &= ~DRV_SWITCH_TRAP_ICMPV6;
                    type_mask |= DRV_SWITCH_TRAP_ICMPV6_DISABLE;
                    trap_action = TRUE;
                } else {
                    /* disabled already !!
                     *  two cases might happened in this section.
                     *  1. icmp v6 snoop enabled and trap disabled.
                     *  2. icmp v6 snoop and trap are both disabled
                     */
                    return  BCM_E_NONE;
                }
            }
            break;

        default :
            break;
        }   /* switch case */
        
        if (snoop_action || trap_action){
            if (snoop_action){    /* doing snoop configuration */
            
                LOG_VERBOSE(BSL_LS_BCM_COMMON,
                            (BSL_META_U(unit,
                                        "%s,%d,snoop set with mask=0x%X\n"), 
                             FUNCTION_NAME(), __LINE__,snoop_type_mask));
                BCM_IF_ERROR_RETURN(DRV_SNOOP_SET
                     (unit, snoop_type_mask));
            }
            
            if (trap_action){    /* doing trap configuration */
            
                LOG_VERBOSE(BSL_LS_BCM_COMMON,
                            (BSL_META_U(unit,
                                        "%s,%d,trap set with mask=0x%X\n"), 
                             FUNCTION_NAME(), __LINE__, type_mask));
                BCM_IF_ERROR_RETURN(DRV_TRAP_SET
                    (unit, bmp, type_mask));
            }
            /* reutn for type parameter is handled */
            return BCM_E_NONE;
        }
        
    }   /* SOC_IS_VULCAN(unit), SOC_IS_BLACKBIRD(unit) */

    switch(type) {
    case bcmSwitchBpduToCPU:        
       if (type_mask & DRV_SWITCH_TRAP_BPDU1){
            if (arg){
                return  BCM_E_NONE;
            } else {
                type_mask &= ~DRV_SWITCH_TRAP_BPDU1;
            }
        } else {
            if (arg){
                type_mask |= DRV_SWITCH_TRAP_BPDU1;
            } else {
                return  BCM_E_NONE;
            }
        }
        break;
    case bcmSwitch8021xToCPU:
       if (type_mask & DRV_SWITCH_TRAP_8021X){
            if (arg){
                return  BCM_E_NONE;
            } else {
                type_mask &= ~DRV_SWITCH_TRAP_8021X;
            }
        } else {
            if (arg){
                type_mask |= DRV_SWITCH_TRAP_8021X;
            } else {
                return  BCM_E_NONE;
            }
        }
        break;
    case bcmSwitchBcstToCPU:
        if (type_mask & DRV_SWITCH_TRAP_BCST) {
            if (arg) {
                return BCM_E_NONE;
            } else {
                type_mask &= ~DRV_SWITCH_TRAP_BCST;
            }
        } else {
            if (arg) {
                type_mask |= DRV_SWITCH_TRAP_BCST;
            } else {
                return BCM_E_NONE;
            }
        }
        break;
    default:
        return BCM_E_UNAVAIL;
        break;
    }
    
    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "bcm_robo_switch_control_set():\n"
                            "\t bcm_type=%d, drv_type=0x%x\n"),
                 type, type_mask));
    BCM_IF_ERROR_RETURN(DRV_TRAP_SET
        (unit, bmp, type_mask));
    
    return BCM_E_NONE;

}               

/*
 * Function:
 *  bcm_robo_switch_control_get
 * Description:
 *  Get the current traffic-to-CPU configuration parameters for the switch.
 *      All switch chip ports are configured with the same settings.
 * Parameters:
 *  unit - RoboSwitch PCI device unit number (driver internal).
 *      type - The desired configuration parameter to retrieve.
 *      arg - Pointer to where the retrieved value will be written.
 * Returns:
 *      BCM_E_xxxx 
 */
int 
bcm_robo_switch_control_get(int unit,
                  bcm_switch_control_t type,
                  int *arg)
{
    uint8   tmp_state = 0;
    uint32  type_mask = 0;
    uint32  snoop_type_mask = 0, value = 0;
    uint32  bypass_modes = 0;
    int rv = BCM_E_UNAVAIL, temp = 0;
    soc_port_t port = 0;

    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "BCM API : %s(),type=%d\n"), FUNCTION_NAME(), type));

    /* special case indicated in API guide that this control type is defined
     *  as READ only to report HW's trunk failover detection on Ether port.
     */
    if (type == bcmSwitchFailoverEtherTrunk){
        /* ROBO's trunk failover is HW autodetected */
        *arg = 1;
        return BCM_E_NONE;
    }
    
    switch(type) {
        case bcmSwitchDosAttackV4FirstFrag:
            if (soc_feature(unit, soc_feature_field)) {
                return(_bcm_robo_dos_attack_get(unit, DOS_ATTACK_ALL_PORT, type, arg));
            } else if (soc_feature(unit, soc_feature_hw_dos_prev)) {
                return(_bcm_robo_hw_dos_attack_get(unit, type, arg));
            } else {
                return BCM_E_UNAVAIL;
            }
            break;
        case bcmSwitchDosAttackMinTcpHdrSize:
        case bcmSwitchDosAttackToCpu:
        case bcmSwitchDosAttackSipEqualDip:
        case bcmSwitchDosAttackTcpFlags:
        case bcmSwitchDosAttackTcpFlagsSF:
        case bcmSwitchDosAttackTcpFlagsFUP:
        case bcmSwitchDosAttackSynFrag:
        case bcmSwitchDosAttackFlagZeroSeqZero:
        case bcmSwitchDosAttackL4Port:
        case bcmSwitchDosAttackTcpPortsEqual:
        case bcmSwitchDosAttackUdpPortsEqual:
        case bcmSwitchDosAttackTcpFrag:
        case bcmSwitchDosAttackTcpOffset:
        case bcmSwitchDosAttackIcmpFragments:
        case bcmSwitchDosAttackIcmpPktOversize:
        case bcmSwitchDosAttackIcmpV6PingSize:
        case bcmSwitchDosAttackIcmp:
        case bcmSwitchDosAttackPingFlood:
        case bcmSwitchDosAttackSynFlood:
        case bcmSwitchDosAttackTcpSmurf:
        case bcmSwitchDosAttackTcpHdrPartial:
            if (soc_feature(unit, soc_feature_hw_dos_prev)) {
                rv = _bcm_robo_hw_dos_attack_get(unit, type, arg);
            }
            if ((rv == BCM_E_UNAVAIL) && 
                (soc_feature(unit, soc_feature_field))) {
                rv = _bcm_robo_dos_attack_get(unit, 
                            DOS_ATTACK_ALL_PORT, type, arg);
            }
            return rv;
            break;
        case bcmSwitchTimeSyncEthertype:
        case bcmSwitchTimeSyncDestMacOui:
        case bcmSwitchTimeSyncDestMacNonOui:
            rv = _bcm_robo_switch_eav_mac_ethertype_get(unit, type, arg);
            return rv;
            break;
        case bcmSwitchTimeSyncPktToCpu:
        case bcmSwitchTimeSyncPktDrop:
        case bcmSwitchTimeSyncPktFlood:
            rv = _bcm_robo_switch_eav_action_get(unit, type, arg);
            return rv;
            break;
        default:
            break;
    }

    /* Process vlan related cases */
    switch(type) {
        case bcmSwitchGvrpToCpu:
            return (DRV_VLAN_PROP_GET(unit, DRV_VLAN_PROP_GVRP_TO_CPU, 
                    (uint32 *)arg));
            break;
        case bcmSwitchGmrpToCpu:
            return (DRV_VLAN_PROP_GET(unit, DRV_VLAN_PROP_GMRP_TO_CPU, 
                    (uint32 *)arg));
            break;
        case bcmSwitchUnknownVlanToCpu:
            return (bcm_robo_vlan_control_get(unit, bcmVlanUnknownToCpu, 
                    arg));
            break;

        case bcmSwitchSharedVlanEnable:
            return (bcm_robo_vlan_control_get(unit, bcmVlanShared, 
                    arg));
            break;
        case bcmSwitchMcastFloodDefault: {
            bcm_vlan_mcast_flood_t mode;
            BCM_IF_ERROR_RETURN(_bcm_robo_vlan_flood_default_get(unit, &mode));
            *arg = mode;
            return BCM_E_NONE;
        }
        default:
            break;
    }

    /* Process the EEE related cases */
    switch (type) {
        case bcmSwitchEEEPipelineTime:
            if (soc_feature (unit, soc_feature_eee)) {
                rv = DRV_DEV_PROP_GET(unit, 
                    DRV_DEV_PROP_EEE_PIPELINE_TIMER, &value);
                *arg = value;
            } else {
                rv = BCM_E_UNAVAIL;
            }
            return rv;
        case bcmSwitchEEEGlobalCongestionThreshold:
            if (soc_feature (unit, soc_feature_eee)) {
                rv = DRV_DEV_PROP_GET(unit, 
                    DRV_DEV_PROP_EEE_GLOBAL_CONG_THRESH, &value);
                *arg = value;
            } else {
                rv = BCM_E_UNAVAIL;
            }
            return rv;
        default:
            break;
    }

    /* system basis bypass mode process : 
     *  - Get this type will retrieve all enabled bypass mode bitmap.
     *
     * Note :
     *  1. 'bcmSwitchFiltersControlModes' is for getting on filter bypass
     *      control, but this symbol definition in semantic view is opposite
     *      to the controling target. (i.e. filter v.s. filter bypass)
     *      >> To reverted the getting value to prevent user been confused.
     */
    if (type == bcmSwitchFiltersControlModes || 
            type == bcmSwitchFiltersControlList){
        BCM_IF_ERROR_RETURN(_bcm_robo_bypass_modes_get(unit, 
                bcmSwitchFiltersControlList, &type_mask));
		
        if (type == bcmSwitchFiltersControlList){
            *arg = type_mask;
            rv = BCM_E_NONE;
        } else {
            /* type == bcmSwitchFiltersControlModes 
             *   - value will be reverted.
             */
            BCM_IF_ERROR_RETURN(_bcm_robo_bypass_modes_get(unit, 
                    type, &bypass_modes));
            *arg = (~bypass_modes) & type_mask;
            rv = BCM_E_NONE;
        }	
        return rv;
    }

    /* system basis LED configuration */
    if (type == bcmSwitchLedFuncGrp0 || type == bcmSwitchLedFuncGrp1){
        uint32  led_group = 0, led_functions = 0;

        led_group = (type == bcmSwitchLedFuncGrp0) ? 
                BCM_LED_FUNCGRP_0 : BCM_LED_FUNCGRP_1;
        rv = DRV_LED_FUNC_GET(unit, led_group, &led_functions);
        *arg = led_functions;
        
        return rv;
    }

    /* Mac low power mode */
    if (type == bcmSwitchMacLowPower) {
        uint32 freq_khz = 0;

        rv = DRV_DEV_PROP_GET(unit, 
            DRV_DEV_PROP_LOW_POWER_ENABLE, &freq_khz);
        *arg = freq_khz;

        return rv;
    }

    /* AutoVoIP wildcard rule */
    switch (type) {
        case bcmSwitchUdpPktActionEnable:
            /* Enable AutoVoIP wildcard rule */
            if (SOC_IS_BLACKBIRD2(unit)) {
                rv = DRV_CFP_CONTROL_GET(unit, 
                    DRV_CFP_UDP_DEFAULT_ACTION_ENABLE, 0, &value);
                *arg = value;
                return rv;
            } else {
                return BCM_E_UNAVAIL;
            }
            break;
        case bcmSwitchUdpPktRedirectPbmp:
             if (SOC_IS_BLACKBIRD2(unit)) {
                rv = DRV_CFP_CONTROL_GET(unit, 
                        DRV_CFP_UDP_DEFAULT_ACTION, 
                        DRV_CFP_ACT_OB_REDIRECT, &value);
                if (rv == SOC_E_DISABLED) {
                    *arg = 0;
                    rv = BCM_E_NONE;
                } else {
                    *arg = value;
                }
                return rv;
            } else {
                return BCM_E_UNAVAIL;
            }
            break;
        case bcmSwitchUdpPktCopyToPbmp:
            if (SOC_IS_BLACKBIRD2(unit)) {
                rv = DRV_CFP_CONTROL_GET(unit, 
                        DRV_CFP_UDP_DEFAULT_ACTION, 
                        DRV_CFP_ACT_OB_APPEND, &value);
                if (rv == SOC_E_DISABLED) {
                    *arg = 0;
                    rv = BCM_E_NONE;
                } else {
                    *arg = value;
                }
                return rv;
            } else {
                return BCM_E_UNAVAIL;
            }
            break;
        case bcmSwitchUdpPktNewTc:
            if (SOC_IS_BLACKBIRD2(unit)) {
                rv = DRV_CFP_CONTROL_GET(unit, 
                        DRV_CFP_UDP_DEFAULT_ACTION, 
                        DRV_CFP_ACT_CHANGE_TC, &value);
                if (rv == SOC_E_DISABLED) {
                    *arg = 0;
                    rv = BCM_E_NONE;
                } else {
                    *arg = value;
                }
                return rv;
            } else {
                return BCM_E_UNAVAIL;
            }
            break;
        default:
            break;
    }

    /* device misc control realted */
    switch(type) {
        case bcmSwitchPortBasedQos :
            BCM_IF_ERROR_RETURN(DRV_QUEUE_MAPPING_TYPE_GET(
                    unit, 0, DRV_QUEUE_MAP_PORT, &tmp_state));
            *arg = tmp_state;
            return BCM_E_NONE;
            break;
        case bcmSwitchL2McastAllRouterPortsAddEnable:
            *arg = SOC_MCAST_ADD_ALL_ROUTER_PORTS(unit);
            return BCM_E_NONE;
        case bcmSwitchUseGport :
            *arg = SOC_USE_GPORT(unit);
            return BCM_E_NONE;
            break;
        case bcmSwitchIgmpMldToCpu : 
            if (soc_feature(unit, soc_feature_igmp_mld_support)) {
                rv = _bcm_robo_dev_control_get(unit, type, &temp);
                if (rv == BCM_E_NONE){
                    *arg = (temp == DRV_SNOOP_MODE_CPU_TRAP) ? 
                                _BCM_IGMPMLD_MODE_CPU_TRAP : 
                            ((temp == DRV_SNOOP_MODE_CPU_SNOOP) ? 
                                _BCM_IGMPMLD_MODE_CPU_SNOOP : 
                                _BCM_IGMPMLD_MODE_NONE);
                }
                return rv;
            } else {
                return BCM_E_UNAVAIL;
            }
            break;
        case bcmSwitchJumboFrameDrop :
        case bcmSwitchDroppedJumboFrameDisLearn :
        case bcmSwitchRangeErrDrop :
        case bcmSwitchDroppedRangeErrDisLearn :
        case bcmSwitchMcastSaDrop :
        case bcmSwitchIngressRateLimitIncludeIFG :
        case bcmSwitchDestMacZeroDrop :
        case bcmSwitchArpDhcpToCpu :
        case bcmSwitchReservedMcastLearn:
        case bcmSwitchUnknownUcastToCpu:
        case bcmSwitchUnknownMcastToCpu:
        case bcmSwitchL2LearnLimitToCpu :
        case bcmSwitchPPPoESessionPktDscpRemarkEnable :
            return (_bcm_robo_dev_control_get(unit, type, arg));
            break;
        case bcmSwitchPPPoESessionParseEnable:
            BCM_PBMP_ITER(PBMP_ALL(unit) ,port) {
                /* Get the first valid port id */
                break;
            }
            return bcm_robo_switch_control_port_get(unit, port, 
                type, arg);
            break;
        case bcmSwitchPPPoESessionEthertype:
            rv = (DRV_DEV_PROP_GET(unit, 
                DRV_DEV_PROP_PPPOE_SESSION_ETYPE,&value));
            if (BCM_SUCCESS(rv)) {
                *arg = value;
            }
            return rv;
            break;
        default:
            break;
    }

    /* GNATS 40759 : redesign IGMP/MLD trap/snoop */
    if (_BCM_SWITCH_IGMP_MLD_TYPES(type)){

        rv = _bcm_robo_igmp_mld_control_get(unit, type, arg);
        return rv;
    }

    /* get non-igmp/non-mld trap configuring status */
    BCM_IF_ERROR_RETURN(DRV_TRAP_GET
        (unit, port, &type_mask));
                    
    /* bcm53115, bcm53118 new feature on supporting different CPU snooping/traping 
     * behavior per separated the IGMP/MLD packet types.
     *
     * The designed process for this new feature are :
     *  1. bcmSwitchxxxxToCPU :
     *      - Return TRUE either snoop or trap is set will reutrn true.
     *      - else return FALSE.
     *  2. bcmSwitchxxxxDrop :
     *      - Return TRUE only when the trap is set.
     *      - else return FALSE.
     */ 
    if (SOC_IS_ROBO_ARCH_VULCAN(unit)){
        
        /* get snoop configuring status */
        BCM_IF_ERROR_RETURN(DRV_SNOOP_GET
            (unit, &snoop_type_mask));
        
        *arg = FALSE;   /* assigned the default return value */
        switch(type) {
        /* traping and snooping of non-imgp/non-mld request */
        case bcmSwitchIcmpRedirectToCpu:
            if (type_mask & DRV_SWITCH_TRAP_ICMPV6){
                *arg = TRUE;
            }
            return  BCM_E_NONE;
            break;
        case bcmSwitchArpReplyToCpu:    /* ARP */
            if (snoop_type_mask & DRV_SNOOP_ARP){
                *arg = TRUE;
            }
            return  BCM_E_NONE;
            break;
        case bcmSwitchDhcpPktToCpu:     /* DHCP */
            if (snoop_type_mask & DRV_SNOOP_DHCP){
                *arg = TRUE;
            }
            return  BCM_E_NONE;
            break;
        default :
            break;
        }   /* switch case */
                
    }   /* SOC_IS_VULCAN(unit), SOC_IS_BLACKBIRD(unit) */
                    
    switch(type) {
    case bcmSwitchBpduToCPU:        
        *arg = (type_mask & DRV_SWITCH_TRAP_BPDU1) ? TRUE : FALSE;
        break;
    case bcmSwitch8021xToCPU:
        *arg = (type_mask & DRV_SWITCH_TRAP_8021X) ? TRUE : FALSE;
        break;
    case bcmSwitchBcstToCPU:
        *arg = (type_mask & DRV_SWITCH_TRAP_BCST) ? TRUE : FALSE;
        break;
    default:
        return BCM_E_UNAVAIL;
        break;
    }
        
    return BCM_E_NONE;
}               

/*
 * Function:
 *  bcm_robo_switch_control_port_set
 * Description:
 *  Specify general switch behaviors on a per-port basis.
 * Parameters:
 *  unit - Device unit number
 *  port - Port to affect
 *  type - The desired configuration parameter to modify
 *  arg - The value with which to set the parameter
 * Returns:
 *  BCM_E_xxx
 */

int
bcm_robo_switch_control_port_set(int unit,
                                bcm_port_t port,
                                bcm_switch_control_t type,
                                int arg)
{
    uint8   tmp_state = 0;
    int rv = BCM_E_NONE;
    uint32  type_mask = 0, temp = 0;
    soc_pbmp_t bmp;
        
    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(
            _bcm_robo_switch_control_gport_resolve(unit, port, &port));
    }
    BCM_PBMP_CLEAR(bmp);

    /* for port based LED configurations */
    if (type == bcmSwitchLedFuncGrpSelect) {
        rv = DRV_LED_FUNCGRP_SELECT_SET(unit, port, arg);
        return rv;
    } else if (type == bcmSwitchLedMode) {
        temp = (uint32)arg;
        rv = DRV_LED_MODE_SET(unit, port, temp);
        return rv;
    }

    /* for misc control port based set */
    if (type == bcmSwitchPortBasedQos){
        BCM_PBMP_PORT_ADD(bmp, port);
        tmp_state = (arg) ? TRUE : FALSE;
        BCM_IF_ERROR_RETURN(DRV_QUEUE_MAPPING_TYPE_SET(
                unit, bmp, DRV_QUEUE_MAP_PORT, tmp_state));
        return BCM_E_NONE;
    }

    /* Enable PPPoE session packet parser */
    if (type == bcmSwitchPPPoESessionParseEnable) {
        BCM_PBMP_PORT_ADD(bmp, port);
        temp = (arg) ? TRUE : FALSE;
        rv = DRV_PORT_SET(unit, bmp, 
            DRV_PORT_PROP_PPPOE_PARSE_EN, temp);
        return rv;
    }
            
    switch(type) {
        case bcmSwitchDosAttackToCpu:
        case bcmSwitchDosAttackSipEqualDip:
        case bcmSwitchDosAttackMinTcpHdrSize:
        case bcmSwitchDosAttackV4FirstFrag:
        case bcmSwitchDosAttackTcpFlags:
        case bcmSwitchDosAttackTcpFlagsSF:
        case bcmSwitchDosAttackTcpFlagsFUP:
        case bcmSwitchDosAttackSynFrag:
        case bcmSwitchDosAttackFlagZeroSeqZero:
        case bcmSwitchDosAttackL4Port:
        case bcmSwitchDosAttackTcpFrag:
        case bcmSwitchDosAttackPingFlood:
        case bcmSwitchDosAttackSynFlood:
        case bcmSwitchDosAttackTcpSmurf:
            /* 
             * A primary check point for DoS Attack?
             */
            if (soc_feature(unit, soc_feature_field)) {
                rv = _bcm_robo_dos_attack_set
                    (unit, port, type, arg);
            } else {
                rv = BCM_E_UNAVAIL;
            }
            break;
        case bcmSwitch8021xToCPU:
            if (SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit)) {
                /* get per-port trap configuring status */
                BCM_IF_ERROR_RETURN(DRV_TRAP_GET
                    (unit, port, &type_mask));
               if (type_mask & DRV_SWITCH_TRAP_8021X){
                    if (arg){
                        return  BCM_E_NONE;
                    } else {
                        type_mask &= ~DRV_SWITCH_TRAP_8021X;
                    }
                } else {
                    if (arg){
                        type_mask |= DRV_SWITCH_TRAP_8021X;
                    } else {
                        return  BCM_E_NONE;
                    }
                }
                BCM_PBMP_PORT_ADD(bmp, port);
                BCM_IF_ERROR_RETURN(DRV_TRAP_SET
                    (unit, bmp, type_mask));
            } else {
                return BCM_E_UNAVAIL;
            }
            break;
        case bcmSwitchIngressRateLimitIncludeIFG:
            BCM_PBMP_PORT_ADD(bmp, port);
            if (arg) {
                temp = 1;
            } else {
                temp = 0;
            }
            BCM_IF_ERROR_RETURN(
                DRV_RATE_CONFIG_SET(unit, bmp, 
                        DRV_RATE_CONFIG_INGRESS_IPG_INCLUDE, temp));
            break;
        case bcmSwitchSharedVlanEnable:
            /* for port based Shared VLAN */
            BCM_PBMP_PORT_ADD(bmp, port);
            temp  = (arg) ? TRUE : FALSE;
            BCM_IF_ERROR_RETURN(DRV_VLAN_PROP_PORT_ENABLE_SET
                    (unit, DRV_VLAN_PROP_VLAN_LEARNING_MODE, 
                    bmp, temp));
            break;
        default:
            return BCM_E_UNAVAIL;
    }

    return rv;
}

/*
 * Function:
 *  bcm_robo_switch_control_port_get
 * Description:
 *  Retrieve general switch behaviors on a per-port basis
 * Parameters:
 *  unit - Device unit number
 *  port - Port to check
 *  type - The desired configuration parameter to retrieve
 *  arg - Pointer to where the retrieved value will be written
 * Returns:
 *  BCM_E_xxx
 */

int
bcm_robo_switch_control_port_get(int unit,
                                bcm_port_t port,
                                bcm_switch_control_t type,
                                int *arg)
{
    int rv = BCM_E_NONE;
    uint8   tmp_state = 0;
    uint32  type_mask = 0, temp = 0;
        
    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(
            _bcm_robo_switch_control_gport_resolve(unit, port, &port));
    }

    /* for port based LED configurations */
    if (type == bcmSwitchLedFuncGrpSelect) {
        rv = DRV_LED_FUNCGRP_SELECT_GET(unit, port, arg);
        return rv;
    } else if (type == bcmSwitchLedMode) {
        rv = DRV_LED_MODE_GET(unit, port, &temp);
        *arg = (int)temp;
        return rv;
    }

    /* for misc control port based get */
    if (type == bcmSwitchPortBasedQos){
        BCM_IF_ERROR_RETURN(DRV_QUEUE_MAPPING_TYPE_GET(
                unit, port, DRV_QUEUE_MAP_PORT, &tmp_state));
        *arg = tmp_state;
        return BCM_E_NONE;
    }

    /* Enable PPPoE session packet parser */
    if (type == bcmSwitchPPPoESessionParseEnable) {
        rv = DRV_PORT_GET(unit, port, 
            DRV_PORT_PROP_PPPOE_PARSE_EN, &temp);
        *arg = temp;
        return rv;
    }

    switch(type) {
        case bcmSwitchDosAttackToCpu:
        case bcmSwitchDosAttackSipEqualDip:
        case bcmSwitchDosAttackMinTcpHdrSize:
        case bcmSwitchDosAttackV4FirstFrag:
        case bcmSwitchDosAttackTcpFlags:
        case bcmSwitchDosAttackTcpFlagsSF:
        case bcmSwitchDosAttackTcpFlagsFUP:
        case bcmSwitchDosAttackSynFrag:
        case bcmSwitchDosAttackFlagZeroSeqZero:
        case bcmSwitchDosAttackL4Port:
        case bcmSwitchDosAttackTcpFrag:
        case bcmSwitchDosAttackPingFlood:
        case bcmSwitchDosAttackSynFlood:
        case bcmSwitchDosAttackTcpSmurf:
            if (soc_feature(unit, soc_feature_field)) {
                rv = _bcm_robo_dos_attack_get
                    (unit, port, type, arg);
            } else {
                rv = BCM_E_UNAVAIL;
            }
            break;
        case bcmSwitch8021xToCPU:
            if (SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit)) {
                BCM_IF_ERROR_RETURN(DRV_TRAP_GET
                    (unit, port, &type_mask));
                *arg = (type_mask & DRV_SWITCH_TRAP_8021X) ? TRUE : FALSE;
            } else {
                return BCM_E_UNAVAIL;
            }
            break;
        case bcmSwitchIngressRateLimitIncludeIFG:
            BCM_IF_ERROR_RETURN(
                DRV_RATE_CONFIG_GET(unit, port, 
                        DRV_RATE_CONFIG_INGRESS_IPG_INCLUDE, &temp));
            *arg = (temp) ? TRUE : FALSE;
            break;
        case bcmSwitchSharedVlanEnable:
            /* for port based Shared VLAN */
            BCM_IF_ERROR_RETURN(DRV_VLAN_PROP_PORT_ENABLE_GET
                    (unit, DRV_VLAN_PROP_VLAN_LEARNING_MODE, 
                    port, &temp));
            *arg = (temp) ? TRUE : FALSE;
            break;
        default:
            return BCM_E_UNAVAIL;
    }


    return rv;
}

/*
 * Function:
 *  _bcm_switch_init
 * Description:
 *  Initialize switch controls.
 * Parameters:
 *  unit        - Device unit number
 * Returns:
 *      BCM_E_xxx
 */
int 
_bcm_robo_switch_init(int unit)
{
    int i;

    /* Initialize sw copy of dos attack Field configuration */
    for (i = 0; i < BCM_MAX_NUM_UNITS; i++) {
        dos_attack_group_id[i] = 0;
        dos_attack_tcp_group_id[i] = 0;
    }

    sal_memset(&dos_attack_data, 0, \
        sizeof(dos_attack_t)*BCM_MAX_NUM_UNITS*SOC_MAX_NUM_PORTS);

    return BCM_E_NONE;
}

/*
 * Function:
 *	bcm_robo_switch_event_register
 * Description:
 *	Registers a call back function for switch critical events
 * Parameters:
 *	unit        - Device unit number
 *  cb          - The desired call back function to register
 *  userdata    - Pointer to any user data to carry on.
 * Returns:
 *      BCM_E_xxx
 * Notes:
 *      
 *	Several call back functions could be registered, they all will be called 
 *	upon critical event. If registered callback is called it is adviced to log 
 *  the information and reset the chip. 
 *  Same call back function with different userdata is allowed to be 
 *  registered. 
 */
int 
bcm_robo_switch_event_register(int unit, bcm_switch_event_cb_t cb, 
                              void *userdata)
{
    return soc_event_register(unit, (soc_event_cb_t)cb, userdata);
}


/*
 * Function:
 *	bcm_robo_switch_event_unregister
 * Description:
 *	Unregisters a call back function for switch critical events
 * Parameters:
 *	unit        - Device unit number
 *  cb          - The desired call back function to unregister.
 *  userdata    - Pointer to user data associated with a call back function
 * Returns:
 *      BCM_E_xxx
 * Notes:
 *      
 *  If userdata = NULL then all matched call back functions will be 
 *  unregistered
 */
int 
bcm_robo_switch_event_unregister(int unit, bcm_switch_event_cb_t cb, 
                                void *userdata)
{
    return soc_event_unregister(unit, (soc_event_cb_t)cb, userdata);
}

