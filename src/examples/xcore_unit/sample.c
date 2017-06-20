/*
 * $Id: sample.c,v 1.2 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

int rc;
int i;
int unit = 1;
int vftbase = 0;
int femod = 0;
int native_vid = 17;
int pci = 31;
int pcisid = 42;

rc = soc_sbx_g2p3_vlan_ft_base_get(unit, &vftbase);

rc = soc_sbx_g2p3_age_set(unit, 0);

rc = soc_sbx_g2p3_node_set(unit, femod);

soc_sbx_g2p3_p2e_t p2e;
soc_sbx_g2p3_p2e_t_init(p2e);

p2e.nativevid = native_vid;
p2e.provider = 1;
rc = soc_sbx_g2p3_p2e_set(unit, pci, p2e);

/* clear QOS mapping 0 (default) */
soc_sbx_g2p3_qos_t qos;
soc_sbx_g2p3_qos_t_init(qos);

for (i=0; i < 16; i++) {
  rc = soc_sbx_g2p3_qos_set(unit, (i % 2), (i / 2), 0, qos);
}

rc = soc_sbx_g2p3_mc_ft_offset_set(unit, 0);
   
soc_sbx_g2p3_lp_t lp;
soc_sbx_g2p3_lp_t_init(lp);

lp.pid = pcisid;
rc = soc_sbx_g2p3_lp_set(unit, pci, lp);

print rc;

cint_reset();


/* the end */

/* 
# Untagged packet, miss
proc AT_g2p3i_001 {chip unit args} {
    set p [i_bridge_pkt]
    set ip $p
    set ep [i_erh_pkt $p $::native_vlan $::native_qid 1 ]
    return [g2p3_unit_txrx $ip $ep 0 "untagged miss"]
}

proc g2p3_unit_txrx {txp rxp {egress 0} {rxpn "g2p3 unit test packet"} } {
    set rxunits [list $::fe]
    if {$egress == 2} {
        bcm shell "1:nicconfig free"
        lappend rxunits ethernet
    } elseif {$egress == 1} {
        bcm shell "1:nicconfig egress"
        lappend rxunits ethernet
    } else {
        bcm shell "1:nicconfig ingress"
        if {$::sirius} {
            lappend rxunits gu2irh_sirius
        } elseif {$::qess} {
            lappend rxunits gu2erh_qess
        } else {
            lappend rxunits sbx
        }
    }
    set txs [list $::fe $txp]
    set rxs {}
    if {[llength $rxp] > 0} {
        lappend rxs $::fe $rxpn $rxp 0
    }
    if {![sbx_pkt_test $txs $rxs $rxunits]} {
        return FAIL
    }
    return PASS
}

*/
