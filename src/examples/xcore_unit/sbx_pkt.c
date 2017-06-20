/*
 * $Id: sbx_pkt.c,v 1.2 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

rc = bcm_pkt_alloc (unit, length, flags, pkt);

bcm_pkt_memcpy (pkt, 0, pktp);

bcm_tx (unit, pkt, NULL);

gu2_sbx_pkt_test_find_rx

bcm_pkt_free


# Send and receive test packets through PCI interfaces
#
# tx_pkts is a sequence of pairs:
#     - unit on which to send
#     - packet to send
# rx_pkts is a sequence of triples:
#     - unit on which to receive
#     - name of packet expected
#     - packet to expect
#     - exception code to expect (or 0 == no exception)
# rx_units is a sequence of pairs:
#     - unit from which to receive
#     - mode: ethernet, sbx (SBX ERH-encapsulated), higig (HiGig-encapsulated)
#             sbx_higig (ERH and HiGig encapsulated)
proc sbx_pkt_test {tx_pkts rx_pkts rx_units} {
    global _pkt_test_wake
    set _pkt_test_wake 0
    set pass 1
    set txBuffs {}

    foreach {txu txp} $tx_pkts {
        set txpb [pkt_bytes $txp]
        set pkt_flags $::BCM_TX_CRC_ALLOC
        set rv [bcm_pkt_alloc $txu [llength $txpb] $pkt_flags pkt]
        if {$::BCM_E_NONE != $rv} {
          puts "ERROR: $rv unable to allocate TX buffer for $txp"
          after $::sbx_pkt_rx_wait incr _pkt_test_wake
          vwait _pkt_test_wake
          set rxed [gu2_sbx_pkt_test_rx $rx_units]
          return 0
        }
        bcm_pkt_memcpy $pkt 0 $txpb

        if {$::sbx_pkt_test_verbose} {
            puts "txing to unit $txu:"
            pkt_dump $txp
        }

        bcm_tx $txu $pkt NULL
        lappend txBuffs $txu $pkt
    }

    after $::sbx_pkt_rx_wait incr _pkt_test_wake
    vwait _pkt_test_wake
    set rxed [gu2_sbx_pkt_test_rx $rx_units]

    foreach {txu pkt} $txBuffs {
      bcm_pkt_free $txu $pkt
    }

    foreach {rxu rxn rxp rxe} $rx_pkts {
        set rxpb [pkt_bytes $rxp]
        if {$::sbx_pkt_test_verbose} {
            puts "rxing '$rxn' (on unit $rxu):"
            pkt_dump $rxp
        }

        if {![gu2_sbx_pkt_test_find_rx rxed $rxu $rxpb $rxe]} {
            puts -nonewline "ERROR: did not rx '$rxn' on unit $rxu"
            if {$rxe} {
                puts -nonewline " exception "
                puts -nonewline [format "0x%02X" $rxe]
            }
            puts " :::"
            pkt_dump $rxp
            set pass 0
        } else {
            if {$::sbx_pkt_test_verbose} {
                puts "got the expected packet"
            }
        }
    }

    if {[llength $rxed] != 0} {
        set upkts [expr [llength $rxed] / 4]
        puts "ERROR: $upkts unclaimed packets in test:"
        foreach {rxu rxpb rxe mode} $rxed {
            puts -nonewline "on unit $rxu"
            if {$rxe} {
                puts -nonewline " exception $rxe"
            }
            puts " $mode mode ([llength $rxpb] bytes) :::"
            set ::sbx_pkt_unclaimed $rxpb
            switch $mode {
                ethernet { pkt_dump [pkt_enet_parse $rxpb] }
                sbx      { pkt_dump [pkt_gu2erh_parse $rxpb] }
                higig    { pkt_dump [pkt_higig_parse $rxpb] }
                sbx_higig { pkt_dump [pkt_sbx_higig_parse $rxpb] }
                gu2irh_sirius { pkt_dump [pkt_gu2irh_sirius_parse $rxpb] }
                gu2erh_sirius { pkt_dump [pkt_gu2erh_sirius_parse $rxpb] }
                gu2erh_qess   { pkt_dump [pkt_gu2erh_qess_parse $rxpb] }
                default  { error "unexpected packet mode: $mode" }
            }
        }
        set pass 0
    }

    return $pass
}


proc gu2_sbx_pkt_test_rx {units} {
    global BCM_E_TIMEOUT
    set rxed {}

    foreach {u m} $units {
        while {1} {
            # ase_rx_sync's pkt param changed from IN/OUT to OUT meaning the
            # pkt var must not be created before being passed by name
            #set _sbx_pkt_test_rx_pkt [bcm_pkt_t]
            #bcm_pkt_t_init _sbx_pkt_test_rx_pkt
            set rv [ase_rx_sync $u _sbx_pkt_test_rx_pkt 0]
            if {$rv < 0} {
                #if running C2-Sirius skip error
                if { (!$::sirius) && ($rv != $BCM_E_TIMEOUT) } {
                    puts "sync rx failed: $rv [bcm_errmsg $rv] UNIT $u"
                }
                ase_rx_pkt_free $_sbx_pkt_test_rx_pkt
                break
            }

            set len [$_sbx_pkt_test_rx_pkt cget -pkt_len]
            ase_pkt_data_memget $_sbx_pkt_test_rx_pkt 0 pkt_data $len
            set p {}
            set rhlen [$_sbx_pkt_test_rx_pkt cget -_sbx_hdr_len]
            set rh [$_sbx_pkt_test_rx_pkt cget -_sbx_rh]
            for {set i 0} {$i < $rhlen} {incr i} {
                lappend p [format "0x%02X" [lindex $rh $i]]
            }
            foreach o $pkt_data {
                lappend p [format "0x%02X" $o]
            }
            set exception [$_sbx_pkt_test_rx_pkt cget -rx_reason]

            if {![string match $m sbx] && \
                ![string match $m sbx_higig] && \
                [expr $rhlen > 0]} {
                set m sbx
            }
            if {[string match $m gu2irh_sirius] } {
                set m gu2irh_sirius
            }
            if {[string match $m gu2erh_sirius] } {
                set m gu2erh_sirius
            }
            if {[string match $m gu2erh_qess] } {
                set m gu2erh_qess
            }
            lappend rxed $u $p [format "0x%02X" [expr $exception & 0xff]] $m

            ase_rx_pkt_free $_sbx_pkt_test_rx_pkt
        }
    }

    return $rxed
}
