/*
 * $Id: counter.c,v 1.10 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Packet Statistics Counter Management
 *
 */

#include <shared/bsl.h>

#include <soc/drv.h>
#include <soc/debug.h>
#include <soc/robo_stat.h>

 

 /*
  *  Function : drv_counter_thread_set
  *
  *  Purpose :
  *      Set the operation code of the counter thread.
  *
  *  Parameters :
  *      thread_op   :   thread op code.
  *      uint    :   uint number.
  *      flags        :   mode settings.
  *      interval   :   counters scan interval in microseconds.
  *      bmp  :   port bitmap.
  *
  *  Return :
  *      SOC_E_XXX
  *
  *  Note :
  *      
  *
  */
 int
 drv_counter_thread_set(int unit, uint32 thread_op, uint32 flags, 
                 int interval, soc_pbmp_t bmp)
 {
     int  rv = SOC_E_NONE;

     if (soc_feature(unit, soc_feature_no_stat_mib)) {
         return SOC_E_UNAVAIL;
     }

     switch (thread_op) {
         case DRV_COUNTER_THREAD_START:
#ifdef BCM_DINO8_SUPPORT
             if (SOC_IS_DINO8(unit)) {
                 SOC_PBMP_PORT_REMOVE(bmp, CMIC_PORT(unit));
             }
#endif /* BCM_DINO8_SUPPORT */
             rv = soc_robo_counter_start(unit, flags, interval, bmp);
             break;
         case DRV_COUNTER_THREAD_STOP:
             rv = soc_robo_counter_stop(unit);
             break;
         case DRV_COUNTER_THREAD_SYNC:
             rv = soc_robo_counter_sync(unit);
             break;
         default:
             rv = SOC_E_PARAM;
     }
 
     return rv;
 }
 
 
 /*
  *  Function : drv_counter_set
  *
  *  Purpose :
  *      Set the snmp counter value.
  *
  *  Parameters :
  *      uint    :   uint number.
  *      port        :   port bitmap.
  *      counter_type   :   counter_type.
  *                  If want clear all counters associated to this port, 
  *                  counter_type = DRV_SNMP_COUNTER_COUNT
  *      val  :   counter val.
  *              Now only support to set zero.
  *
  *  Return :
  *      SOC_E_XXX
  *
  *  Note :
  *      
  *
  */
 int
 drv_counter_set(int unit, soc_pbmp_t bmp, uint32 counter_type, uint64 val)
 {
     int  rv = SOC_E_NONE;
     uint32  port;

     if (soc_feature(unit, soc_feature_no_stat_mib)) {
         return SOC_E_UNAVAIL;
     }
 
     if (!COMPILER_64_IS_ZERO(val)) {
         return SOC_E_UNAVAIL;
     }
 
     /* set by port */
     if (counter_type == DRV_SNMP_VAL_COUNT) {
         rv = soc_robo_counter_set_by_port(unit, bmp, val);
         return rv;
     }
     
     PBMP_ITER(bmp, port) {
         switch (counter_type) {
             /* *** RFC 1213 *** */
             case DRV_SNMP_IF_IN_OCTETS:         
                 soc_robo_counter_set(unit, port, INDEX(RXOCTETSr), val);
                 break;
 
             case DRV_SNMP_IF_IN_UCAST_PKTS:     
                 /* Total non-error frames minus broadcast/multicast frames */
                 soc_robo_counter_set(unit, port, INDEX(RXUNICASTPKTSr), val);  
                 break;
 
             case DRV_SNMP_IF_IN_N_UCAST_PKTS:    
                 /* Multicast frames plus broadcast frames */
                 soc_robo_counter_set(unit, port, INDEX(RXMULTICASTPKTSr), val);
                 if (SOC_IS_TBX(unit) || SOC_IS_POLAR(unit) || 
                     SOC_IS_NORTHSTAR(unit) || SOC_IS_NORTHSTARPLUS(unit) ||
                     SOC_IS_STARFIGHTER3(unit)) {
#if defined(BCM_TB_SUPPORT) || defined(BCM_POLAR_SUPPORT) || \
    defined(BCM_NORTHSTAR_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT) || \
    defined(BCM_STARFIGHTER3_SUPPORT)
                     soc_robo_counter_set
                         (unit, port, INDEX(RXBROADCASTPKTSr), val);
#endif
                 } else {
                     soc_robo_counter_set
                         (unit, port, INDEX(RXBROADCASTPKTr), val);
                 }
                 soc_robo_counter_set(unit, port, INDEX(RXPAUSEPKTSr), val);
 
                 break;
 
 
             case DRV_SNMP_IF_IN_DISCARDS:
                 if (SOC_IS_TBX(unit)) {
#ifdef BCM_TB_SUPPORT
                     soc_robo_counter_set
                         (unit, port, INDEX(RXFWDDISCPKTSr), val);
#endif
                 } else if (SOC_IS_HARRIER(unit)){
#ifdef BCM_HARRIER_SUPPORT
                     soc_robo_counter_set(unit, port, INDEX(RXDROPPKTSr), val);
                     soc_robo_counter_set
                         (unit, port, INDEX(RXFWDDISCPKTSr), val);
#endif /* BCM_HARRIER_SUPPORT */ 
                 } else if (SOC_IS_ROBO_ARCH_VULCAN(unit)) {
#ifdef BCM_GEX_SUPPORT
                     soc_robo_counter_set(unit, port, INDEX(RXDROPPKTSr), val);
                     soc_robo_counter_set(unit, port, INDEX(RXDISCARDr), val);
#endif
                 } else if (SOC_IS_DINO8(unit)) {
#ifdef BCM_DINO8_SUPPORT
                     soc_robo_counter_set(unit, port, INDEX(RXDROPPKTSr), val);
#endif /* BCM_DINO8_SUPPORT */
                 } else {
                     return SOC_E_UNAVAIL;
                     
                 }
                 break;
 
             case DRV_SNMP_IF_IN_ERRORS: 
                 soc_robo_counter_set
                     (unit, port, INDEX(RXALIGNMENTERRORSr), val);
                 soc_robo_counter_set(unit, port, INDEX(RXFCSERRORSr), val);
                 soc_robo_counter_set(unit, port, INDEX(RXFRAGMENTSr), val);
                 soc_robo_counter_set(unit, port, INDEX(RXOVERSIZEPKTSr), val);
                 soc_robo_counter_set(unit, port, INDEX(RXUNDERSIZEPKTSr), val);
                 if (SOC_IS_TBX(unit)) {
#ifdef BCM_TB_SUPPORT
                     soc_robo_counter_set
                         (unit, port, INDEX(RXJABBERPKTSr), val);
                     soc_robo_counter_set(unit, port, INDEX(RXSYMBOLERRr), val);
#endif
                 } else {
                     soc_robo_counter_set(unit, port, INDEX(RXJABBERSr), val);
                     soc_robo_counter_set(unit, port, INDEX(RXSYMBLERRr), val);
                 }
                 break;
 
             case DRV_SNMP_IF_IN_UNKNOWN_PROTOS:
                 break;
 
             case DRV_SNMP_IF_OUT_OCTETS:
                 soc_robo_counter_set(unit, port, INDEX(TXOCTETSr), val);   
                 break;
 
             case DRV_SNMP_IF_OUT_UCAST_PKTS:  /* ALL - mcast - bcast */
                 soc_robo_counter_set(unit, port, INDEX(TXUNICASTPKTSr), val);  
                 break;
 
             case DRV_SNMP_IF_OUT_N_UCAST_PKTS:  
                 /* broadcast frames plus multicast frames */
                 soc_robo_counter_set(unit, port, INDEX(TXBROADCASTPKTSr), val);
                 soc_robo_counter_set(unit, port, INDEX(TXMULTICASTPKTSr), val);
                 soc_robo_counter_set(unit, port, INDEX(TXPAUSEPKTSr), val);
                 break;
 
 
             case DRV_SNMP_IF_OUT_DISCARDS:
                 if (SOC_IS_TBX(unit)) {
#ifdef BCM_TB_SUPPORT
                     soc_robo_counter_set
                         (unit, port, INDEX(TXPORTCONGESTIONDROPr), val);
#endif
                 } else {
                     soc_robo_counter_set(unit, port, INDEX(TXDROPPKTSr), val);
                     soc_robo_counter_set
                         (unit, port, INDEX(TXFRAMEINDISCr), val);
                 }
                 break;
 
             case DRV_SNMP_IF_OUT_ERRORS:
                 soc_robo_counter_set
                     (unit, port, INDEX(TXEXCESSIVECOLLISIONr), val);
                 soc_robo_counter_set(unit, port, INDEX(TXLATECOLLISIONr), val);
                 break;
 
             case DRV_SNMP_IF_OUT_Q_LEN:  /* robo not suppport */
                 return SOC_E_UNAVAIL;
 
             case DRV_SNMP_IP_IN_RECEIVES:  /* robo not suppport */
                 return SOC_E_UNAVAIL;
 
             case DRV_SNMP_IP_IN_HDR_ERRORS:  /* robo not suppport */
                 return SOC_E_UNAVAIL;
 
             case DRV_SNMP_IP_FORW_DATAGRAMS:  /* robo not suppport */
                 return SOC_E_UNAVAIL;
 
             case DRV_SNMP_IP_IN_DISCARDS:  /* robo not suppport */
                 return SOC_E_UNAVAIL;
 
 
             /* *** RFC 1493 *** */   
 
             case DRV_SNMP_DOT1D_BASE_PORT_DELAY_EXCEEDED_DISCARDS:  
                 /* robo not suppport */
                 return SOC_E_UNAVAIL;
 
             case DRV_SNMP_DOT1D_BASE_PORT_MTU_EXCEEDED_DISCARDS:  
                 /* robo not suppport */
                 return SOC_E_UNAVAIL;
 
 
             case DRV_SNMP_DOT1D_TP_PORT_IN_FRAMES:
                 soc_robo_counter_set(unit, port, INDEX(RXUNICASTPKTSr), val);
                 soc_robo_counter_set(unit, port, INDEX(RXMULTICASTPKTSr), val);
                 soc_robo_counter_set(unit, port, INDEX(RXPAUSEPKTSr), val);
                 if (SOC_IS_TBX(unit) || SOC_IS_POLAR(unit) || 
                     SOC_IS_NORTHSTAR(unit) || SOC_IS_NORTHSTARPLUS(unit) ||
                     SOC_IS_STARFIGHTER3(unit)) {
#if defined(BCM_TB_SUPPORT) || defined(BCM_POLAR_SUPPORT) || \
    defined(BCM_NORTHSTAR_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT) || \
    defined(BCM_STARFIGHTER3_SUPPORT)
                     soc_robo_counter_set
                         (unit, port, INDEX(RXBROADCASTPKTSr), val);
#endif
                 } else {
                     soc_robo_counter_set
                         (unit, port, INDEX(RXBROADCASTPKTr), val);
                 }
                 break;
 
             case DRV_SNMP_DOT1D_TP_PORT_OUT_FRAMES:
                 soc_robo_counter_set(unit, port, INDEX(TXUNICASTPKTSr), val);
                 soc_robo_counter_set(unit, port, INDEX(TXMULTICASTPKTSr), val);
                 soc_robo_counter_set(unit, port, INDEX(TXBROADCASTPKTSr), val);
                 soc_robo_counter_set(unit, port, INDEX(TXPAUSEPKTSr), val);
                 break;
 
             case DRV_SNMP_DOT1D_PORT_IN_DISCARDS:
                 if (SOC_IS_TBX(unit)|| SOC_IS_HARRIER(unit)){
#if defined(BCM_TB_SUPPORT) || defined(BCM_HARRIER_SUPPORT)
                     soc_robo_counter_set
                         (unit, port, INDEX(RXFWDDISCPKTSr), val);
#endif
                 } else if (SOC_IS_ROBO_ARCH_VULCAN(unit)) {
#ifdef BCM_GEX_SUPPORT
                     soc_robo_counter_set(unit, port, INDEX(RXDISCARDr), val);
#endif
                 } else {
                     soc_robo_counter_set(unit, port, INDEX(RXDROPPKTSr), val); 
                 }
                 break;
 
             /* *** RFC 1757 *** */
             case DRV_SNMP_ETHER_STATS_DROP_EVENTS:
                 if (SOC_IS_TBX(unit)) {
#ifdef BCM_TB_SUPPORT
                     return SOC_E_UNAVAIL;
#endif
                 } else {
                     soc_robo_counter_set(unit, port, INDEX(TXDROPPKTSr), val);
                     soc_robo_counter_set(unit, port, INDEX(RXDROPPKTSr), val);
                 }
                 break;
 
             case DRV_SNMP_ETHER_STATS_MULTICAST_PKTS:
                 soc_robo_counter_set(unit, port, INDEX(RXMULTICASTPKTSr), val);
                 soc_robo_counter_set(unit, port, INDEX(TXMULTICASTPKTSr), val);
                 break;
 
             case DRV_SNMP_ETHER_STATS_BROADCAST_PKTS:
                 if (SOC_IS_TBX(unit) || SOC_IS_POLAR(unit) || 
                     SOC_IS_NORTHSTAR(unit) || SOC_IS_NORTHSTARPLUS(unit) ||
                     SOC_IS_STARFIGHTER3(unit)) {
#if defined(BCM_TB_SUPPORT) || defined(BCM_POLAR_SUPPORT) || \
    defined(BCM_NORTHSTAR_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT) || \
    defined(BCM_STARFIGHTER3_SUPPORT)
                     soc_robo_counter_set
                         (unit, port, INDEX(RXBROADCASTPKTSr), val);
#endif
                 } else {
                     soc_robo_counter_set
                         (unit, port, INDEX(RXBROADCASTPKTr), val);
                 }
                 soc_robo_counter_set(unit, port, INDEX(TXBROADCASTPKTSr), val);
                 break;
 
             case DRV_SNMP_ETHER_STATS_UNDERSIZE_PKTS:   /* Undersize frames */
                 soc_robo_counter_set(unit, port, INDEX(RXUNDERSIZEPKTSr), val);    
                 break;
 
             case DRV_SNMP_ETHER_STATS_FRAGMENTS:
                 soc_robo_counter_set(unit, port, INDEX(RXFRAGMENTSr), val); 
                 break;
 
             case DRV_SNMP_ETHER_STATS_PKTS64_OCTETS:
                 if (SOC_IS_TBX(unit)) {
#if defined(BCM_TB_SUPPORT)
                     soc_robo_counter_set
                         (unit, port, INDEX(RXPKTS64OCTETSr), val);
                     soc_robo_counter_set
                         (unit, port, INDEX(TXPKTS64OCTETSr), val);
#endif
                 } else if (SOC_IS_STARFIGHTER3(unit)) {
#if defined(BCM_STARFIGHTER3_SUPPORT)
                     soc_robo_counter_set
                         (unit, port, INDEX(RXPKTS64OCTETSr), val);
#endif
                 } else {
                     soc_robo_counter_set
                         (unit, port, INDEX(PKTS64OCTETSr), val);
                 }
                 break;
 
             case DRV_SNMP_ETHER_STATS_PKTS65TO127_OCTETS:
                 if (SOC_IS_TBX(unit)) {
#if defined(BCM_TB_SUPPORT)
                     soc_robo_counter_set
                         (unit, port, INDEX(RXPKTS65TO127OCTETSr), val);
                     soc_robo_counter_set
                         (unit, port, INDEX(TXPKTS65TO127OCTETSr), val);
#endif
                 } else if (SOC_IS_STARFIGHTER3(unit)) {
#if defined(BCM_STARFIGHTER_SUPPORT)
                     soc_robo_counter_set
                         (unit, port, INDEX(RXPKTS65TO127OCTETSr), val);
#endif
                 } else {
                     soc_robo_counter_set
                         (unit, port, INDEX(PKTS65TO127OCTETSr), val);   
                 }
                 break;
 
             case DRV_SNMP_ETHER_STATS_PKTS128TO255_OCTETS:
                 if (SOC_IS_TBX(unit)) {
#if defined(BCM_TB_SUPPORT)
                     soc_robo_counter_set
                         (unit, port, INDEX(RXPKTS128TO255OCTETSr), val);
                     soc_robo_counter_set
                         (unit, port, INDEX(TXPKTS128TO255OCTETSr), val);
#endif
                 } else if (SOC_IS_STARFIGHTER3(unit)) {
#if defined(BCM_STARFIGHTER3_SUPPORT)
                     soc_robo_counter_set
                         (unit, port, INDEX(RXPKTS128TO255OCTETSr), val);
#endif
                 } else {
                     soc_robo_counter_set
                         (unit, port, INDEX(PKTS128TO255OCTETSr), val);
                 }
                 break;
 
             case DRV_SNMP_ETHER_STATS_PKTS256TO511_OCTETS:
                 if (SOC_IS_TBX(unit)) {
#if defined(BCM_TB_SUPPORT)
                     soc_robo_counter_set
                         (unit, port, INDEX(RXPKTS256TO511OCTETSr), val);
                     soc_robo_counter_set
                         (unit, port, INDEX(TXPKTS256TO511OCTETSr), val);
#endif
                 } else if (SOC_IS_STARFIGHTER3(unit)) {
#if defined(BCM_STARFIGHTER3_SUPPORT)
                     soc_robo_counter_set
                         (unit, port, INDEX(RXPKTS256TO511OCTETSr), val);
#endif
                 } else {
                     soc_robo_counter_set
                         (unit, port, INDEX(PKTS256TO511OCTETSr), val);
                 }
                 break;
 
             case DRV_SNMP_ETHER_STATS_PKTS512TO1023_OCTETS:
                 if (SOC_IS_TBX(unit)) {
#if defined(BCM_TB_SUPPORT)
                     soc_robo_counter_set
                         (unit, port, INDEX(RXPKTS512TO1023OCTETSr), val);
                     soc_robo_counter_set
                         (unit, port, INDEX(TXPKTS512TO1023OCTETSr), val);
#endif
                 } else if (SOC_IS_STARFIGHTER3(unit)) {
#if defined(BCM_STARFIGHTER3_SUPPORT)
                     soc_robo_counter_set
                         (unit, port, INDEX(RXPKTS512TO1023OCTETSr), val);
#endif
                 } else {
                     soc_robo_counter_set
                         (unit, port, INDEX(PKTS512TO1023OCTETSr), val);
                 }
                 break;
 
             case DRV_SNMP_ETHER_STATS_PKTS1024TO1518_OCTETS:
                 /*
                  * ROBO chips don't support this range MIBs (Pkts1024to1518Octets) 
                  * The MIB counter range for ROBO chips are :
                  * - 1024 to standard maximum packet size,
                  * - 1024 to 1522 packet size.
                  */
                 return SOC_E_UNAVAIL;
 
             case DRV_SNMP_ETHER_STATS_OVERSIZE_PKTS:
                 soc_robo_counter_set(unit, port, INDEX(RXOVERSIZEPKTSr), val);  
                 break;
             case DRV_SNMP_ETHER_RX_OVERSIZE_PKTS:
                 soc_robo_counter_set(unit, port, INDEX(RXOVERSIZEPKTSr), val);  
                 break;
             case DRV_SNMP_ETHER_TX_OVERSIZE_PKTS:
                 return SOC_E_UNAVAIL;  
                 break;
 
             case DRV_SNMP_ETHER_STATS_JABBERS:
                 if (SOC_IS_TBX(unit)) {
#ifdef BCM_TB_SUPPORT
                     soc_robo_counter_set
                         (unit, port, INDEX(RXJABBERPKTSr), val);
#endif
                 } else {
                     soc_robo_counter_set(unit, port, INDEX(RXJABBERSr), val);
                 }
                 break;
 
             case DRV_SNMP_ETHER_STATS_OCTETS:
                 soc_robo_counter_set(unit, port, INDEX(RXOCTETSr), val);
                 soc_robo_counter_set(unit, port, INDEX(TXOCTETSr), val);
                 break;
 
             case DRV_SNMP_ETHER_STATS_PKTS:
                 soc_robo_counter_set(unit, port, INDEX(RXUNICASTPKTSr), val);   
                 soc_robo_counter_set(unit, port, INDEX(RXMULTICASTPKTSr), val);
 
                 soc_robo_counter_set(unit, port, INDEX(TXUNICASTPKTSr), val);   
                 soc_robo_counter_set(unit, port, INDEX(TXMULTICASTPKTSr), val);
                 soc_robo_counter_set(unit, port, INDEX(TXBROADCASTPKTSr), val);
                 soc_robo_counter_set(unit, port, INDEX(TXPAUSEPKTSr), val);
     
                 soc_robo_counter_set
                     (unit, port, INDEX(RXALIGNMENTERRORSr), val);
                 soc_robo_counter_set(unit, port, INDEX(RXFCSERRORSr), val);
                 soc_robo_counter_set(unit, port, INDEX(RXFRAGMENTSr), val);
                 soc_robo_counter_set(unit, port, INDEX(RXOVERSIZEPKTSr), val);
                 soc_robo_counter_set(unit, port, INDEX(RXPAUSEPKTSr), val);
                 soc_robo_counter_set(unit, port, INDEX(RXUNDERSIZEPKTSr), val);
                 if (SOC_IS_TBX(unit)) {
#ifdef BCM_TB_SUPPORT
                     soc_robo_counter_set
                         (unit, port, INDEX(RXBROADCASTPKTSr), val);
                     soc_robo_counter_set
                         (unit, port, INDEX(RXJABBERPKTSr), val);
                     soc_robo_counter_set(unit, port, INDEX(RXSYMBOLERRr), val);
#endif
                 } else {
                     if (SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) || 
                         SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
                         soc_robo_counter_set
                             (unit, port, INDEX(RXBROADCASTPKTSr), val);
#endif
                     } else {
                         soc_robo_counter_set
                             (unit, port, INDEX(RXBROADCASTPKTr), val);
                     }
                     soc_robo_counter_set(unit, port, INDEX(RXJABBERSr), val);
                     soc_robo_counter_set(unit, port, INDEX(RXSYMBLERRr), val);
                 }
                 break;
 
             case DRV_SNMP_ETHER_STATS_COLLISIONS:
                 soc_robo_counter_set(unit, port, INDEX(TXCOLLISIONSr), val);    
                 break;
 
             case DRV_SNMP_ETHER_STATS_CRC_ALIGN_ERRORS: 
                 /* CRC errors + alignment errors */
                 soc_robo_counter_set
                     (unit, port, INDEX(RXALIGNMENTERRORSr), val);
                 soc_robo_counter_set(unit, port, INDEX(RXFCSERRORSr), val);
                 break;
 
 
             case DRV_SNMP_ETHER_STATS_TX_NO_ERRORS:  
                 /*  TPKT - (TNCL + TOVR + TFRG + TUND) */
                 return SOC_E_UNAVAIL;
 
             case DRV_SNMP_ETHER_STATS_RX_NO_ERRORS:  
                 /* RPKT - ( RFCS + RXUO + RFLR) */
                 return SOC_E_UNAVAIL;       
 
             /* RFC 1643 */        
             case DRV_SNMP_DOT3_STATS_INTERNAL_MAC_RECEIVE_ERRORS:       
                 if (SOC_IS_TBX(unit)) {
#ifdef BCM_TB_SUPPORT
                     soc_robo_counter_set
                         (unit, port, INDEX(RXFWDDISCPKTSr), val);
#endif
                 } else {
                     soc_robo_counter_set(unit, port, INDEX(RXDROPPKTSr), val);
                 }
                 break;        
 
             case DRV_SNMP_DOT3_STATS_FRAME_TOO_LONGS:
                 soc_robo_counter_set(unit, port, INDEX(RXOVERSIZEPKTSr), val);
                 if (SOC_IS_TBX(unit)) {
#ifdef BCM_TB_SUPPORT
                     soc_robo_counter_set
                         (unit, port, INDEX(RXJABBERPKTSr), val);
#endif
                 } else {
                     soc_robo_counter_set(unit, port, INDEX(RXJABBERSr), val);
                 }
                 break;  
 
             case DRV_SNMP_DOT3_STATS_ALIGNMENT_ERRORS:  /* *** RFC 2665 *** */
                 soc_robo_counter_set
                     (unit, port, INDEX(RXALIGNMENTERRORSr), val);   
                 break;      
 
             case DRV_SNMP_DOT3_STATS_FCS_ERRORS:    /* *** RFC 2665 *** */
                 soc_robo_counter_set(unit, port, INDEX(RXFCSERRORSr), val); 
                 break;  
 
             case DRV_SNMP_DOT3_STATS_INTERNAL_MAC_TRANSMIT_ERRORS:  
                 if (SOC_IS_TBX(unit)) {
#ifdef BCM_TB_SUPPORT
                     soc_robo_counter_set
                         (unit, port, INDEX(TXPORTCONGESTIONDROPr), val);
#endif
                 } else {
                     soc_robo_counter_set(unit, port, INDEX(TXDROPPKTSr), val);
                 }
                 break;      
 
             case DRV_SNMP_DOT3_STATS_SINGLE_COLLISION_FRAMES:   
                 /* *** RFC 2665 *** */
                 if (SOC_IS_TBX(unit)) {
#ifdef BCM_TB_SUPPORT
                     soc_robo_counter_set
                         (unit, port, INDEX(TXSINGLECOLLISIONSr), val);
#endif
                 } else {
                     soc_robo_counter_set
                         (unit, port, INDEX(TXSINGLECOLLISIONr), val);
                 }
                 break;    
 
             case DRV_SNMP_DOT3_STATS_MULTIPLE_COLLISION_FRAMES: 
                 /* *** RFC 2665 *** */
                 if (SOC_IS_TBX(unit)) {
#ifdef BCM_TB_SUPPORT
                     soc_robo_counter_set
                         (unit, port, INDEX(TXMULTIPLECOLLISIONSr), val);
#endif
                 } else {
                     soc_robo_counter_set
                         (unit, port, INDEX(TXMULTIPLECOLLISIONr), val);
                 }
                 break;
 
             case DRV_SNMP_DOT3_STATS_DEFERRED_TRANSMISSIONS:    
                 soc_robo_counter_set
                     (unit, port, INDEX(TXDEFERREDTRANSMITr), val);  
                 break;  
 
             case DRV_SNMP_DOT3_STATS_LATE_COLLISIONS:   
                 soc_robo_counter_set(unit, port, INDEX(TXLATECOLLISIONr), val); 
                 break;      
 
             case DRV_SNMP_DOT3_STATS_EXCESSIVE_COLLISIONS:  
                 soc_robo_counter_set
                     (unit, port, INDEX(TXEXCESSIVECOLLISIONr), val);    
                 break;      
 
             case DRV_SNMP_DOT3_STATS_CARRIER_SENSE_ERRORS:  
                 /* not support for robo */
                 return SOC_E_UNAVAIL;
 
             case DRV_SNMP_DOT3_STATS_SQET_TEST_ERRORS:   
                 /* not support for robo */   
                 return SOC_E_UNAVAIL;                                               
 
                 
             /* *** RFC 2665 *** some object same as RFC 1643 */
 
             case DRV_SNMP_DOT3_STATS_SYMBOL_ERRORS:  
                 if (SOC_IS_TBX(unit)) {
#ifdef BCM_TB_SUPPORT
                     soc_robo_counter_set(unit, port, INDEX(RXSYMBOLERRr), val);
#endif
                 } else {
                     soc_robo_counter_set(unit, port, INDEX(RXSYMBLERRr), val);
                 }
                 break;
 
             case DRV_SNMP_DOT3_CONTROL_IN_UNKNOWN_OPCODES:  
                 /* not support for robo */   
                 return SOC_E_UNAVAIL;
 
             case DRV_SNMP_DOT3_IN_PAUSE_FRAMES:  
                 soc_robo_counter_set(unit, port, INDEX(RXPAUSEPKTSr), val);    
                 break;  
 
             case DRV_SNMP_DOT3_OUT_PAUSE_FRAMES:  
                 soc_robo_counter_set(unit, port, INDEX(TXPAUSEPKTSr), val);    
                 break;  
 
             /*** RFC 2233 ***/
             case DRV_SNMP_IF_HC_IN_OCTETS:   
                 soc_robo_counter_set(unit, port, INDEX(RXOCTETSr), val);    
                 break;      
 
             case DRV_SNMP_IF_HC_IN_UCAST_PKTS: 
                 soc_robo_counter_set(unit, port, INDEX(RXUNICASTPKTSr), val);   
                 break;      
 
             case DRV_SNMP_IF_HC_IN_MULTICAST_PKTS:   
             case DRV_SNMP_IF_IN_MULTICAST_PKTS:   
                 soc_robo_counter_set(unit, port, INDEX(RXMULTICASTPKTSr), val); 
                 break;      
 
             case DRV_SNMP_IF_HC_IN_BROADCAST_PKTS:   
             case DRV_SNMP_IF_IN_BROADCAST_PKTS:   
                 if (SOC_IS_TBX(unit) || SOC_IS_POLAR(unit) || 
                     SOC_IS_NORTHSTAR(unit) || SOC_IS_NORTHSTARPLUS(unit) ||
                     SOC_IS_STARFIGHTER3(unit)) {
#if defined(BCM_TB_SUPPORT) || defined(BCM_POLAR_SUPPORT) || \
    defined(BCM_NORTHSTAR_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT) || \
    defined(BCM_STARFIGHTER3_SUPPORT)
                     soc_robo_counter_set
                         (unit, port, INDEX(RXBROADCASTPKTSr), val);
#endif
                 } else {
                     soc_robo_counter_set
                         (unit, port, INDEX(RXBROADCASTPKTr), val);
                 }
                 break;    
 
             case DRV_SNMP_IF_HC_OUT_OCTETS:  
                 soc_robo_counter_set(unit, port, INDEX(TXOCTETSr), val);    
                 break;      
 
             case DRV_SNMP_IF_HC_OUT_UCAST_PKTS:    
                 soc_robo_counter_set(unit, port, INDEX(TXUNICASTPKTSr), val);   
                 break;      
 
             case DRV_SNMP_IF_HC_OUT_MULTICAST_PKTS:  
             case DRV_SNMP_IF_OUT_MULTICAST_PKTS:  
                 soc_robo_counter_set(unit, port, INDEX(TXMULTICASTPKTSr), val); 
                 break;      
 
             case DRV_SNMP_IF_HC_OUT_BROADCAST_PCKTS:  
             case DRV_SNMP_IF_OUT_BROADCAST_PKTS:  
                 soc_robo_counter_set(unit, port, INDEX(TXBROADCASTPKTSr), val); 
                 break;          
 
             /*** RFC 2465 ***/
             case DRV_SNMP_IPV6_IF_STATS_IN_RECEIVES:
             case DRV_SNMP_IPV6_IF_STATS_IN_HDR_ERRORS:
             case DRV_SNMP_IPV6_IF_STATS_IN_ADDR_ERRORS:
             case DRV_SNMP_IPV6_IF_STATS_IN_DISCARDS:
             case DRV_SNMP_IPV6_IF_STATS_OUT_FORW_DATAGRAMS:
             case DRV_SNMP_IPV6_IF_STATS_OUT_DISCARDS:
             case DRV_SNMP_IPV6_IF_STATS_IN_MCAST_PKTS:
             case DRV_SNMP_IPV6_IF_STATS_OUT_MCAST_PKTS:
                 /* not support for robo */
                 return SOC_E_UNAVAIL;
 
             /* IEEE 802.1bb */
             case DRV_SNMP_IEEE8021_PFC_REQUESTS:
             case DRV_SNMP_IEEE8021_PFC_INDICATIONS:
                 /* not support for robo */
                 return SOC_E_UNAVAIL;
 
             /*** Additional Broadcom stats ***/
             case DRV_SNMP_BCM_IPMC_BRIDGED_PCKTS:
             case DRV_SNMP_BCM_IPMC_ROUTED_PCKTS:    
             case DRV_SNMP_BCM_IPMC_IN_DROPPED_PCKTS:
             case DRV_SNMP_BCM_IPMC_OUT_DROPPED_PCKTS:
                 /* not support for robo */
                 return SOC_E_UNAVAIL;
 
             case DRV_SNMP_BCM_ETHER_STATS_PKTS1519TO1522_OCTETS:
                 /* not support for robo */
                 return SOC_E_UNAVAIL;
             case DRV_SNMP_BCM_ETHER_STATS_PKTS1522TO2047_OCTETS:
                 return SOC_E_UNAVAIL;
             case DRV_SNMP_BCM_ETHER_STATS_PKTS2048TO4095_OCTETS:
                 if (SOC_IS_DINO8(unit)) {
#ifdef BCM_DINO8_SUPPORT
                     soc_robo_counter_set
                         (unit, port, INDEX(PKTS2048TO4095r), val);
                     break;
#endif /* BCM_DINO8_SUPPORT */
                 } else {
                     return SOC_E_UNAVAIL;
                     
                 }
             case DRV_SNMP_BCM_ETHER_STATS_PKTS4095TO9216_OCTETS:
                 /* not support for robo */
                 return SOC_E_UNAVAIL;
 
             case DRV_SNMP_BCM_RECEIVED_PKTS64_OCTETS:
                 if (SOC_IS_TBX(unit) || SOC_IS_STARFIGHTER3(unit)) {
#if defined(BCM_TB_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
                     soc_robo_counter_set
                         (unit, port, INDEX(RXPKTS64OCTETSr), val);
                     break;
#endif
                 } else {
                     return SOC_E_UNAVAIL;
                 }
     
             case DRV_SNMP_BCM_RECEIVED_PKTS65TO127_OCTETS:
                 if (SOC_IS_TBX(unit) || SOC_IS_NORTHSTARPLUS(unit) ||
                      SOC_IS_STARFIGHTER3(unit)) {
#if defined(BCM_TB_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT) || \
                 defined(BCM_STARFIGHTER3_SUPPORT)
                     soc_robo_counter_set
                         (unit, port, INDEX(RXPKTS65TO127OCTETSr), val);
                     break;
#endif
                 } else {
                     return SOC_E_UNAVAIL;
                 }
     
             case DRV_SNMP_BCM_RECEIVED_PKTS128TO255_OCTETS:
                 if (SOC_IS_TBX(unit) || SOC_IS_NORTHSTARPLUS(unit) ||
                     SOC_IS_STARFIGHTER3(unit)) {
#if defined(BCM_TB_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT) || \
                 defined(BCM_STARFIGHTER3_SUPPORT)
                     soc_robo_counter_set
                         (unit, port, INDEX(RXPKTS128TO255OCTETSr), val);
                     break;
#endif
                 } else {
                     return SOC_E_UNAVAIL;
                 }
     
             case DRV_SNMP_BCM_RECEIVED_PKTS256TO511_OCTETS:
                 if (SOC_IS_TBX(unit) || SOC_IS_NORTHSTARPLUS(unit) ||
                     SOC_IS_STARFIGHTER3(unit)) {
#if defined(BCM_TB_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT) || \
                 defined(BCM_STARFIGHTER3_SUPPORT)
                     soc_robo_counter_set
                         (unit, port, INDEX(RXPKTS256TO511OCTETSr), val);
                     break;
#endif
                 } else {
                     return SOC_E_UNAVAIL;
                 }
     
             case DRV_SNMP_BCM_RECEIVED_PKTS512TO1023_OCTETS:
                 if (SOC_IS_TBX(unit) || SOC_IS_NORTHSTARPLUS(unit) ||
                     SOC_IS_STARFIGHTER3(unit)) {
#if defined(BCM_TB_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT) || \
                 defined(BCM_STARFIGHTER3_SUPPORT)
                     soc_robo_counter_set
                         (unit, port, INDEX(RXPKTS512TO1023OCTETSr), val);
                     break;
#endif
                 } else {
                     return SOC_E_UNAVAIL;
                 }
     
             case DRV_SNMP_BCM_RECEIVED_PKTS1024TO1518_OCTETS:
             case DRV_SNMP_BCM_RECEIVED_PKTS1519TO2047_OCTETS:
             case DRV_SNMP_BCM_RECEIVED_PKTS2048TO4095_OCTETS:
             case DRV_SNMP_BCM_RECEIVED_PKTS4095TO9216_OCTETS:
                return SOC_E_UNAVAIL;
     
             case DRV_SNMP_BCM_TRANSMITTED_PKTS64_OCTETS:
                 if (SOC_IS_TBX(unit) || SOC_IS_NORTHSTARPLUS(unit) ||
                     SOC_IS_STARFIGHTER3(unit)) {
#if defined(BCM_TB_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT) || \
                 defined(BCM_STARFIGHTER3_SUPPORT)
                     soc_robo_counter_set
                         (unit, port, INDEX(TXPKTS64OCTETSr), val);
                     break;
#endif
                 } else {
                     return SOC_E_UNAVAIL;
                 }
     
             case DRV_SNMP_BCM_TRANSMITTED_PKTS65TO127_OCTETS:
                 if (SOC_IS_TBX(unit) || SOC_IS_NORTHSTARPLUS(unit) ||
                     SOC_IS_STARFIGHTER3(unit)) {
#if defined(BCM_TB_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT) || \
                 defined(BCM_STARFIGHTER3_SUPPORT)
                     soc_robo_counter_set
                         (unit, port, INDEX(TXPKTS65TO127OCTETSr), val);
                     break;
#endif
                 } else {
                     return SOC_E_UNAVAIL;
                 }
     
             case DRV_SNMP_BCM_TRANSMITTED_PKTS128TO255_OCTETS:
                 if (SOC_IS_TBX(unit) || SOC_IS_NORTHSTARPLUS(unit) ||
                     SOC_IS_STARFIGHTER3(unit)) {
#if defined(BCM_TB_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT) || \
                 defined(BCM_STARFIGHTER3_SUPPORT)
                     soc_robo_counter_set
                         (unit, port, INDEX(TXPKTS128TO255OCTETSr), val);
                     break;
#endif
                 } else {
                     return SOC_E_UNAVAIL;
                 }
     
             case DRV_SNMP_BCM_TRANSMITTED_PKTS256TO511_OCTETS:
                 if (SOC_IS_TBX(unit) || SOC_IS_NORTHSTARPLUS(unit) ||
                     SOC_IS_STARFIGHTER3(unit)) {
#if defined(BCM_TB_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT) || \
                 defined(BCM_STARFIGHTER3_SUPPORT)
                     soc_robo_counter_set
                         (unit, port, INDEX(TXPKTS256TO511OCTETSr), val);
                     break;
#endif
                 } else {
                     return SOC_E_UNAVAIL;
                 }
     
             case DRV_SNMP_BCM_TRANSMITTED_PKTS512TO1023_OCTETS:
                 if (SOC_IS_TBX(unit) || SOC_IS_NORTHSTARPLUS(unit) ||
                     SOC_IS_STARFIGHTER3(unit)) {
#if defined(BCM_TB_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT) || \
                 defined(BCM_STARFIGHTER3_SUPPORT)
                     soc_robo_counter_set
                         (unit, port, INDEX(TXPKTS512TO1023OCTETSr), val);
                     break;
#endif
                 } else {
                     return SOC_E_UNAVAIL;
                 }
     
             case DRV_SNMP_BCM_TRANSMITTED_PKTS1024TO1518_OCTETS:
             case DRV_SNMP_BCM_TRANSMITTED_PKTS1519TO2047_OCTETS:
             case DRV_SNMP_BCM_TRANSMITTED_PKTS2048TO4095_OCTETS:
             case DRV_SNMP_BCM_TRANSMITTED_PKTS4095TO9216_OCTETS:
                 return SOC_E_UNAVAIL;
 
             case DRV_SNMP_BCM_CUSTOM_RECEIVE0:
             case DRV_SNMP_BCM_CUSTOM_RECEIVE1:
             case DRV_SNMP_BCM_CUSTOM_RECEIVE2:
             case DRV_SNMP_BCM_CUSTOM_RECEIVE3:
             case DRV_SNMP_BCM_CUSTOM_RECEIVE4:
             case DRV_SNMP_BCM_CUSTOM_RECEIVE5:
             case DRV_SNMP_BCM_CUSTOM_RECEIVE6:
             case DRV_SNMP_BCM_CUSTOM_RECEIVE7:
             case DRV_SNMP_BCM_CUSTOM_RECEIVE8:
             case DRV_SNMP_BCM_CUSTOM_TRANSMIT0:
             case DRV_SNMP_BCM_CUSTOM_TRANSMIT1:
             case DRV_SNMP_BCM_CUSTOM_TRANSMIT2:
             case DRV_SNMP_BCM_CUSTOM_TRANSMIT3:
             case DRV_SNMP_BCM_CUSTOM_TRANSMIT4:
             case DRV_SNMP_BCM_CUSTOM_TRANSMIT5:
             case DRV_SNMP_BCM_CUSTOM_TRANSMIT6:
             case DRV_SNMP_BCM_CUSTOM_TRANSMIT7:
             case DRV_SNMP_BCM_CUSTOM_TRANSMIT8:
             case DRV_SNMP_BCM_CUSTOM_TRANSMIT9:
             case DRV_SNMP_BCM_CUSTOM_TRANSMIT10:
             case DRV_SNMP_BCM_CUSTOM_TRANSMIT11:
             case DRV_SNMP_BCM_CUSTOM_TRANSMIT12:
             case DRV_SNMP_BCM_CUSTOM_TRANSMIT13:
             case DRV_SNMP_BCM_CUSTOM_TRANSMIT14:
                 /* not support for robo */
                 return SOC_E_UNAVAIL;
 
             default:
                 LOG_VERBOSE(BSL_LS_SOC_COMMON,
                             (BSL_META_U(unit,
                                         "drv_snmp_counter_set: Statistic not supported: %d\n"), 
                              counter_type));
                 return SOC_E_PARAM;
         }
     }
 
     return rv;
 }
 
  /*
  *  Function : drv_counter_get
  *
  *  Purpose :
  *      Get the snmp counter value.
  *
  *  Parameters :
  *      uint    :   uint number.
  *      port        :   port number.
  *      counter_type   :   counter_type.
  *      val  :   counter val.
  *
  *  Return :
  *      SOC_E_XXX
  *
  *  Note :
  *      
  *
  */
 int
 drv_counter_get(int unit, uint32 port, uint32 counter_type, 
     int sync_hw, uint64 *val)
 {
     uint64  count, count_tmp;
 
     COMPILER_64_ZERO(count);
     COMPILER_64_ZERO(count_tmp);

     if (soc_feature(unit, soc_feature_no_stat_mib)) {
         return SOC_E_UNAVAIL;
     }
 
     switch (counter_type) 
     {
         /* *** RFC 1213 *** */
 
         case DRV_SNMP_IF_IN_OCTETS:         
             soc_robo_counter_get(unit, port, INDEX(RXOCTETSr), 
                 sync_hw, &count);
             break;
 
         case DRV_SNMP_IF_IN_UCAST_PKTS:     
             /* Total non-error frames minus broadcast/multicast frames */
             soc_robo_counter_get(unit, port, INDEX(RXUNICASTPKTSr), 
                 sync_hw, &count);   
             break;
 
         case DRV_SNMP_IF_IN_N_UCAST_PKTS:    
             /* Multicast frames plus broadcast frames */
             soc_robo_counter_get(unit, port, INDEX(RXMULTICASTPKTSr), 
                 sync_hw, &count_tmp);
             count=count_tmp;
             if (SOC_IS_TBX(unit) || SOC_IS_POLAR(unit) || 
                 SOC_IS_NORTHSTAR(unit) || SOC_IS_NORTHSTARPLUS(unit) ||
                 SOC_IS_STARFIGHTER3(unit)) {
#if defined(BCM_TB_SUPPORT) || defined(BCM_POLAR_SUPPORT) || \
    defined(BCM_NORTHSTAR_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT) || \
    defined(BCM_STARFIGHTER3_SUPPORT)
                 soc_robo_counter_get(unit, port, INDEX(RXBROADCASTPKTSr), 
                     sync_hw, &count_tmp);
#endif
             } else {
                 soc_robo_counter_get(unit, port, INDEX(RXBROADCASTPKTr), 
                     sync_hw, &count_tmp);
             }
             COMPILER_64_ADD_64(count, count_tmp); 
             soc_robo_counter_get(unit, port, INDEX(RXPAUSEPKTSr), 
                 sync_hw, &count_tmp);
             COMPILER_64_ADD_64(count, count_tmp); 
             break;
 
 
         case DRV_SNMP_IF_IN_DISCARDS:
             if (SOC_IS_TBX(unit)) {
#ifdef BCM_TB_SUPPORT
                 soc_robo_counter_get(unit, port, INDEX(RXFWDDISCPKTSr), 
                     sync_hw, &count);
#endif
             } else if (SOC_IS_HARRIER(unit)){
#ifdef BCM_HARRIER_SUPPORT            
                 soc_robo_counter_get(unit, port, INDEX(RXDROPPKTSr), 
                     sync_hw, &count);
                 soc_robo_counter_get(unit, port, INDEX(RXFWDDISCPKTSr), 
                     sync_hw, &count_tmp);
                 COMPILER_64_ADD_64(count, count_tmp);
#endif /* BCM_HARRIER_SUPPORT */ 
             } else if (SOC_IS_ROBO_ARCH_VULCAN(unit)) {
#ifdef BCM_GEX_SUPPORT
                 soc_robo_counter_get(unit, port, INDEX(RXDROPPKTSr), 
                     sync_hw, &count);
                 soc_robo_counter_get(unit, port, INDEX(RXDISCARDr), 
                     sync_hw, &count_tmp);
                 COMPILER_64_ADD_64(count, count_tmp);
#endif
             } else if (SOC_IS_DINO8(unit)) {
#ifdef BCM_DINO8_SUPPORT
                 soc_robo_counter_get(unit, port, INDEX(RXDROPPKTSr), 
                     sync_hw, &count);
#endif /* BCM_DINO8_SUPPORT */
             } else {
                 /* 
                  * Other robo chips can't counter the packets were dropped by
                  * forwarding or filtering processes. 
                  */
                 return SOC_E_UNAVAIL;
             }
             break;
 
         case DRV_SNMP_IF_IN_ERRORS: 
             soc_robo_counter_get(unit, port, INDEX(RXALIGNMENTERRORSr), 
                 sync_hw, &count_tmp);
             count=count_tmp;
             soc_robo_counter_get(unit, port, INDEX(RXFCSERRORSr), 
                 sync_hw, &count_tmp);
             COMPILER_64_ADD_64(count, count_tmp);
             soc_robo_counter_get(unit, port, INDEX(RXFRAGMENTSr), 
                 sync_hw, &count_tmp);
             COMPILER_64_ADD_64(count, count_tmp);
             soc_robo_counter_get(unit, port, INDEX(RXOVERSIZEPKTSr), 
                 sync_hw, &count_tmp);
             COMPILER_64_ADD_64(count, count_tmp);
             soc_robo_counter_get(unit, port, INDEX(RXUNDERSIZEPKTSr), 
                 sync_hw, &count_tmp);
             COMPILER_64_ADD_64(count, count_tmp);
             if (SOC_IS_TBX(unit)) {
#ifdef BCM_TB_SUPPORT
                 soc_robo_counter_get(unit, port, INDEX(RXJABBERPKTSr), 
                     sync_hw, &count_tmp);
                 COMPILER_64_ADD_64(count, count_tmp);
                 soc_robo_counter_get(unit, port, INDEX(RXSYMBOLERRr), 
                     sync_hw, &count_tmp);
#endif
             } else {
                 soc_robo_counter_get(unit, port, INDEX(RXJABBERSr), 
                     sync_hw, &count_tmp);
                 COMPILER_64_ADD_64(count, count_tmp);
                 soc_robo_counter_get(unit, port, INDEX(RXSYMBLERRr), 
                     sync_hw, &count_tmp);
             }
             COMPILER_64_ADD_64(count, count_tmp);
             break;
     
             case DRV_SNMP_IF_IN_UNKNOWN_PROTOS:
                 COMPILER_64_ZERO(*val);
                 return SOC_E_UNAVAIL;
 
         case DRV_SNMP_IF_OUT_OCTETS:
             if (SOC_IS_HARRIER(unit)) {
#ifdef BCM_HARRIER_SUPPORT
                 soc_robo_counter_get(unit, port, INDEX(TXQOS0OCTETSr), 
                     sync_hw, &count_tmp);
                 count=count_tmp;
                 soc_robo_counter_get(unit, port, INDEX(TXQOS1OCTETSr), 
                     sync_hw, &count_tmp);
                 COMPILER_64_ADD_64(count, count_tmp);
                 soc_robo_counter_get(unit, port, INDEX(TXQOS2OCTETSr), 
                     sync_hw, &count_tmp);
                 COMPILER_64_ADD_64(count, count_tmp);
                 soc_robo_counter_get(unit, port, INDEX(TXQOS3OCTETSr), 
                     sync_hw, &count_tmp);
                 COMPILER_64_ADD_64(count, count_tmp);
#endif
             } else {
                 soc_robo_counter_get(unit, port, INDEX(TXOCTETSr), 
                     sync_hw, &count);
             }
             break;
 
         case DRV_SNMP_IF_OUT_UCAST_PKTS:  /* ALL - mcast - bcast */
             soc_robo_counter_get(unit, port, INDEX(TXUNICASTPKTSr), 
                 sync_hw, &count);
             break;
 
         case DRV_SNMP_IF_OUT_N_UCAST_PKTS:  
             /* broadcast frames plus multicast frames */
             soc_robo_counter_get(unit, port, INDEX(TXBROADCASTPKTSr), 
                 sync_hw, &count_tmp);
             count=count_tmp;
             soc_robo_counter_get(unit, port, INDEX(TXMULTICASTPKTSr), 
                 sync_hw, &count_tmp);
             COMPILER_64_ADD_64(count, count_tmp); 
             soc_robo_counter_get(unit, port, INDEX(TXPAUSEPKTSr), 
                 sync_hw, &count_tmp);
             COMPILER_64_ADD_64(count, count_tmp);
             break;
 
 
         case DRV_SNMP_IF_OUT_DISCARDS:
             if (SOC_IS_TBX(unit)) {
#ifdef BCM_TB_SUPPORT
                 soc_robo_counter_get(unit, port, INDEX(TXPORTCONGESTIONDROPr), 
                     sync_hw, &count);
#endif
             } else {
                 soc_robo_counter_get(unit, port, INDEX(TXDROPPKTSr), 
                     sync_hw, &count_tmp);
                 count=count_tmp;
                 soc_robo_counter_get(unit, port, INDEX(TXFRAMEINDISCr), 
                     sync_hw, &count_tmp);
                 COMPILER_64_ADD_64(count, count_tmp);
             }
             break;
 
         case DRV_SNMP_IF_OUT_ERRORS:
             soc_robo_counter_get(unit, port, INDEX(TXEXCESSIVECOLLISIONr), 
                 sync_hw, &count_tmp);
             count=count_tmp;
             soc_robo_counter_get(unit, port, INDEX(TXLATECOLLISIONr), 
                 sync_hw, &count_tmp);
             COMPILER_64_ADD_64(count, count_tmp);
             break;
 
         case DRV_SNMP_IF_OUT_Q_LEN:  /* robo not suppport */
             return SOC_E_UNAVAIL;
 
         case DRV_SNMP_IP_IN_RECEIVES:  /* robo not suppport */
             return SOC_E_UNAVAIL;
 
         case DRV_SNMP_IP_IN_HDR_ERRORS:  /* robo not suppport */
             return SOC_E_UNAVAIL;
 
         case DRV_SNMP_IP_FORW_DATAGRAMS:  /* robo not suppport */
             return SOC_E_UNAVAIL;
 
         case DRV_SNMP_IP_IN_DISCARDS:  /* robo not suppport */
             return SOC_E_UNAVAIL;
 
 
         /* *** RFC 1493 *** */   
 
         case DRV_SNMP_DOT1D_BASE_PORT_DELAY_EXCEEDED_DISCARDS:  
             /* robo not suppport */
             return SOC_E_UNAVAIL;
 
         case DRV_SNMP_DOT1D_BASE_PORT_MTU_EXCEEDED_DISCARDS:  
             /* robo not suppport */
             return SOC_E_UNAVAIL;
 
 
         case DRV_SNMP_DOT1D_TP_PORT_IN_FRAMES:
             soc_robo_counter_get(unit, port, INDEX(RXUNICASTPKTSr), 
                 sync_hw, &count_tmp);
             count=count_tmp;
             soc_robo_counter_get(unit, port, INDEX(RXMULTICASTPKTSr), 
                 sync_hw, &count_tmp);
             COMPILER_64_ADD_64(count, count_tmp);
             soc_robo_counter_get(unit, port, INDEX(RXPAUSEPKTSr), 
                 sync_hw, &count_tmp);
             COMPILER_64_ADD_64(count, count_tmp);
             if (SOC_IS_TBX(unit) || SOC_IS_POLAR(unit) || 
                 SOC_IS_NORTHSTAR(unit) || SOC_IS_NORTHSTARPLUS(unit) ||
                 SOC_IS_STARFIGHTER3(unit)) {
#if defined(BCM_TB_SUPPORT) || defined(BCM_POLAR_SUPPORT) || \
    defined(BCM_NORTHSTAR_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT) || \
    defined(BCM_STARFIGHTER3_SUPPORT)
                 soc_robo_counter_get(unit, port, INDEX(RXBROADCASTPKTSr), 
                     sync_hw, &count_tmp);
#endif
             } else {
                 soc_robo_counter_get(unit, port, INDEX(RXBROADCASTPKTr), 
                     sync_hw, &count_tmp);
             }
             COMPILER_64_ADD_64(count, count_tmp);
             break;
 
         case DRV_SNMP_DOT1D_TP_PORT_OUT_FRAMES:
             soc_robo_counter_get(unit, port, INDEX(TXUNICASTPKTSr), 
                 sync_hw, &count_tmp);
             count=count_tmp;
             soc_robo_counter_get(unit, port, INDEX(TXMULTICASTPKTSr), 
                 sync_hw, &count_tmp);
             COMPILER_64_ADD_64(count, count_tmp);
             soc_robo_counter_get(unit, port, INDEX(TXBROADCASTPKTSr), 
                 sync_hw, &count_tmp);
             COMPILER_64_ADD_64(count, count_tmp);
             soc_robo_counter_get(unit, port, INDEX(TXPAUSEPKTSr), 
                 sync_hw, &count_tmp);
             COMPILER_64_ADD_64(count, count_tmp);
             break;
 
         case DRV_SNMP_DOT1D_PORT_IN_DISCARDS:
             if (SOC_IS_TBX(unit)|| SOC_IS_HARRIER(unit)){
#if defined(BCM_TB_SUPPORT) || defined(BCM_HARRIER_SUPPORT)
                 soc_robo_counter_get(unit, port, INDEX(RXFWDDISCPKTSr), 
                     sync_hw, &count);
#endif
             } else if (SOC_IS_ROBO_ARCH_VULCAN(unit)) {
#ifdef BCM_GEX_SUPPORT
                 soc_robo_counter_get(unit, port, INDEX(RXDISCARDr), 
                     sync_hw, &count);
#endif
             } else {
                 soc_robo_counter_get(unit, port, INDEX(RXDROPPKTSr), 
                     sync_hw, &count);
             }
             break;
 
         /* *** RFC 1757 *** */
         case DRV_SNMP_ETHER_STATS_DROP_EVENTS:
             if (SOC_IS_TBX(unit)) {
#ifdef BCM_TB_SUPPORT
                 return SOC_E_UNAVAIL;
#endif
             } else {
                 soc_robo_counter_get(unit, port, INDEX(TXDROPPKTSr), 
                     sync_hw, &count_tmp);
                 count=count_tmp;
                 soc_robo_counter_get(unit, port, INDEX(RXDROPPKTSr), 
                     sync_hw, &count_tmp);
                 COMPILER_64_ADD_64(count, count_tmp);
             }
             break;
 
         case DRV_SNMP_ETHER_STATS_MULTICAST_PKTS:
             soc_robo_counter_get(unit, port, INDEX(RXMULTICASTPKTSr), 
                 sync_hw, &count_tmp);
             count = count_tmp;
             soc_robo_counter_get(unit, port, INDEX(TXMULTICASTPKTSr), 
                 sync_hw, &count_tmp);
             COMPILER_64_ADD_64(count, count_tmp);
             break;
 
         case DRV_SNMP_ETHER_STATS_BROADCAST_PKTS:
             if (SOC_IS_TBX(unit) || SOC_IS_POLAR(unit) || 
                 SOC_IS_NORTHSTAR(unit) || SOC_IS_NORTHSTARPLUS(unit) ||
                 SOC_IS_STARFIGHTER3(unit)) {
#if defined(BCM_TB_SUPPORT) || defined(BCM_POLAR_SUPPORT) || \
    defined(BCM_NORTHSTAR_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT) || \
    defined(BCM_STARFIGHTER3_SUPPORT)
                 soc_robo_counter_get(unit, port, INDEX(RXBROADCASTPKTSr), 
                     sync_hw, &count_tmp);
#endif
             } else {
                 soc_robo_counter_get(unit, port, INDEX(RXBROADCASTPKTr), 
                     sync_hw, &count_tmp);
             }
             count = count_tmp;
             soc_robo_counter_get(unit, port, INDEX(TXBROADCASTPKTSr), 
                 sync_hw, &count_tmp);
             COMPILER_64_ADD_64(count, count_tmp);
             break;
 
         case DRV_SNMP_ETHER_STATS_UNDERSIZE_PKTS:   /* Undersize frames */
             soc_robo_counter_get(unit, port, INDEX(RXUNDERSIZEPKTSr), 
                 sync_hw, &count);
             break;
 
         case DRV_SNMP_ETHER_STATS_FRAGMENTS:
             soc_robo_counter_get(unit, port, INDEX(RXFRAGMENTSr), 
                 sync_hw, &count);
             break;
 
         case DRV_SNMP_ETHER_STATS_PKTS64_OCTETS:
             if (SOC_IS_TBX(unit) || SOC_IS_NORTHSTARPLUS(unit)) {
#if defined(BCM_TB_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT)
                 soc_robo_counter_get(unit, port, INDEX(RXPKTS64OCTETSr), 
                     sync_hw, &count_tmp);
                 count = count_tmp;
                 soc_robo_counter_get(unit, port, INDEX(TXPKTS64OCTETSr), 
                     sync_hw, &count_tmp);
                 COMPILER_64_ADD_64(count, count_tmp);
#endif
             } else if (SOC_IS_STARFIGHTER3(unit)) {
#if defined(BCM_STARFIGHTER3_SUPPORT)
                 soc_robo_counter_get(unit, port, INDEX(RXPKTS64OCTETSr), 
                     sync_hw, &count);
#endif
             } else {
                 soc_robo_counter_get(unit, port, INDEX(PKTS64OCTETSr), 
                     sync_hw, &count);
             }
             break;
 
         case DRV_SNMP_ETHER_STATS_PKTS65TO127_OCTETS:
             if (SOC_IS_TBX(unit) || SOC_IS_NORTHSTARPLUS(unit)) {
#if defined(BCM_TB_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT)
                 soc_robo_counter_get(unit, port, INDEX(RXPKTS65TO127OCTETSr), 
                     sync_hw, &count_tmp);
                 count = count_tmp;
                 soc_robo_counter_get(unit, port, INDEX(TXPKTS65TO127OCTETSr), 
                     sync_hw, &count_tmp);
                 COMPILER_64_ADD_64(count, count_tmp);
#endif
             } else if (SOC_IS_STARFIGHTER3(unit)) {
#if defined(BCM_STARFIGHTER3_SUPPORT)
                 soc_robo_counter_get(unit, port, INDEX(RXPKTS65TO127OCTETSr), 
                     sync_hw, &count);
#endif
             } else {
                 soc_robo_counter_get(unit, port, INDEX(PKTS65TO127OCTETSr), 
                     sync_hw, &count);
             }
             break;
 
         case DRV_SNMP_ETHER_STATS_PKTS128TO255_OCTETS:
             if (SOC_IS_TBX(unit) || SOC_IS_NORTHSTARPLUS(unit)) {
#if defined(BCM_TB_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT)
                 soc_robo_counter_get(unit, port, INDEX(RXPKTS128TO255OCTETSr), 
                     sync_hw, &count_tmp);
                 count = count_tmp;
                 soc_robo_counter_get(unit, port, INDEX(TXPKTS128TO255OCTETSr), 
                     sync_hw, &count_tmp);
                 COMPILER_64_ADD_64(count, count_tmp);
#endif
             } else if (SOC_IS_STARFIGHTER3(unit)) {
#if defined(BCM_STARFIGHTER3_SUPPORT)
                 soc_robo_counter_get(unit, port, INDEX(RXPKTS128TO255OCTETSr), 
                     sync_hw, &count);
#endif
             } else {
                 soc_robo_counter_get(unit, port, INDEX(PKTS128TO255OCTETSr), 
                     sync_hw, &count);
             }
             break;
 
         case DRV_SNMP_ETHER_STATS_PKTS256TO511_OCTETS:
             if (SOC_IS_TBX(unit) || SOC_IS_NORTHSTARPLUS(unit)) {
#if defined(BCM_TB_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT)
                 soc_robo_counter_get(unit, port, INDEX(RXPKTS256TO511OCTETSr), 
                     sync_hw, &count_tmp);
                 count = count_tmp;
                 soc_robo_counter_get(unit, port, INDEX(TXPKTS256TO511OCTETSr), 
                     sync_hw, &count_tmp);
                 COMPILER_64_ADD_64(count, count_tmp);
#endif
             } else if (SOC_IS_STARFIGHTER3(unit)) {
#if defined(BCM_STARFIGHTER3_SUPPORT)
                 soc_robo_counter_get(unit, port, INDEX(RXPKTS256TO511OCTETSr), 
                     sync_hw, &count);
#endif
             } else {
                 soc_robo_counter_get(unit, port, INDEX(PKTS256TO511OCTETSr), 
                     sync_hw, &count);
             }
             break;
 
         case DRV_SNMP_ETHER_STATS_PKTS512TO1023_OCTETS:
             if (SOC_IS_TBX(unit) || SOC_IS_NORTHSTARPLUS(unit)) {
#if defined(BCM_TB_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT)
                 soc_robo_counter_get(unit, port, INDEX(RXPKTS512TO1023OCTETSr), 
                     sync_hw, &count_tmp);
                 count = count_tmp;
                 soc_robo_counter_get(unit, port, INDEX(TXPKTS512TO1023OCTETSr), 
                     sync_hw, &count_tmp);
                 COMPILER_64_ADD_64(count, count_tmp);
#endif
             } else if (SOC_IS_STARFIGHTER3(unit)) {
#if defined(BCM_STARFIGHTER3_SUPPORT)
                 soc_robo_counter_get(unit, port, INDEX(RXPKTS512TO1023OCTETSr), 
                     sync_hw, &count);
#endif
             } else {
                 soc_robo_counter_get(unit, port, INDEX(PKTS512TO1023OCTETSr), 
                     sync_hw, &count);
             }
             break;
 
         case DRV_SNMP_ETHER_STATS_PKTS1024TO1518_OCTETS:
             /*
              * ROBO chips don't support this range MIBs (Pkts1024to1518Octets)
              * The MIB counter range for ROBO chips are :
              * - 1024 to standard maximum packet size,
              * - 1024 to 1522 packet size.
              */
             return SOC_E_UNAVAIL;
 
         case DRV_SNMP_ETHER_STATS_OVERSIZE_PKTS:
             soc_robo_counter_get(unit, port, INDEX(RXOVERSIZEPKTSr), 
                 sync_hw, &count);
             break;
         case DRV_SNMP_ETHER_RX_OVERSIZE_PKTS:
             soc_robo_counter_get(unit, port, INDEX(RXOVERSIZEPKTSr), 
                 sync_hw, &count);
             break;
         case DRV_SNMP_ETHER_TX_OVERSIZE_PKTS:
             return SOC_E_UNAVAIL;   
             break;
 
         case DRV_SNMP_ETHER_STATS_JABBERS:
             if (SOC_IS_TBX(unit)) {
#ifdef BCM_TB_SUPPORT
                 soc_robo_counter_get(unit, port, INDEX(RXJABBERPKTSr), 
                     sync_hw, &count);
#endif
             } else {
                 soc_robo_counter_get(unit, port, INDEX(RXJABBERSr), 
                     sync_hw, &count);
             }
             break;
 
         case DRV_SNMP_ETHER_STATS_OCTETS:
             soc_robo_counter_get(unit, port, INDEX(RXOCTETSr), 
                 sync_hw, &count_tmp);
             count = count_tmp;
             soc_robo_counter_get(unit, port, INDEX(TXOCTETSr), 
                 sync_hw, &count_tmp);
             COMPILER_64_ADD_64(count, count_tmp);
             break;
 
         case DRV_SNMP_ETHER_STATS_PKTS:
             soc_robo_counter_get(unit, port, INDEX(RXUNICASTPKTSr), 
                 sync_hw, &count_tmp);
             count = count_tmp;
             soc_robo_counter_get(unit, port, INDEX(RXMULTICASTPKTSr), 
                 sync_hw, &count_tmp);
             COMPILER_64_ADD_64(count, count_tmp);
 
             soc_robo_counter_get(unit, port, INDEX(TXUNICASTPKTSr), 
                 sync_hw, &count_tmp);
             COMPILER_64_ADD_64(count, count_tmp);
             soc_robo_counter_get(unit, port, INDEX(TXMULTICASTPKTSr), 
                 sync_hw, &count_tmp);
             COMPILER_64_ADD_64(count, count_tmp);
             soc_robo_counter_get(unit, port, INDEX(TXBROADCASTPKTSr), 
                 sync_hw, &count_tmp);
             COMPILER_64_ADD_64(count, count_tmp);
             soc_robo_counter_get(unit, port, INDEX(TXPAUSEPKTSr), 
                 sync_hw, &count_tmp);
             COMPILER_64_ADD_64(count, count_tmp);
     
             soc_robo_counter_get(unit, port, INDEX(RXALIGNMENTERRORSr), 
                 sync_hw, &count_tmp);
             COMPILER_64_ADD_64(count, count_tmp);
             soc_robo_counter_get(unit, port, INDEX(RXFCSERRORSr), 
                 sync_hw, &count_tmp);
             COMPILER_64_ADD_64(count, count_tmp);
             soc_robo_counter_get(unit, port, INDEX(RXFRAGMENTSr), 
                 sync_hw, &count_tmp);
             COMPILER_64_ADD_64(count, count_tmp);
             soc_robo_counter_get(unit, port, INDEX(RXOVERSIZEPKTSr), 
                 sync_hw, &count_tmp);
             COMPILER_64_ADD_64(count, count_tmp);
             soc_robo_counter_get(unit, port, INDEX(RXPAUSEPKTSr), 
                 sync_hw, &count_tmp);
             COMPILER_64_ADD_64(count, count_tmp);
             soc_robo_counter_get(unit, port, INDEX(RXUNDERSIZEPKTSr), 
                 sync_hw, &count_tmp);
             COMPILER_64_ADD_64(count, count_tmp);
             if (SOC_IS_TBX(unit)) {
#ifdef BCM_TB_SUPPORT
                 soc_robo_counter_get(unit, port, INDEX(RXBROADCASTPKTSr), 
                     sync_hw, &count_tmp);
                 COMPILER_64_ADD_64(count, count_tmp);
                 soc_robo_counter_get(unit, port, INDEX(RXSYMBOLERRr), 
                     sync_hw, &count_tmp);
                 COMPILER_64_ADD_64(count, count_tmp);
                 soc_robo_counter_get(unit, port, INDEX(RXJABBERPKTSr), 
                     sync_hw, &count_tmp);
                 COMPILER_64_ADD_64(count, count_tmp);
#endif
             } else {
                 if (SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
                     SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
                     soc_robo_counter_get(unit, port, INDEX(RXBROADCASTPKTSr), 
                         sync_hw, &count_tmp);
#endif
                 } else {
                     soc_robo_counter_get(unit, port, INDEX(RXBROADCASTPKTr), 
                         sync_hw, &count_tmp);
                 }
                 COMPILER_64_ADD_64(count, count_tmp);
                 soc_robo_counter_get(unit, port, INDEX(RXSYMBLERRr), 
                     sync_hw, &count_tmp);
                 COMPILER_64_ADD_64(count, count_tmp);
                 soc_robo_counter_get(unit, port, INDEX(RXJABBERSr), 
                     sync_hw, &count_tmp);
                 COMPILER_64_ADD_64(count, count_tmp);   
             }        
             break;
 
         case DRV_SNMP_ETHER_STATS_COLLISIONS:
             soc_robo_counter_get(unit, port, INDEX(TXCOLLISIONSr), 
                 sync_hw, &count); 
             break;
 
         case DRV_SNMP_ETHER_STATS_CRC_ALIGN_ERRORS: 
             /* CRC errors + alignment errors */
             soc_robo_counter_get(unit, port, INDEX(RXALIGNMENTERRORSr), 
                 sync_hw, &count_tmp);
             count = count_tmp;
             soc_robo_counter_get(unit, port, INDEX(RXFCSERRORSr), 
                 sync_hw, &count_tmp);
             COMPILER_64_ADD_64(count, count_tmp);
             break;
 
 
         case DRV_SNMP_ETHER_STATS_TX_NO_ERRORS:  
             soc_robo_counter_get(unit, port, INDEX(TXUNICASTPKTSr), 
                 sync_hw, &count_tmp);
             count=count_tmp;
             soc_robo_counter_get(unit, port, INDEX(TXMULTICASTPKTSr), 
                 sync_hw, &count_tmp);
             COMPILER_64_ADD_64(count, count_tmp);
             soc_robo_counter_get(unit, port, INDEX(TXBROADCASTPKTSr), 
                 sync_hw, &count_tmp);
             COMPILER_64_ADD_64(count, count_tmp);
             soc_robo_counter_get(unit, port, INDEX(TXPAUSEPKTSr), 
                 sync_hw, &count_tmp);
             COMPILER_64_ADD_64(count, count_tmp);
             break;    
 
         case DRV_SNMP_ETHER_STATS_RX_NO_ERRORS:  
             soc_robo_counter_get(unit, port, INDEX(RXUNICASTPKTSr), 
                 sync_hw, &count_tmp);
             count=count_tmp;
             soc_robo_counter_get(unit, port, INDEX(RXMULTICASTPKTSr), 
                 sync_hw, &count_tmp);
             COMPILER_64_ADD_64(count, count_tmp);
             soc_robo_counter_get(unit, port, INDEX(RXPAUSEPKTSr), 
                 sync_hw, &count_tmp);
             COMPILER_64_ADD_64(count, count_tmp);
             if (SOC_IS_TBX(unit) || SOC_IS_POLAR(unit) || 
                 SOC_IS_NORTHSTAR(unit) || SOC_IS_NORTHSTARPLUS(unit) ||
                 SOC_IS_STARFIGHTER3(unit)) {
#if defined(BCM_TB_SUPPORT) || defined(BCM_POLAR_SUPPORT) || \
    defined(BCM_NORTHSTAR_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT) || \
    defined(BCM_STARFIGHTER3_SUPPORT)
                 soc_robo_counter_get(unit, port, INDEX(RXBROADCASTPKTSr), 
                     sync_hw, &count_tmp);
#endif
             } else {
                 soc_robo_counter_get(unit, port, INDEX(RXBROADCASTPKTr), 
                     sync_hw, &count_tmp);
             }  
             COMPILER_64_ADD_64(count, count_tmp);
             break;
 
         /* RFC 1643 */        
         case DRV_SNMP_DOT3_STATS_INTERNAL_MAC_RECEIVE_ERRORS:       
             if (SOC_IS_TBX(unit)) {
#ifdef BCM_TB_SUPPORT
                 soc_robo_counter_get(unit, port, INDEX(RXFWDDISCPKTSr), 
                     sync_hw, &count);
#endif
             } else {
                 soc_robo_counter_get(unit, port, INDEX(RXDROPPKTSr), 
                     sync_hw, &count);
             }
             break;
 
         case DRV_SNMP_DOT3_STATS_FRAME_TOO_LONGS:   
             soc_robo_counter_get(unit, port, INDEX(RXOVERSIZEPKTSr), 
                 sync_hw, &count_tmp);
             count = count_tmp;
             if (SOC_IS_TBX(unit)) {
#ifdef BCM_TB_SUPPORT
                 soc_robo_counter_get(unit, port, INDEX(RXJABBERPKTSr), 
                     sync_hw, &count_tmp);
#endif
             } else {
                 soc_robo_counter_get(unit, port, INDEX(RXJABBERSr), 
                     sync_hw, &count_tmp);
             }
             COMPILER_64_ADD_64(count, count_tmp);
             break;        
 
         case DRV_SNMP_DOT3_STATS_ALIGNMENT_ERRORS:  /* *** RFC 2665 *** */
             soc_robo_counter_get(unit, port, INDEX(RXALIGNMENTERRORSr), 
                 sync_hw, &count);
             break;    
 
         case DRV_SNMP_DOT3_STATS_FCS_ERRORS:    /* *** RFC 2665 *** */
             soc_robo_counter_get(unit, port, INDEX(RXFCSERRORSr), 
                 sync_hw, &count);
             break;  
 
         case DRV_SNMP_DOT3_STATS_INTERNAL_MAC_TRANSMIT_ERRORS:  
             if (SOC_IS_TBX(unit)) {
#ifdef BCM_TB_SUPPORT
                 soc_robo_counter_get(unit, port, INDEX(TXPORTCONGESTIONDROPr), 
                     sync_hw, &count);
#endif
             } else {
                 soc_robo_counter_get(unit, port, INDEX(TXDROPPKTSr), 
                     sync_hw, &count);
             }
             break;
 
         case DRV_SNMP_DOT3_STATS_SINGLE_COLLISION_FRAMES:   
             /* *** RFC 2665 *** */
             if (SOC_IS_TBX(unit)) {
#ifdef BCM_TB_SUPPORT
                 soc_robo_counter_get(unit, port, INDEX(TXSINGLECOLLISIONSr), 
                     sync_hw, &count);
#endif
             } else {
                 soc_robo_counter_get(unit, port, INDEX(TXSINGLECOLLISIONr), 
                     sync_hw, &count);
             }
             break;
 
         case DRV_SNMP_DOT3_STATS_MULTIPLE_COLLISION_FRAMES: 
             /* *** RFC 2665 *** */
             if (SOC_IS_TBX(unit)) {
#ifdef BCM_TB_SUPPORT
                 soc_robo_counter_get(unit, port, INDEX(TXMULTIPLECOLLISIONSr), 
                     sync_hw, &count);
#endif
             } else {
                 soc_robo_counter_get(unit, port, INDEX(TXMULTIPLECOLLISIONr), 
                     sync_hw, &count);
             }
             break;
 
         case DRV_SNMP_DOT3_STATS_DEFERRED_TRANSMISSIONS:    
             soc_robo_counter_get(unit, port, INDEX(TXDEFERREDTRANSMITr), 
                 sync_hw, &count);
             break;  
 
         case DRV_SNMP_DOT3_STATS_LATE_COLLISIONS:   
             soc_robo_counter_get(unit, port, INDEX(TXLATECOLLISIONr), 
                 sync_hw, &count);
             break;      
 
         case DRV_SNMP_DOT3_STATS_EXCESSIVE_COLLISIONS:  
             soc_robo_counter_get(unit, port, INDEX(TXEXCESSIVECOLLISIONr), 
                 sync_hw, &count);
             break;      
 
         case DRV_SNMP_DOT3_STATS_CARRIER_SENSE_ERRORS:  
             /* not support for robo */
             return SOC_E_UNAVAIL;
 
         case DRV_SNMP_DOT3_STATS_SQET_TEST_ERRORS:   
             /* not support for robo */   
             return SOC_E_UNAVAIL;                                              
 
             
         /* *** RFC 2665 *** some object same as RFC 1643 */
 
         case DRV_SNMP_DOT3_STATS_SYMBOL_ERRORS:  
             if (SOC_IS_TBX(unit)) {
#ifdef BCM_TB_SUPPORT
                 soc_robo_counter_get(unit, port, INDEX(RXSYMBOLERRr), 
                     sync_hw, &count);
#endif
             } else {
                 soc_robo_counter_get(unit, port, INDEX(RXSYMBLERRr), 
                     sync_hw, &count);
             }
             break;
 
         case DRV_SNMP_DOT3_CONTROL_IN_UNKNOWN_OPCODES:  
             /* not support for robo */   
             return SOC_E_UNAVAIL; 
 
         case DRV_SNMP_DOT3_IN_PAUSE_FRAMES:  
             soc_robo_counter_get(unit, port, INDEX(RXPAUSEPKTSr), 
                 sync_hw, &count);
             break;  
 
         case DRV_SNMP_DOT3_OUT_PAUSE_FRAMES:  
             soc_robo_counter_get(unit, port, INDEX(TXPAUSEPKTSr), 
                 sync_hw, &count);
             break;  
 
         /*** RFC 2233 ***/
         case DRV_SNMP_IF_HC_IN_OCTETS:   
             soc_robo_counter_get(unit, port, INDEX(RXOCTETSr), 
                 sync_hw, &count);
             break;      
 
         case DRV_SNMP_IF_HC_IN_UCAST_PKTS: 
             soc_robo_counter_get(unit, port, INDEX(RXUNICASTPKTSr), 
                 sync_hw, &count);
             break;      
 
         case DRV_SNMP_IF_HC_IN_MULTICAST_PKTS:   
         case DRV_SNMP_IF_IN_MULTICAST_PKTS:   
             soc_robo_counter_get(unit, port, INDEX(RXMULTICASTPKTSr), 
                 sync_hw, &count);
             break;      
 
         case DRV_SNMP_IF_HC_IN_BROADCAST_PKTS:   
         case DRV_SNMP_IF_IN_BROADCAST_PKTS:   
             if (SOC_IS_TBX(unit) || SOC_IS_POLAR(unit) || 
                 SOC_IS_NORTHSTAR(unit) || SOC_IS_NORTHSTARPLUS(unit) ||
                 SOC_IS_STARFIGHTER3(unit)) {
#if defined(BCM_TB_SUPPORT) || defined(BCM_POLAR_SUPPORT) || \
    defined(BCM_NORTHSTAR_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT) || \
    defined(BCM_STARFIGHTER3_SUPPORT)
                 soc_robo_counter_get(unit, port, INDEX(RXBROADCASTPKTSr), 
                     sync_hw, &count);
#endif
             } else {
                 soc_robo_counter_get(unit, port, INDEX(RXBROADCASTPKTr), 
                     sync_hw, &count);
             }  
             break;      
 
         case DRV_SNMP_IF_HC_OUT_OCTETS:  
             soc_robo_counter_get(unit, port, INDEX(TXOCTETSr), 
                 sync_hw, &count);
             break;      
 
         case DRV_SNMP_IF_HC_OUT_UCAST_PKTS:    
             soc_robo_counter_get(unit, port, INDEX(TXUNICASTPKTSr), 
                 sync_hw, &count);
             break;      
 
         case DRV_SNMP_IF_HC_OUT_MULTICAST_PKTS:  
         case DRV_SNMP_IF_OUT_MULTICAST_PKTS:  
             soc_robo_counter_get(unit, port, INDEX(TXMULTICASTPKTSr), 
                 sync_hw, &count);
             break;      
 
         case DRV_SNMP_IF_HC_OUT_BROADCAST_PCKTS:  
         case DRV_SNMP_IF_OUT_BROADCAST_PKTS:  
             soc_robo_counter_get(unit, port, INDEX(TXBROADCASTPKTSr), 
                 sync_hw, &count);
             break;         
 
         /*** RFC 2465 ***/
         case DRV_SNMP_IPV6_IF_STATS_IN_RECEIVES:
         case DRV_SNMP_IPV6_IF_STATS_IN_HDR_ERRORS:
         case DRV_SNMP_IPV6_IF_STATS_IN_ADDR_ERRORS:
         case DRV_SNMP_IPV6_IF_STATS_IN_DISCARDS:
         case DRV_SNMP_IPV6_IF_STATS_OUT_FORW_DATAGRAMS:
         case DRV_SNMP_IPV6_IF_STATS_OUT_DISCARDS:
         case DRV_SNMP_IPV6_IF_STATS_IN_MCAST_PKTS:
         case DRV_SNMP_IPV6_IF_STATS_OUT_MCAST_PKTS:
             /* not support for robo */
             COMPILER_64_ZERO(*val);
             return SOC_E_UNAVAIL;
 
         /* IEEE 802.1bb */
         case DRV_SNMP_IEEE8021_PFC_REQUESTS:
         case DRV_SNMP_IEEE8021_PFC_INDICATIONS:
             /* not support for robo */
             COMPILER_64_ZERO(*val);
             return SOC_E_UNAVAIL;
 
         /*** Additional Broadcom stats ***/
         case DRV_SNMP_BCM_IPMC_BRIDGED_PCKTS:
         case DRV_SNMP_BCM_IPMC_ROUTED_PCKTS:    
         case DRV_SNMP_BCM_IPMC_IN_DROPPED_PCKTS:
         case DRV_SNMP_BCM_IPMC_OUT_DROPPED_PCKTS:
             /* not support for robo */
             COMPILER_64_ZERO(*val);
             return SOC_E_UNAVAIL;
 
         case DRV_SNMP_BCM_ETHER_STATS_PKTS1519TO1522_OCTETS:
             /* not support for robo */
             COMPILER_64_ZERO(*val);
             return SOC_E_UNAVAIL;
         case DRV_SNMP_BCM_ETHER_STATS_PKTS1522TO2047_OCTETS:
             COMPILER_64_ZERO(*val);
             return SOC_E_UNAVAIL;
         case DRV_SNMP_BCM_ETHER_STATS_PKTS2048TO4095_OCTETS:
             if (SOC_IS_DINO8(unit)) {
#ifdef BCM_DINO8_SUPPORT
                 soc_robo_counter_get(unit, port, INDEX(PKTS2048TO4095r), 
                     sync_hw, &count);    
                 break;
#endif /* BCM_DINO8_SUPPORT */
             } else {
                 COMPILER_64_ZERO(*val);
                 return SOC_E_UNAVAIL;
             }
         case DRV_SNMP_BCM_ETHER_STATS_PKTS4095TO9216_OCTETS:
             /* not support for robo */
             COMPILER_64_ZERO(*val);
             return SOC_E_UNAVAIL;
 
         case DRV_SNMP_BCM_RECEIVED_PKTS64_OCTETS:
             if (SOC_IS_TBX(unit) || SOC_IS_NORTHSTARPLUS(unit) ||
                 SOC_IS_STARFIGHTER3(unit)) {
#if defined(BCM_TB_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT) || \
             defined(BCM_STARFIGHTER3_SUPPORT)
                 soc_robo_counter_get(unit, port, INDEX(RXPKTS64OCTETSr), 
                     sync_hw, &count);
                 break;
#endif
             } else {
                 COMPILER_64_ZERO(*val);
                 return SOC_E_UNAVAIL;
             }
 
         case DRV_SNMP_BCM_RECEIVED_PKTS65TO127_OCTETS:
             if (SOC_IS_TBX(unit) || SOC_IS_NORTHSTARPLUS(unit) ||
                 SOC_IS_STARFIGHTER3(unit)) {
#if defined(BCM_TB_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT) || \
             defined(BCM_STARFIGHTER3_SUPPORT)
                 soc_robo_counter_get(unit, port, INDEX(RXPKTS65TO127OCTETSr), 
                     sync_hw, &count);
                 break;
#endif
             } else {
                 COMPILER_64_ZERO(*val);
                 return SOC_E_UNAVAIL;
             }
 
         case DRV_SNMP_BCM_RECEIVED_PKTS128TO255_OCTETS:
             if (SOC_IS_TBX(unit) || SOC_IS_NORTHSTARPLUS(unit) ||
                 SOC_IS_STARFIGHTER3(unit)) {
#if defined(BCM_TB_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT) || \
             defined(BCM_STARFIGHTER3_SUPPORT)
                 soc_robo_counter_get(unit, port, INDEX(RXPKTS128TO255OCTETSr), 
                     sync_hw, &count);
                 break;
#endif
             } else {
                 COMPILER_64_ZERO(*val);
                 return SOC_E_UNAVAIL;
             }
 
         case DRV_SNMP_BCM_RECEIVED_PKTS256TO511_OCTETS:
             if (SOC_IS_TBX(unit) || SOC_IS_NORTHSTARPLUS(unit) ||
                 SOC_IS_STARFIGHTER3(unit)) {
#if defined(BCM_TB_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT) || \
             defined(BCM_STARFIGHTER3_SUPPORT)
                 soc_robo_counter_get(unit, port, INDEX(RXPKTS256TO511OCTETSr), 
                     sync_hw, &count);
                 break;
#endif
             } else {
                 COMPILER_64_ZERO(*val);
                 return SOC_E_UNAVAIL;
             }
 
         case DRV_SNMP_BCM_RECEIVED_PKTS512TO1023_OCTETS:
             if (SOC_IS_TBX(unit) || SOC_IS_NORTHSTARPLUS(unit) ||
                 SOC_IS_STARFIGHTER3(unit)) {
#if defined(BCM_TB_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT) || \
             defined(BCM_STARFIGHTER3_SUPPORT)
                 soc_robo_counter_get(unit, port, INDEX(RXPKTS512TO1023OCTETSr), 
                     sync_hw, &count);
                 break;
#endif
             } else {
                 COMPILER_64_ZERO(*val);
                 return SOC_E_UNAVAIL;
             }
 
         case DRV_SNMP_BCM_RECEIVED_PKTS1024TO1518_OCTETS:
         case DRV_SNMP_BCM_RECEIVED_PKTS1519TO2047_OCTETS:
         case DRV_SNMP_BCM_RECEIVED_PKTS2048TO4095_OCTETS:
         case DRV_SNMP_BCM_RECEIVED_PKTS4095TO9216_OCTETS:
             /* not support for robo */
             COMPILER_64_ZERO(*val);
             return SOC_E_UNAVAIL;
 
         case DRV_SNMP_BCM_TRANSMITTED_PKTS64_OCTETS:
             if (SOC_IS_TBX(unit) || SOC_IS_NORTHSTARPLUS(unit) ||
                 SOC_IS_STARFIGHTER3(unit)) {
#if defined(BCM_TB_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT) || \
             defined(BCM_STARFIGHTER3_SUPPORT)
                 soc_robo_counter_get(unit, port, INDEX(TXPKTS64OCTETSr), 
                     sync_hw, &count);
                 break;
#endif
             } else {
                 COMPILER_64_ZERO(*val);
                 return SOC_E_UNAVAIL;
             }
 
         case DRV_SNMP_BCM_TRANSMITTED_PKTS65TO127_OCTETS:
             if (SOC_IS_TBX(unit) || SOC_IS_NORTHSTARPLUS(unit) ||
                 SOC_IS_STARFIGHTER3(unit)) {
#if defined(BCM_TB_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT) || \
             defined(BCM_STARFIGHTER3_SUPPORT)
                 soc_robo_counter_get(unit, port, INDEX(TXPKTS65TO127OCTETSr), 
                     sync_hw, &count);
                 break;
#endif
             } else {
                 COMPILER_64_ZERO(*val);
                 return SOC_E_UNAVAIL;
             }
 
         case DRV_SNMP_BCM_TRANSMITTED_PKTS128TO255_OCTETS:
             if (SOC_IS_TBX(unit) || SOC_IS_NORTHSTARPLUS(unit) ||
                 SOC_IS_STARFIGHTER3(unit)) {
#if defined(BCM_TB_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT) || \
             defined(BCM_STARFIGHTER3_SUPPORT)
                 soc_robo_counter_get(unit, port, INDEX(TXPKTS128TO255OCTETSr), 
                     sync_hw, &count);
                 break;
#endif
             } else {
                 COMPILER_64_ZERO(*val);
                 return SOC_E_UNAVAIL;
             }
 
         case DRV_SNMP_BCM_TRANSMITTED_PKTS256TO511_OCTETS:
             if (SOC_IS_TBX(unit) || SOC_IS_NORTHSTARPLUS(unit) ||
                 SOC_IS_STARFIGHTER3(unit)) {
#if defined(BCM_TB_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT) || \
             defined(BCM_STARFIGHTER3_SUPPORT)
                 soc_robo_counter_get(unit, port, INDEX(TXPKTS256TO511OCTETSr), 
                     sync_hw, &count);
                 break;
#endif
             } else {
                 COMPILER_64_ZERO(*val);
                 return SOC_E_UNAVAIL;
             }
 
         case DRV_SNMP_BCM_TRANSMITTED_PKTS512TO1023_OCTETS:
             if (SOC_IS_TBX(unit) || SOC_IS_NORTHSTARPLUS(unit) ||
                 SOC_IS_STARFIGHTER3(unit)) {
#if defined(BCM_TB_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT) || \
             defined(BCM_STARFIGHTER3_SUPPORT)
                 soc_robo_counter_get(unit, port, INDEX(TXPKTS512TO1023OCTETSr), 
                     sync_hw, &count);
                 break;
#endif
             } else {
                 COMPILER_64_ZERO(*val);
                 return SOC_E_UNAVAIL;
             }
 
         case DRV_SNMP_BCM_TRANSMITTED_PKTS1024TO1518_OCTETS:
         case DRV_SNMP_BCM_TRANSMITTED_PKTS1519TO2047_OCTETS:
         case DRV_SNMP_BCM_TRANSMITTED_PKTS2048TO4095_OCTETS:
         case DRV_SNMP_BCM_TRANSMITTED_PKTS4095TO9216_OCTETS:
             /* not support for robo */
             COMPILER_64_ZERO(*val);
             return SOC_E_UNAVAIL;
 
         case DRV_SNMP_BCM_CUSTOM_RECEIVE0:
         case DRV_SNMP_BCM_CUSTOM_RECEIVE1:
         case DRV_SNMP_BCM_CUSTOM_RECEIVE2:
         case DRV_SNMP_BCM_CUSTOM_RECEIVE3:
         case DRV_SNMP_BCM_CUSTOM_RECEIVE4:
         case DRV_SNMP_BCM_CUSTOM_RECEIVE5:
         case DRV_SNMP_BCM_CUSTOM_RECEIVE6:
         case DRV_SNMP_BCM_CUSTOM_RECEIVE7:
         case DRV_SNMP_BCM_CUSTOM_RECEIVE8:
         case DRV_SNMP_BCM_CUSTOM_TRANSMIT0:
         case DRV_SNMP_BCM_CUSTOM_TRANSMIT1:
         case DRV_SNMP_BCM_CUSTOM_TRANSMIT2:
         case DRV_SNMP_BCM_CUSTOM_TRANSMIT3:
         case DRV_SNMP_BCM_CUSTOM_TRANSMIT4:
         case DRV_SNMP_BCM_CUSTOM_TRANSMIT5:
         case DRV_SNMP_BCM_CUSTOM_TRANSMIT6:
         case DRV_SNMP_BCM_CUSTOM_TRANSMIT7:
         case DRV_SNMP_BCM_CUSTOM_TRANSMIT8:
         case DRV_SNMP_BCM_CUSTOM_TRANSMIT9:
         case DRV_SNMP_BCM_CUSTOM_TRANSMIT10:
         case DRV_SNMP_BCM_CUSTOM_TRANSMIT11:
         case DRV_SNMP_BCM_CUSTOM_TRANSMIT12:
         case DRV_SNMP_BCM_CUSTOM_TRANSMIT13:
         case DRV_SNMP_BCM_CUSTOM_TRANSMIT14:
             /* not support for robo */
             return SOC_E_UNAVAIL;
             
         default:
             LOG_VERBOSE(BSL_LS_SOC_COMMON,
                         (BSL_META_U(unit,
                                     "drv_snmp_counter_get: Statistic not supported: %d\n"), counter_type));
             return SOC_E_PARAM;
     }
 
     *val = count;
 
     return SOC_E_NONE;
 }
 
 /*
  * Function:
  *      drv_counter_reset
  * Purpose:
  *      Reset the MIB counters
  * Parameters:
  *      unit    - RoboSwitch unit number.
  * Returns:
  *      SOC_E_XXX.
  */
 int
 drv_counter_reset(int unit)
 {
     int     rv = SOC_E_NONE;
     uint32  reg_value, temp;
     int     i;
     soc_pbmp_t  pbmp;
     uint64 val64;
 
     if (soc_feature(unit, soc_feature_no_stat_mib)) {
         return SOC_E_UNAVAIL;
     }
 
     if (SOC_IS_HARRIER(unit)){
#ifdef BCM_HARRIER_SUPPORT
         /* read control setting */                
         SOC_IF_ERROR_RETURN(REG_READ_RST_TABLE_MEMr
             (unit, &reg_value));
 
         temp = 1;
         SOC_IF_ERROR_RETURN(soc_RST_TABLE_MEMr_field_set
             (unit, &reg_value, RST_MIB_CNTf, &temp));
 
         SOC_IF_ERROR_RETURN(REG_WRITE_RST_TABLE_MEMr
             (unit, &reg_value));
 
         /* wait for complete */
         for (i = 0; i < SOC_TIMEOUT_VAL; i++) {
             SOC_IF_ERROR_RETURN(REG_READ_RST_TABLE_MEMr
                 (unit, &reg_value));
             SOC_IF_ERROR_RETURN(soc_RST_TABLE_MEMr_field_get
                 (unit, &reg_value, RST_MIB_CNTf, &temp));
             if (!temp) {
                 break;
             }
         }
         if (i >= SOC_TIMEOUT_VAL) {
             return SOC_E_TIMEOUT;
         }
#endif
     } else if (SOC_IS_TBX(unit)) {
#ifdef BCM_TB_SUPPORT
         SOC_IF_ERROR_RETURN(REG_READ_RST_TABLE_MEMr
             (unit, &reg_value));
 
         temp = 1;
         SOC_IF_ERROR_RETURN(soc_RST_TABLE_MEMr_field_set
             (unit, &reg_value, RST_MIB_CNTf, &temp));
 
         SOC_IF_ERROR_RETURN(REG_WRITE_RST_TABLE_MEMr
             (unit, &reg_value));
 
         /* wait for complete */
         for (i = 0; i < SOC_TIMEOUT_VAL; i++) {
             SOC_IF_ERROR_RETURN(REG_READ_RST_TABLE_MEMr
                 (unit, &reg_value));
             SOC_IF_ERROR_RETURN(soc_RST_TABLE_MEMr_field_get
                 (unit, &reg_value, RST_MIB_CNTf, &temp));
             if (!temp) {
                 break;
             }
         }
     
         if (i >= SOC_TIMEOUT_VAL) {
             return SOC_E_TIMEOUT;
         }
#endif
     } else {
#ifdef BCM_NORTHSTARPLUS_SUPPORT
         if (SOC_IS_NORTHSTARPLUS(unit)) {
             /* Enable all port mib clear function */
             reg_value = 0x1ff;
             SOC_IF_ERROR_RETURN(
                REG_WRITE_RST_MIB_CNT_ENr(unit, &reg_value));
         }
#endif /* BCM_NORTHSTARPLUS_SUPPORT */
         SOC_IF_ERROR_RETURN(REG_READ_GMNGCFGr
             (unit, &reg_value));
 
         temp = 1;
         SOC_IF_ERROR_RETURN(soc_GMNGCFGr_field_set
             (unit, &reg_value, RST_MIB_CNTf, &temp));
         
         SOC_IF_ERROR_RETURN(REG_WRITE_GMNGCFGr
             (unit, &reg_value));
     
         for (i = 0; i < 10000; i++) {
             temp = i;
         }
         temp = 0;
         SOC_IF_ERROR_RETURN(soc_GMNGCFGr_field_set
             (unit, &reg_value, RST_MIB_CNTf, &temp));
         
         SOC_IF_ERROR_RETURN(REG_WRITE_GMNGCFGr
             (unit, &reg_value));
     }
 
     /* clear SW counter table*/
     SOC_PBMP_CLEAR(pbmp);
     SOC_PBMP_ASSIGN(pbmp, PBMP_ALL(unit));
     COMPILER_64_ZERO(val64);
     _soc_robo_counter_sw_table_set(unit, pbmp, val64);
 
     return rv;
 }


