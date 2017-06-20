/*
 * $Id: stats.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:    stats.c
 *
 * Purpose: 
 *
 * Functions:
 *      bcm_common_ptp_packet_counters_get
 *      bcm_common_ptp_peer_dataset_get
 *
 *      _bcm_ptp_packet_counters_msg_get
 *      _bcm_ptp_packet_counters_pci_get
 *      _bcm_ptp_update_peer_counts
 *      _bcm_ptp_update_peer_dataset
 *      _bcm_ptp_peer_dataset_get
 *      _bcm_ptp_clock_description_get
 *      _bcm_ptp_show_system_info
 */

#if defined(INCLUDE_PTP)

#include <bcm/ptp.h>
#include <bcm_int/common/ptp.h>
#include <bcm_int/ptp_common.h>
#include <bcm/error.h>

/* Constants and variables. */
static const bcm_ptp_packet_counters_t counters_zero;


static const bcm_ptp_port_identity_t portid_all = 
    {{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff}, PTP_IEEE1588_ALL_PORTS};

/* Static functions. */
static int _bcm_ptp_packet_counters_msg_get(int unit, bcm_ptp_stack_id_t ptp_id,
    int clock_num, bcm_ptp_packet_counters_t *counters);
    

/*
 * Function:
 *      bcm_common_ptp_packet_counters_get
 * Purpose:
 *      Get packet counters.
 * Parameters:
 *      unit      - (IN)  Unit number.
 *      ptp_id    - (IN)  PTP stack ID.
 *      clock_num - (IN)  PTP clock number.
 *      counters  - (OUT) Packet counts/statistics.
 * Returns:
 *      BCM_E_XXX - Function status.
 * Notes:
 */
int 
bcm_common_ptp_packet_counters_get(
    int unit, 
    bcm_ptp_stack_id_t ptp_id,
    int clock_num, 
    bcm_ptp_packet_counters_t *counters)
{   
    return _bcm_ptp_packet_counters_msg_get(unit, ptp_id, clock_num, counters);
}

/*
 * Function:
 *      _bcm_ptp_packet_counters_msg_get
 * Purpose:
 *      Get packet counters via custom PTP managment message.
 * Parameters:
 *      unit      - (IN)  Unit number.
 *      ptp_id    - (IN)  PTP stack ID.
 *      clock_num - (IN)  PTP clock number.
 *      counters  - (OUT) Packet counters.
 * Returns:
 *      BCM_E_XXX - Function status.
 * Notes:
 */
static int 
_bcm_ptp_packet_counters_msg_get(
    int unit, 
    bcm_ptp_stack_id_t ptp_id,
    int clock_num, 
    bcm_ptp_packet_counters_t *counters)
{   
    int rv = BCM_E_UNAVAIL;
    
    uint8 resp[PTP_MGMTMSG_RESP_MAX_SIZE_OCTETS];
    int resp_len = PTP_MGMTMSG_RESP_MAX_SIZE_OCTETS; 
    int i = 0;

    bcm_ptp_port_identity_t portid;  
    
    int port;
    uint16 num_ports;
    
    *counters = counters_zero;
    
    if (BCM_FAILURE(rv = _bcm_ptp_function_precheck(unit, ptp_id, clock_num, 
            PTP_CLOCK_PORT_NUMBER_DEFAULT))) {
        return rv;   
    }
        
    if (BCM_FAILURE(rv = bcm_common_ptp_clock_port_identity_get(unit, ptp_id, 
            clock_num, PTP_IEEE1588_ALL_PORTS, &portid))) {
        return rv;
    }
    
    if (BCM_FAILURE(rv = _bcm_ptp_management_message_send(unit, ptp_id, 
            clock_num, &portid, 
            PTP_MGMTMSG_GET, PTP_MGMTMSG_ID_PACKET_STATS, 
            0, 0, resp, &resp_len))) {
        return rv;
    }
    
    if (resp_len < PTP_MGMTMSG_PAYLOAD_PACKET_STATS_MIN_SIZE_OCTETS) {
        /* Internal error : message too short to contain correct data. */
        return BCM_E_INTERNAL;
    }
    
    /*
     * Parse response.
     *    Octet 0...5      : Custom management message key/identifier. 
     *                       BCM<null><null><null>.
     *    Octet 6...7      : Number of ports.    
     *    Octet 8...11     : Packets transmitted.
     *    Octet 12...15    : Packets received.
     *    Octet 16...19    : Packets discarded.
     *    Octet 20...23    : RCPU encapsulated packets received.
     *    Octet 24...27    : IPv4 packets received.
     *    Octet 28...31    : IPv6 packets received.
     *    Octet 32...35    : L2 PTP packets received.
     *    Octet 36...39    : UDP PTP packets received.
     *    Octet 40...43    : Enduro sync packets transmitted.
     *    Octet 44...47    : Enduro sync packets received.
     *    Octet 48...51    : Rx queue overflows.
     *    Per Port: # = Port Number, I = 52 + 12(#-1) 
     *    Octet I...I+3    : PTP port # packets transmitted.
     *    Octet I+4...I+7  : PTP port # packets received.
     *    Octet I+8...I+11 : PTP port # packets discarded.
     */
    i = 6;    
    num_ports = _bcm_ptp_uint16_read(resp+i);
    i += sizeof(uint16); 
    counters->packets_transmitted = _bcm_ptp_uint32_read(resp+i);
    i += sizeof(uint32);
    counters->packets_received = _bcm_ptp_uint32_read(resp+i);
    i += sizeof(uint32);
    counters->packets_discarded = _bcm_ptp_uint32_read(resp+i);
    i += sizeof(uint32);
    counters->rcpu_encap_packets_received = _bcm_ptp_uint32_read(resp+i);
    i += sizeof(uint32);
    counters->ipv4_packets_received = _bcm_ptp_uint32_read(resp+i);
    i += sizeof(uint32);
    counters->ipv6_packets_received = _bcm_ptp_uint32_read(resp+i);
    i += sizeof(uint32);
    counters->l2_ptp_packets_received = _bcm_ptp_uint32_read(resp+i);
    i += sizeof(uint32);
    counters->udp_ptp_packets_received = _bcm_ptp_uint32_read(resp+i);
    i += sizeof(uint32);
    counters->enduro_sync_packets_transmitted = _bcm_ptp_uint32_read(resp+i);
    i += sizeof(uint32);
    counters->enduro_sync_packets_received = _bcm_ptp_uint32_read(resp+i);
    i += sizeof(uint32);
    counters->rx_queue_overflows = _bcm_ptp_uint32_read(resp+i);
    i += sizeof(uint32);
    
    for (port = 0; port < num_ports; ++port) {
        counters->port_packets_transmitted[port] = _bcm_ptp_uint32_read(resp+i);
        i += sizeof(uint32);
        counters->port_packets_received[port] = _bcm_ptp_uint32_read(resp+i);
        i += sizeof(uint32);
        counters->port_packets_discarded[port] = _bcm_ptp_uint32_read(resp+i);
        i += sizeof(uint32);
    }
        
    return rv;
}


/*
 * Function:
 *      _bcm_ptp_update_peer_counts
 * Purpose:
 *      Update peer message counts.
 * Parameters: 
 *      unit         - (IN) Unit number.
 *      ptp_id       - (IN) PTP stack id
 *      clock_num    - (IN) PTP clock index
 *      localPortNum - (IN) Local PTP clock port number.
 *      clockID      - (IN) Peer PTP clock identity.
 *      port_addr    - (IN) Peer PTP port address.
 *      isMaster     - (IN) Peer-is-master Boolean.
 *      announces    - (IN) Number of addt'l announce messages.
 *      syncs        - (IN) Number of addt'l sync messages.
 *      delayreqs    - (IN) Number of addt'l delay request messages.
 *      delayresps   - (IN) Number of addt'l dealy response messages.
 *      mgmts        - (IN) Number of addt'l management messages.
 *      signals      - (IN) Number of addt'l signaling messages.
 *      rejected     - (IN) Number of addt'l rejected messages.
 * Returns:
 *      BCM_E_XXX - Function status.
 * Notes:
 */
int
_bcm_ptp_update_peer_counts(
    int unit,
    int ptp_id,
    int clock_num,
    uint16 localPortNum,
    bcm_ptp_clock_identity_t *clockID,
    bcm_ptp_clock_port_address_t *port_addr,
    int isMaster,
    unsigned announces,
    unsigned syncs,
    unsigned followups,
    unsigned delayreqs,
    unsigned delayresps,
    unsigned mgmts,
    unsigned signals,
    unsigned rejected,
    bcm_gport_t *phy_port)
{
    int         rv = BCM_E_NONE;
    unsigned    index;
    _bcm_ptp_info_t *ptp_info_p;
    int max_peer_dataset_entries;
    bcm_ptp_peer_dataset_t *peerTable;

    if (BCM_FAILURE(rv = _bcm_ptp_function_precheck(unit, ptp_id, clock_num, PTP_CLOCK_PORT_NUMBER_DEFAULT))) {
        return rv;
    }

    peerTable = &_bcm_common_ptp_unit_array[unit].stack_array[ptp_id].clock_array[clock_num].peerTable;

    sal_mutex_take(peerTable->lock, sal_mutex_FOREVER);

    /* Look in the table for a match */
    for (index = 0; index < peerTable->num_peers; index++) {
        if ((localPortNum == 0xffff) ||
            (peerTable->peer[index].local_port_number == 0xffff) ||
            (localPortNum == peerTable->peer[index].local_port_number)) {
            if (!_bcm_ptp_port_address_cmp(port_addr, &peerTable->peer[index].port_address)) {
                /* We have a match, so inc the counts as listed a return */
                if (isMaster) {
                    COMPILER_64_ADD_32(peerTable->peer[index].rx_announces, announces);
                    COMPILER_64_ADD_32(peerTable->peer[index].rx_syncs, syncs);
                    COMPILER_64_ADD_32(peerTable->peer[index].rx_followups, followups);
                    COMPILER_64_ADD_32(peerTable->peer[index].tx_delayreqs, delayreqs);
                    COMPILER_64_ADD_32(peerTable->peer[index].rx_delayresps, delayresps);
                    COMPILER_64_ADD_32(peerTable->peer[index].tx_mgmts, mgmts);
                    COMPILER_64_ADD_32(peerTable->peer[index].tx_signals, signals);
                } else {
                    COMPILER_64_ADD_32(peerTable->peer[index].tx_announces, announces);
                    COMPILER_64_ADD_32(peerTable->peer[index].tx_syncs, syncs);
                    COMPILER_64_ADD_32(peerTable->peer[index].tx_followups, followups);
                    COMPILER_64_ADD_32(peerTable->peer[index].rx_delayreqs, delayreqs);
                    COMPILER_64_ADD_32(peerTable->peer[index].tx_delayresps, delayresps);
                    COMPILER_64_ADD_32(peerTable->peer[index].rx_mgmts, mgmts);
                    COMPILER_64_ADD_32(peerTable->peer[index].rx_signals, signals);
                }

                if (peerTable->peer[index].local_port_number == 0xffff) {
                    peerTable->peer[index].local_port_number = localPortNum;
                }

                COMPILER_64_ADD_32(peerTable->peer[index].rejected, rejected);
                if (clockID) {
                    memcpy(peerTable->peer[index].clock_identity, clockID, BCM_PTP_CLOCK_EUID_IEEE1588_SIZE);
                }
                if (SOC_HAS_PTP_INTERNAL_STACK_SUPPORT(unit)) {
                    if (phy_port) {
                        peerTable->peer[index].phy_port = *phy_port;
                    }
                }
                sal_mutex_give(peerTable->lock);
                return rv;
            }
        }
    }

    SET_PTP_INFO;

    max_peer_dataset_entries = ptp_info_p->stack_info->unicast_slave_table_size +  
                               PTP_MAX_CLOCK_INSTANCE_PORTS + _PTP_MAX_FOREIGN_MASTER_DATASET_ENTRIES;

    /* Make sure we have room */
    if (peerTable->num_peers >= max_peer_dataset_entries) {
        sal_mutex_give(peerTable->lock);
        LOG_VERBOSE(BSL_LS_BCM_PTP,
                    (BSL_META_U(unit,
                                "PTP Peer Statistics full, dropping count\n")));
        return BCM_E_NONE;
    }

    /* Add an entry */
    index = peerTable->num_peers;

    peerTable->peer[index].local_port_number = localPortNum;
    if (clockID) {
        memcpy(peerTable->peer[index].clock_identity, clockID, BCM_PTP_CLOCK_EUID_IEEE1588_SIZE);
    } else {
        memset(peerTable->peer[index].clock_identity, 0, BCM_PTP_CLOCK_EUID_IEEE1588_SIZE);
    }
    peerTable->peer[index].port_address = *port_addr;

    if (isMaster) {
        COMPILER_64_SET(peerTable->peer[index].rx_announces, 0, announces);
        COMPILER_64_SET(peerTable->peer[index].rx_syncs, 0, syncs);
        COMPILER_64_SET(peerTable->peer[index].rx_followups, 0, followups);
        COMPILER_64_SET(peerTable->peer[index].tx_delayreqs, 0, delayreqs);
        COMPILER_64_SET(peerTable->peer[index].rx_delayresps, 0, delayresps);
        COMPILER_64_SET(peerTable->peer[index].tx_mgmts, 0, mgmts);
        COMPILER_64_SET(peerTable->peer[index].tx_signals, 0, signals);
        COMPILER_64_SET(peerTable->peer[index].tx_announces, 0, 0);
        COMPILER_64_SET(peerTable->peer[index].tx_syncs, 0, 0);
        COMPILER_64_SET(peerTable->peer[index].tx_followups, 0, 0);
        COMPILER_64_SET(peerTable->peer[index].rx_delayreqs, 0, 0);
        COMPILER_64_SET(peerTable->peer[index].tx_delayresps, 0, 0);
        COMPILER_64_SET(peerTable->peer[index].rx_mgmts, 0, 0);
        COMPILER_64_SET(peerTable->peer[index].rx_signals, 0, 0);
    } else {
        COMPILER_64_SET(peerTable->peer[index].tx_announces, 0, announces);
        COMPILER_64_SET(peerTable->peer[index].tx_syncs, 0, syncs);
        COMPILER_64_SET(peerTable->peer[index].tx_followups, 0, followups);
        COMPILER_64_SET(peerTable->peer[index].rx_delayreqs, 0, delayreqs);
        COMPILER_64_SET(peerTable->peer[index].tx_delayresps, 0, delayresps);
        COMPILER_64_SET(peerTable->peer[index].rx_mgmts, 0, mgmts);
        COMPILER_64_SET(peerTable->peer[index].rx_signals, 0, signals);
        COMPILER_64_SET(peerTable->peer[index].rx_announces, 0, 0);
        COMPILER_64_SET(peerTable->peer[index].rx_syncs, 0, 0);
        COMPILER_64_SET(peerTable->peer[index].rx_followups, 0, 0);
        COMPILER_64_SET(peerTable->peer[index].tx_delayreqs, 0, 0);
        COMPILER_64_SET(peerTable->peer[index].rx_delayresps, 0, 0);
        COMPILER_64_SET(peerTable->peer[index].tx_mgmts, 0, 0);
        COMPILER_64_SET(peerTable->peer[index].tx_signals, 0, 0);
    }
    COMPILER_64_SET(peerTable->peer[index].rejected, 0, rejected);
    if (SOC_HAS_PTP_INTERNAL_STACK_SUPPORT(unit)) {
        if (phy_port) {
            peerTable->peer[index].phy_port = *phy_port;
        }
    }

    peerTable->num_peers++;

    sal_mutex_give(peerTable->lock);

    return rv;
}

/*
 * Function:
 *      _bcm_ptp_update_peer_dataset
 * Purpose:
 *      Update peer dataset.
 * Parameters:
 *      unit      - (IN) Unit number.
 *      ptp_id    - (IN) PTP stack ID.
 *      clock_num - (IN) PTP clock number.
 *      port_num  - (IN) PTP clock port number.
 * Returns:
 *      BCM_E_XXX - Function status.
 * Notes:
 */
int
_bcm_ptp_update_peer_dataset(
    int unit,
    bcm_ptp_stack_id_t ptp_id,
    int clock_num,
    int port_num)
{
    int rv = BCM_E_NONE;
    unsigned firstTime = 1;
    unsigned index;
    unsigned module_num = 0;
    unsigned phy_port_num = 0;
    bcm_gport_t phy_port;

    bcm_ptp_port_identity_t portid = portid_all;
    uint8 resp[PTP_MGMTMSG_RESP_MAX_SIZE_OCTETS];
    int resp_len = PTP_MGMTMSG_RESP_MAX_SIZE_OCTETS;
    int expected_per_peerdata_resp_len = 0;

    if (BCM_FAILURE(rv = _bcm_ptp_function_precheck(unit, ptp_id, clock_num, port_num))) {
        return rv;
    }

    if (BCM_FAILURE(rv = bcm_common_ptp_clock_port_identity_get(unit, ptp_id,
            clock_num, port_num, &portid))) {
        return rv;
    }
    
    while (1) {
        if (firstTime) {
            /* Send "first" management message. */
            if (BCM_FAILURE(rv = _bcm_ptp_management_message_send(unit, ptp_id,
                    clock_num, &portid, PTP_MGMTMSG_GET, PTP_MGMTMSG_ID_PEER_DATASET_FIRST,
                    0, 0, resp, &resp_len))) {
                return rv;
            }
            firstTime = 0;
        } else {
            if (BCM_FAILURE(rv = _bcm_ptp_management_message_send(unit, ptp_id,
                    clock_num, &portid, PTP_MGMTMSG_GET, PTP_MGMTMSG_ID_PEER_DATASET_NEXT,
                    0, 0, resp, &resp_len))) {
                return rv;
            }
        }

        /*
         * Parse response.
         *    Octet  0     number of peers in dump set (0 - _PTP_MAX_PEER_DATASET_CHUNK_SIZE)
         *    Octet  1     reserved
         *    Octet  2..3  PEER#0  Local port num
         *    Octet  4..11 PEER#0  Peer clock ID
         *    Octet 12     PEER#0  Peer is master
         *    Octet 13     PEER#0  Peer address type
         *    Octet 14..29 PEER#0  Peer network address
         *    Octet 30..31 PEER#0  Announces
         *    Octet 32..33 PEER#0  Syncs
         *    Octet 34..35 PEER#0  Followups
         *    Octet 36..37 PEER#0  delayreqs
         *    Octet 38..39 PEER#0  delayresps
         *    Octet 40..41 PEER#0  rejected count
         *    Octet 41..42 PEER#0  module number
         *    Octet 43..44 PEER#0  physical port number
         *    Octet 42..81 PEER#1 (as PEER#0)
         *    Octet 82....
         */

        if (resp_len < 1) {
            return BCM_E_INTERNAL;
        }

        if (resp[0] > _PTP_MAX_PEER_DATASET_CHUNK_SIZE) {
            return BCM_E_INTERNAL;
        }

        if (SOC_HAS_PTP_INTERNAL_STACK_SUPPORT(unit)) {
            expected_per_peerdata_resp_len = 44;
        } else {
            expected_per_peerdata_resp_len = 40;
        }

        if (resp_len != 2 + resp[0] * expected_per_peerdata_resp_len ) {
            return BCM_E_INTERNAL;
        }

        /* return when the ToP doesn't have any more elements to send us */
        if (resp[0] == 0) {
            return (rv);
        }

        /* Since we have elements update the tables */
        for (index = 0; index < resp[0]; index++) {
            bcm_ptp_clock_identity_t clockIDStorage, *clockID;
            bcm_ptp_clock_port_address_t port_addr;
            int addr_len;
            unsigned localPortNum, announces, syncs, followups, delayreqs, delayresps, mgmts, signals, rejected;
            int isMaster;

            unsigned offset = 2 + index * expected_per_peerdata_resp_len;

            localPortNum = _bcm_ptp_uint16_read(resp + offset);
            offset += 2;

            memcpy(clockIDStorage, resp + offset, BCM_PTP_CLOCK_EUID_IEEE1588_SIZE);
            offset += BCM_PTP_CLOCK_EUID_IEEE1588_SIZE;

            isMaster = resp[offset++];

            port_addr.addr_type = resp[offset++];
            addr_len = _bcm_ptp_addr_len(port_addr.addr_type);
            if (addr_len) {
                memcpy(port_addr.address, resp + offset, addr_len);
            } else {
                return BCM_E_INTERNAL;
            }
            offset += 16;

            announces = _bcm_ptp_uint16_read(resp + offset);
            offset += 2;

            syncs = _bcm_ptp_uint16_read(resp + offset);
            offset += 2;

            followups = _bcm_ptp_uint16_read(resp + offset);
            offset += 2;

            delayreqs = _bcm_ptp_uint16_read(resp + offset);
            offset += 2;

            delayresps = _bcm_ptp_uint16_read(resp + offset);
            offset += 2;

            mgmts = 0;
            signals = 0;

            rejected = _bcm_ptp_uint16_read(resp + offset);
            offset += 2;

            if (SOC_HAS_PTP_INTERNAL_STACK_SUPPORT(unit)) {
                module_num = _bcm_ptp_uint16_read(resp + offset);
                offset += 2;
                phy_port_num = _bcm_ptp_uint16_read(resp + offset);
                BCM_GPORT_MODPORT_SET(phy_port, module_num, phy_port_num);
            }

            if (_bcm_ptp_is_clockid_null(clockIDStorage)) {
                clockID = 0;
            } else {
                clockID = &clockIDStorage;
            }

            if (BCM_FAILURE(rv = _bcm_ptp_update_peer_counts(unit, ptp_id, clock_num, localPortNum, clockID,
                    &port_addr, isMaster, announces, syncs, followups, delayreqs,
                    delayresps, mgmts, signals, rejected, &phy_port))) {
                return rv;
            }
        }

        /*
         * Reset response message length.
         * _bcm_ptp_management_message_send() uses argument as maximum allowable 
         * message length and actual message lengths on function entry and exit,
         * respectively.
         */
        resp_len = PTP_MGMTMSG_RESP_MAX_SIZE_OCTETS;
    }

    return (rv);
}

/*
 * Function:
 *      _bcm_ptp_peer_dataset_get
 * Purpose:
 *      Get peer dataset(s).
 * Parameters:
 *      unit          - (IN)  Unit number.
 *      ptp_id        - (IN)  PTP stack ID.
 *      clock_num     - (IN)  PTP clock number.
 *      port_num      - (IN)  PTP clock port number.
 *      max_num_peers - (IN)  Maximum number of peer dataset entries.
 *      peers         - (OUT) Peers.
 *      num_peers     - (OUT) Number of peer dataset entries.
 * Returns:
 *      BCM_E_XXX - Function status.
 * Notes:
 *      All-ports identifier (i.e. all-ones port_num) returns all peers.
 */
int
_bcm_ptp_peer_dataset_get(
    int unit,
    bcm_ptp_stack_id_t ptp_id,
    int clock_num,
    int port_num,
    int max_num_peers,
    bcm_ptp_peer_entry_t *peers,
    int *num_peers)
{
    int rv;
    static const bcm_ptp_peer_entry_t zero_peer = {0};
    bcm_ptp_peer_dataset_t *peerTable;
    unsigned peer_index;
    unsigned temp_index;

    if (BCM_FAILURE(rv = _bcm_ptp_function_precheck(unit, ptp_id, clock_num, port_num))) {
        return rv;
    }

    if (BCM_FAILURE(rv = _bcm_ptp_update_peer_dataset(unit, ptp_id, clock_num, port_num))) {
        return rv;
    }

    peerTable = &_bcm_common_ptp_unit_array[unit].stack_array[ptp_id].clock_array[clock_num].peerTable;

    sal_mutex_take(peerTable->lock, sal_mutex_FOREVER);

    *num_peers = 0;
    for (peer_index = 0; peer_index < peerTable->num_peers; ) {
        /* Stop if caller-provided table is full. */
        if (*num_peers >= max_num_peers) {
            break;
        }

        /* Find peers associated with matching local port number. */
        if (port_num == peerTable->peer[peer_index].local_port_number ||
            port_num == PTP_IEEE1588_ALL_PORTS) {
            /* Get matching peer. */
            peers[(*num_peers)++] = peerTable->peer[peer_index];

            /* Move remaining peers up in list. */
            for (temp_index = peer_index + 1; temp_index < peerTable->num_peers; temp_index++) {
                peerTable->peer[temp_index - 1] = peerTable->peer[temp_index];
            }
            peerTable->peer[--(peerTable->num_peers)] = zero_peer;
        } else {
            peer_index++;
        }
    }

    sal_mutex_give(peerTable->lock);

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_ptp_clock_description_get
 * Purpose:
 *      Get PTP clock description.
 * Parameters:
 *      unit        - (IN)  Unit number.
 *      ptp_id      - (IN)  PTP stack ID.
 *      clock_num   - (IN)  PTP clock number.
 *      port_num    - (IN)  PTP clock port number.
 *      description - (OUT) PTP clock description.
 * Returns:
 *      BCM_E_XXX - Function status.
 * Notes:
 */
int
_bcm_ptp_clock_description_get(
    int unit,
    bcm_ptp_stack_id_t ptp_id,
    int clock_num,
    int port_num,
    _bcm_ptp_clock_description_t *description)
{
    int rv = BCM_E_UNAVAIL;

    uint8 resp[PTP_MGMTMSG_RESP_MAX_SIZE_OCTETS];
    int resp_len = PTP_MGMTMSG_RESP_MAX_SIZE_OCTETS;

    bcm_ptp_port_identity_t portid;

    if (BCM_FAILURE(rv = _bcm_ptp_function_precheck(unit, ptp_id, clock_num, port_num))) {
        return rv;
    }

    if (BCM_FAILURE(rv = bcm_common_ptp_clock_port_identity_get(unit, ptp_id,
            clock_num, port_num, &portid))) {
        return rv;
    }

    if (BCM_FAILURE(rv = _bcm_ptp_management_message_send(unit, ptp_id,
            clock_num, &portid, PTP_MGMTMSG_GET, PTP_MGMTMSG_ID_CLOCK_DESCRIPTION,
            0, 0, resp, &resp_len))) {
        return rv;
    }

    description->size = resp_len;
    description->data = sal_alloc(resp_len,"_bcm_ptp_clock_description_get");
    memcpy(description->data,resp,resp_len);
    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_ptp_show_system_info
 * Purpose:
 *      Report ToP CPU and memory use information to debug stream.
 * Parameters:
 *      unit      - (IN) Unit number.
 *      ptp_id    - (IN) PTP stack ID.
 *      clock_num - (IN) PTP clock number.
 * Returns:
 *      BCM_E_XXX - Function status.
 * Notes:
 */
int
_bcm_ptp_show_system_info(
    int unit,
    bcm_ptp_stack_id_t ptp_id,
    int clock_num)
{
    int rv = BCM_E_UNAVAIL;

    uint8 resp[PTP_MGMTMSG_RESP_MAX_SIZE_OCTETS];
    int resp_len = PTP_MGMTMSG_RESP_MAX_SIZE_OCTETS;

    bcm_ptp_port_identity_t portid;

    if (BCM_FAILURE(rv = _bcm_ptp_function_precheck(unit, ptp_id, clock_num,
            PTP_CLOCK_PORT_NUMBER_DEFAULT))) {
        return rv;
    }

    if (BCM_FAILURE(rv = bcm_common_ptp_clock_port_identity_get(unit, ptp_id,
            clock_num, PTP_IEEE1588_ALL_PORTS, &portid))) {
        return rv;
    }

    if (BCM_FAILURE(rv = _bcm_ptp_management_message_send(unit, ptp_id,
            clock_num, &portid, PTP_MGMTMSG_CMD, PTP_MGMTMSG_ID_SHOW_SYSTEM_INFO,
            0, 0, resp, &resp_len))) {
        return rv;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_common_ptp_peer_dataset_get
 * Purpose:
 *      Get Peer Dataset
 * Parameters:
 *      unit          - (IN) Unit number
 *      ptp_id        - (IN) PTP stack ID
 *      clock_num     - (IN) PTP clock number
 *      port_num      - (IN) PTP port number
 *      max_num_peers - (IN) max # of peer entries
 *      peers         - (OUT) list of peer entries
 *      *num_peers    - (OUT) actual # of peer entries returned in peers
 * Returns:
 *      int
 * Notes:
 */
int
bcm_common_ptp_peer_dataset_get(
    int unit,
    bcm_ptp_stack_id_t ptp_id,
    int clock_num,
    int port_num,
    int max_num_peers,
    bcm_ptp_peer_entry_t *peers,
    int *num_peers)
{
    int rc;

    rc = _bcm_ptp_peer_dataset_get(unit, ptp_id, clock_num, port_num, max_num_peers, peers, num_peers);

    return (rc);
}

#endif /* defined(INCLUDE_PTP)*/
