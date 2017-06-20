/* $Id: sw_db.h,v 1.15 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * $
 */
#ifndef _BCM_DNX_SW_DB_H_
#define _BCM_DNX_SW_DB_H_

#include <bcm/switch.h>
#include <bcm/module.h>


typedef struct dos_attack_info_s {
  uint8 tocpu;
  /* flags for all dos attack traps */
  uint8 sipequaldip;
  uint8 mintcphdrsize;
  uint8 v4firstfrag;
  uint8 tcpflags;
  uint8 l4port;
  uint8 tcpfrag;
  uint8 icmp;
  uint8 icmppktoversize;
  uint8 macsaequalmacda;
  uint8 icmpv6pingsize;
  uint8 icmpfragments;
  uint8 tcpoffset;
  uint8 udnxortsequal;
  uint8 tcpportsequal;
  uint8 tcpflagssf;
  uint8 tcpflagsfup;
  uint8 tcphdrpartial;
  uint8 pingflood;
  uint8 synflood;
  uint8 tcpsmurf;
  uint8 tcpxmas;
  uint8 l3header;
} dos_attack_info_t;

typedef struct dnx_switch_sw_db_s {
  dos_attack_info_t dos_attack; 
} bcm_dnx_switch_sw_db_t;

extern int _bcm_dnx_init_finished_ok[BCM_MAX_NUM_UNITS];

int 
_bcm_sw_db_switch_urpf_mode_set(int unit, int urpf_mode);

int 
_bcm_sw_db_switch_urpf_mode_get(int unit, int *urpf_mode);

int 
_bcm_sw_db_switch_dos_attack_info_set(int unit, bcm_switch_control_t bcm_type, int enable);

int 
_bcm_sw_db_switch_dos_attack_info_get(int unit, bcm_switch_control_t bcm_type, int *enable);


int 
_bcm_sw_db_init(int unit);

int 
_bcm_sw_db_deinit(int unit);

int
_bcm_sw_db_cell_id_curr_get(int unit, int *cell_id);

int
_bcm_sw_db_cell_id_curr_set(int unit, int *cell_id);

#endif
