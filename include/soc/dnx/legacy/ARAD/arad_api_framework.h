/* $Id: jer2_arad_api_framework.h,v 1.167 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __JER2_ARAD_API_FRAMEWORK_H_INCLUDED__
/* { */
#define __JER2_ARAD_API_FRAMEWORK_H_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */
#include <soc/dnx/legacy/SAND/Utils/sand_header.h>

#include <soc/dnx/legacy/SAND/Management/sand_chip_descriptors.h>
#include <soc/dnx/legacy/SAND/Management/sand_error_code.h>
#include <soc/dnx/legacy/SAND/Management/sand_general_macros.h>
#include <soc/dnx/legacy/SAND/Utils/sand_os_interface.h>
/* } */

/*************
 * DEFINES   *
 *************/
/* { */

/*
 * Procedure identifiers.
 * {
 */

#define JER2_ARAD_REGISTER_DEVICE                                                       (   0|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_UNREGISTER_DEVICE                                                     (   1|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MGMT_INIT_SEQUENCE_FIXES_APPLY                                        (   2|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MGMT_CREDIT_WORTH_SET                                                 (   7|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MGMT_CREDIT_WORTH_GET                                                 (   8|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MGMT_CREDIT_WORTH_VERIFY                                              (   9|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_CALC_ASSIGNED_REBOUNDED_CREDIT_CONF                                   (  20|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MGMT_CREDIT_WORTH_GET_UNSAFE                                          (  21|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MGMT_INIT_SEQUENCE_PHASE1                                             (  22|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MGMT_FAP_ID_SET                                                       (  23|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MGMT_FAP_ID_GET                                                       (  24|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MGMT_FAP_ID_VERIFY                                                    (  25|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MGMT_FAP_ID_SET_UNSAFE                                                (  26|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MGMT_FAP_ID_GET_UNSAFE                                                (  27|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MGMT_INIT_SEQUENCE_PHASE2                                             (  28|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MGMT_HW_INTERFACES_SET                                                (  29|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MGMT_HW_INTERFACES_GET                                                (  40|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MGMT_HW_INTERFACES_VERIFY                                             (  41|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MGMT_HW_INTERFACES_SET_UNSAFE                                         (  42|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MGMT_HW_INTERFACES_GET_UNSAFE                                         (  43|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MGMT_ALL_CTRL_CELLS_ENABLE_SET                                        (  44|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MGMT_ALL_CTRL_CELLS_ENABLE_GET                                        (  45|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MGMT_ALL_CTRL_CELLS_ENABLE_VERIFY                                     (  46|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MGMT_ALL_CTRL_CELLS_ENABLE_SET_UNSAFE                                 (  47|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MGMT_ALL_CTRL_CELLS_ENABLE_GET_UNSAFE                                 (  48|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MGMT_ENABLE_TRAFFIC_SET                                               (  49|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MGMT_ENABLE_TRAFFIC_GET                                               (  50|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_CHIP_TIME_TO_TICKS                                                    (  51|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_CHIP_TICKS_TO_TIME                                                    (  52|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MGMT_DEVICE_INIT                                                      (  53|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MGMT_ENABLE_TRAFFIC_VERIFY                                            (  61|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MGMT_ENABLE_TRAFFIC_SET_UNSAFE                                        (  62|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MGMT_ENABLE_TRAFFIC_GET_UNSAFE                                        (  63|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_SERDES_LINKS_PARAM_SET                                            (  64|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_SERDES_LINKS_PARAM_GET                                            (  65|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_SERDES_LINKS_PARAM_VERIFY                                         (  66|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_SERDES_LINKS_PARAM_SET_UNSAFE                                     (  67|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_SERDES_LINKS_PARAM_GET_UNSAFE                                     (  68|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_SERDES_LINKS_STATUS_GET                                           (  69|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_LINKS_CONF_SET                                                    (  80|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_LINKS_CONF_GET                                                    (  81|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_LINKS_CONF_VERIFY                                                 (  82|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_LINKS_CONF_SET_UNSAFE                                             (  83|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_LINKS_CONF_GET_UNSAFE                                             (  84|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_XAUI_CONF_SET                                                     (  85|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_XAUI_CONF_GET                                                     (  86|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_XAUI_CONF_VERIFY                                                  (  87|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_XAUI_CONF_SET_UNSAFE                                              (  88|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_XAUI_CONF_GET_UNSAFE                                              (  89|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_SPAUI_CONF_SET                                                    ( 100|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_SPAUI_CONF_GET                                                    ( 101|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_SPAUI_CONF_VERIFY                                                 ( 102|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_SPAUI_CONF_SET_UNSAFE                                             ( 103|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_SPAUI_CONF_GET_UNSAFE                                             ( 104|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_SGMII_CONF_SET                                                    ( 105|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_SGMII_CONF_GET                                                    ( 106|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_SGMII_CONF_VERIFY                                                 ( 107|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_SGMII_CONF_SET_UNSAFE                                             ( 108|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_SGMII_CONF_GET_UNSAFE                                             ( 109|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_OC768C_CONF_SET                                                   ( 120|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_OC768C_CONF_GET                                                   ( 121|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_OC768C_CONF_VERIFY                                                ( 122|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_OC768C_CONF_SET_UNSAFE                                            ( 123|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_OC768C_CONF_GET_UNSAFE                                            ( 124|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_FLOW_CONTROL_CONF_SET                                             ( 125|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_FLOW_CONTROL_CONF_GET                                             ( 126|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_FLOW_CONTROL_CONF_VERIFY                                          ( 127|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_FLOW_CONTROL_CONF_SET_UNSAFE                                      ( 128|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_FLOW_CONTROL_CONF_GET_UNSAFE                                      ( 129|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_LINKS_ENABLE_SET                                                  ( 140|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_LINKS_ENABLE_GET                                                  ( 141|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_LINKS_ENABLE_VERIFY                                               ( 142|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_LINKS_ENABLE_SET_UNSAFE                                           ( 143|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_LINKS_ENABLE_GET_UNSAFE                                           ( 144|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_DIAG_LAST_PACKET_GET                                              ( 146|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_FAP_PORTS_TO_NIF_PORTS_SET                                        ( 147|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_FAP_PORTS_TO_NIF_PORTS_GET                                        ( 148|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_FAP_PORTS_TO_NIF_PORTS_VERIFY                                     ( 149|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_DIAG_LAST_PACKET_GET_UNSAFE                                       ( 150|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_FAP_PORTS_TO_NIF_PORTS_SET_UNSAFE                                 ( 160|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_FAP_PORTS_TO_NIF_PORTS_GET_UNSAFE                                 ( 161|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_FAP_PORTS_TO_RCY_PORTS_SET                                        ( 162|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_FAP_PORTS_TO_RCY_PORTS_GET                                        ( 163|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_FAP_PORTS_TO_RCY_PORTS_VERIFY                                     ( 164|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_FAP_PORTS_TO_RCY_PORTS_SET_UNSAFE                                 ( 165|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_FAP_PORTS_TO_RCY_PORTS_GET_UNSAFE                                 ( 166|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_FAP_PORTS_TO_CPU_PORTS_SET                                        ( 167|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_FAP_PORTS_TO_CPU_PORTS_GET                                        ( 168|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_FAP_PORTS_TO_CPU_PORTS_VERIFY                                     ( 169|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_FAP_PORTS_TO_CPU_PORTS_SET_UNSAFE                                 ( 180|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_FAP_PORTS_TO_CPU_PORTS_GET_UNSAFE                                 ( 181|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PORT_OPTIONS_SET                                                      ( 182|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PORT_OPTIONS_GET                                                      ( 183|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PORT_OPTIONS_VERIFY                                                   ( 184|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PORT_OPTIONS_SET_UNSAFE                                               ( 185|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PORT_OPTIONS_GET_UNSAFE                                               ( 186|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PORT_RAW_SWITCHING_CONF_SET                                           ( 187|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PORT_RAW_SWITCHING_CONF_GET                                           ( 188|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PORT_RAW_SWITCHING_CONF_VERIFY                                        ( 189|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PORT_RAW_SWITCHING_CONF_SET_UNSAFE                                    ( 200|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PORT_RAW_SWITCHING_CONF_GET_UNSAFE                                    ( 201|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PORT_STATISTICS_TAG_CONF_SET                                          ( 202|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PORT_STATISTICS_TAG_CONF_GET                                          ( 203|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PORT_STATISTICS_TAG_CONF_VERIFY                                       ( 204|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PORT_STATISTICS_TAG_CONF_SET_UNSAFE                                   ( 205|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PORT_STATISTICS_TAG_CONF_GET_UNSAFE                                   ( 206|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_CPU_IF_PORT_CONF_SET                                                  ( 207|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_CPU_IF_PORT_CONF_GET                                                  ( 208|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_CPU_IF_PORT_CONF_VERIFY                                               ( 209|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_CPU_IF_PORT_CONF_SET_UNSAFE                                           ( 220|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_CPU_IF_PORT_CONF_GET_UNSAFE                                           ( 221|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGRESS_HEADER_SET                                                     ( 228|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGRESS_HEADER_GET                                                     ( 229|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGRESS_HEADER_VERIFY                                                  ( 240|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGRESS_HEADER_SET_UNSAFE                                              ( 241|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGRESS_HEADER_GET_UNSAFE                                              ( 242|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MIRROR_INGRESS_PORT_SET                                               ( 243|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MIRROR_INGRESS_PORT_GET                                               ( 244|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MIRROR_INGRESS_PORT_VERIFY                                            ( 245|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MIRROR_INGRESS_PORT_SET_UNSAFE                                        ( 246|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MIRROR_INGRESS_PORT_GET_UNSAFE                                        ( 247|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MIRROR_EGRESS_PORT_SET                                                ( 248|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MIRROR_EGRESS_PORT_GET                                                ( 249|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MIRROR_EGRESS_PORT_VERIFY                                             ( 260|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MIRROR_EGRESS_PORT_SET_UNSAFE                                         ( 261|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MIRROR_EGRESS_PORT_GET_UNSAFE                                         ( 262|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPQ_EXPLICIT_MAPPING_MODE_INFO_SET                                    ( 263|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPQ_EXPLICIT_MAPPING_MODE_INFO_GET                                    ( 264|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPQ_EXPLICIT_MAPPING_MODE_INFO_VERIFY                                 ( 265|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPQ_EXPLICIT_MAPPING_MODE_INFO_SET_UNSAFE                             ( 266|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPQ_EXPLICIT_MAPPING_MODE_INFO_GET_UNSAFE                             ( 267|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPQ_TRAFFIC_CLASS_MAP_SET                                             ( 268|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPQ_TRAFFIC_CLASS_MAP_GET                                             ( 269|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPQ_TRAFFIC_CLASS_MAP_VERIFY                                          ( 280|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPQ_TRAFFIC_CLASS_MAP_SET_UNSAFE                                      ( 281|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPQ_TRAFFIC_CLASS_MAP_GET_UNSAFE                                      ( 282|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPQ_TRAFFIC_CLASS_MULTICAST_PRIORITY_MAP_SET                          ( 283|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPQ_TRAFFIC_CLASS_MULTICAST_PRIORITY_MAP_SET_VERIFY                   ( 284|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPQ_TRAFFIC_CLASS_MULTICAST_PRIORITY_MAP_SET_UNSAFE                   ( 285|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPQ_TRAFFIC_CLASS_MULTICAST_PRIORITY_MAP_GET                          ( 286|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPQ_TRAFFIC_CLASS_MULTICAST_PRIORITY_MAP_GET_VERIFY                   ( 287|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPQ_TRAFFIC_CLASS_MULTICAST_PRIORITY_MAP_GET_UNSAFE                   ( 288|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPQ_DESTINATION_ID_PACKETS_BASE_QUEUE_ID_SET                          ( 289|JER2_ARAD_PROC_BITS) 
#define JER2_ARAD_IPQ_DESTINATION_ID_PACKETS_BASE_QUEUE_ID_GET                          ( 300|JER2_ARAD_PROC_BITS) 
#define JER2_ARAD_IPQ_DESTINATION_ID_PACKETS_BASE_QUEUE_ID_VERIFY                       ( 301|JER2_ARAD_PROC_BITS) 
#define JER2_ARAD_IPQ_DESTINATION_ID_PACKETS_BASE_QUEUE_ID_SET_UNSAFE                   ( 302|JER2_ARAD_PROC_BITS) 
#define JER2_ARAD_IPQ_DESTINATION_ID_PACKETS_BASE_QUEUE_ID_GET_UNSAFE                   ( 303|JER2_ARAD_PROC_BITS) 
#define JER2_ARAD_IPQ_QUEUE_INTERDIGITATED_MODE_SET                                     ( 304|JER2_ARAD_PROC_BITS) 
#define JER2_ARAD_IPQ_QUEUE_INTERDIGITATED_MODE_GET                                     ( 305|JER2_ARAD_PROC_BITS) 
#define JER2_ARAD_IPQ_QUEUE_INTERDIGITATED_MODE_VERIFY                                  ( 306|JER2_ARAD_PROC_BITS) 
#define JER2_ARAD_IPQ_QUEUE_INTERDIGITATED_MODE_SET_UNSAFE                              ( 307|JER2_ARAD_PROC_BITS) 
#define JER2_ARAD_IPQ_QUEUE_INTERDIGITATED_MODE_GET_UNSAFE                              ( 308|JER2_ARAD_PROC_BITS) 
#define JER2_ARAD_IPQ_QUEUE_TO_FLOW_MAPPING_SET                                         ( 309|JER2_ARAD_PROC_BITS) 
#define JER2_ARAD_IPQ_QUEUE_TO_FLOW_MAPPING_GET                                         ( 310|JER2_ARAD_PROC_BITS) 
#define JER2_ARAD_IPQ_QUEUE_TO_FLOW_MAPPING_VERIFY                                      ( 311|JER2_ARAD_PROC_BITS) 
#define JER2_ARAD_IPQ_QUEUE_TO_FLOW_MAPPING_SET_UNSAFE                                  ( 312|JER2_ARAD_PROC_BITS) 
#define JER2_ARAD_IPQ_QUEUE_TO_FLOW_MAPPING_GET_UNSAFE                                  ( 313|JER2_ARAD_PROC_BITS) 
#define JER2_ARAD_IPQ_QUARTET_RESET                                                     ( 314|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPQ_QUARTET_RESET_VERIFY                                              ( 315|JER2_ARAD_PROC_BITS)
/*#define JER2_ARAD_IPQ_EXPLICIT_MAPPING_MODE_INFO_SET_UNSAFE                                ( 316|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPQ_EXPLICIT_MAPPING_MODE_INFO_GET_UNSAFE                                ( 317|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPQ_TRAFFIC_CLASS_MAP_SET_UNSAFE                                         ( 318|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPQ_TRAFFIC_CLASS_MAP_GET_UNSAFE                                         ( 319|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPQ_DESTINATION_ID_PACKETS_BASE_QUEUE_ID_SET_UNSAFE                      ( 320|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPQ_DESTINATION_ID_PACKETS_BASE_QUEUE_ID_GET_UNSAFE                      ( 321|JER2_ARAD_PROC_BITS)
*/
#define JER2_ARAD_IPQ_QUARTET_RESET_UNSAFE                                              ( 329|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPQ_K_QUARTET_RESET                                                   ( 330|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPQ_K_QUARTET_RESET_VERIFY                                            ( 331|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPQ_K_QUARTET_RESET_UNSAFE                                            ( 332|JER2_ARAD_PROC_BITS)

#define JER2_ARAD_IPQ_SYS_PHYSICAL_TO_DEST_PORT_SET                                     ( 339|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPQ_SYS_PHYSICAL_TO_DEST_PORT_GET                                     ( 340|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPQ_SYS_PHYSICAL_TO_DEST_PORT_VERIFY                                  ( 341|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPQ_SYS_PHYSICAL_TO_DEST_PORT_SET_UNSAFE                              ( 342|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPQ_SYS_PHYSICAL_TO_DEST_PORT_GET_UNSAFE                              ( 343|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPQ_LOCAL_TO_SYSTEM_PORT_SET                                          ( 344|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPQ_LOCAL_TO_SYSTEM_PORT_GET                                          ( 345|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPQ_LOCAL_TO_SYSTEM_PORT_VERIFY                                       ( 346|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPQ_LOCAL_TO_SYSTEM_PORT_SET_UNSAFE                                   ( 347|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPQ_LOCAL_TO_SYSTEM_PORT_GET_UNSAFE                                   ( 348|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPQ_LAG_GLOBAL_INFO_SET                                               ( 349|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPQ_LAG_GLOBAL_INFO_GET                                               ( 360|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPQ_LAG_GLOBAL_INFO_VERIFY                                            ( 361|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPQ_LAG_GLOBAL_INFO_SET_UNSAFE                                        ( 362|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPQ_LAG_GLOBAL_INFO_GET_UNSAFE                                        ( 363|JER2_ARAD_PROC_BITS)

#define JER2_ARAD_PORTS_LAG_ORDER_PRESERVE_SET                                          ( 369|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PORTS_LAG_ORDER_PRESERVE_GET                                          ( 380|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PORTS_LAG_ORDER_PRESERVE_VERIFY                                       ( 381|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PORTS_LAG_ORDER_PRESERVE_SET_UNSAFE                                   ( 382|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PORTS_LAG_ORDER_PRESERVE_GET_UNSAFE                                   ( 383|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PORTS_LAG_MODE_SET                                                      ( 384|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PORTS_LAG_MODE_GET                                                      ( 385|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PORTS_LAG_NOF_LAG_ENTRIES_GET_UNSAFE                                  ( 386|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PORTS_LAG_NOF_LAG_GROUPS_GET_UNSAFE                                   ( 387|JER2_ARAD_PROC_BITS)

#define JER2_ARAD_ITM_CATEGORY_RNGS_SET                                                 ( 389|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_CATEGORY_RNGS_GET                                                 ( 400|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_CATEGORY_RNGS_VERIFY                                              ( 401|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_CATEGORY_RNGS_SET_UNSAFE                                          ( 402|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_CATEGORY_RNGS_GET_UNSAFE                                          ( 403|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_ADMIT_TEST_TMPLT_SET                                              ( 404|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_ADMIT_TEST_TMPLT_GET                                              ( 405|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_ADMIT_TEST_TMPLT_VERIFY                                           ( 406|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_ADMIT_TEST_TMPLT_SET_UNSAFE                                       ( 407|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_ADMIT_TEST_TMPLT_GET_UNSAFE                                       ( 408|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_CR_REQUEST_SET                                                    ( 409|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_CR_REQUEST_GET                                                    ( 420|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_CR_REQUEST_VERIFY                                                 ( 421|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_CR_REQUEST_SET_UNSAFE                                             ( 422|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_CR_REQUEST_GET_UNSAFE                                             ( 423|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_CR_DISCOUNT_SET                                                   ( 424|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_CR_DISCOUNT_GET                                                   ( 425|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_CR_DISCOUNT_VERIFY                                                ( 426|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_CR_DISCOUNT_SET_UNSAFE                                            ( 427|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_CR_DISCOUNT_GET_UNSAFE                                            ( 428|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_WRED_EXP_WQ_SET                                                   ( 429|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_WRED_EXP_WQ_GET                                                   ( 430|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_WRED_EXP_WQ_SET_UNSAFE                                            ( 431|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_WRED_EXP_WQ_VERIFY                                                ( 432|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_WRED_EXP_WQ_GET_UNSAFE                                            ( 433|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_TAIL_DROP_SET                                                     ( 434|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_TAIL_DROP_GET                                                     ( 435|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_TAIL_DROP_SET_UNSAFE                                              ( 436|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_TAIL_DROP_GET_UNSAFE                                              ( 437|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_TAIL_DROP_VERIFY                                                  ( 438|JER2_ARAD_PROC_BITS)

#define JER2_ARAD_ITM_WRED_SET                                                          ( 444|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_WRED_GET                                                          ( 445|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_WRED_SET_UNSAFE                                                   ( 446|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_WRED_VERIFY                                                       ( 447|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_WRED_GET_UNSAFE                                                   ( 448|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_CR_WD_SET                                                         ( 449|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_CR_WD_GET                                                         ( 460|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_CR_WD_VERIFY                                                      ( 461|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_CR_WD_SET_UNSAFE                                                  ( 462|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_CR_WD_GET_UNSAFE                                                  ( 463|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_VSQ_QT_RT_CLS_SET                                                 ( 464|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_VSQ_QT_RT_CLS_GET                                                 ( 465|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_VSQ_QT_RT_CLS_VERIFY                                              ( 466|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_VSQ_QT_RT_CLS_SET_UNSAFE                                          ( 467|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_VSQ_QT_RT_CLS_GET_UNSAFE                                          ( 468|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_VSQ_FC_SET                                                        ( 469|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_VSQ_FC_GET                                                        ( 480|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_VSQ_FC_VERIFY                                                     ( 481|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_VSQ_FC_SET_UNSAFE                                                 ( 482|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_VSQ_FC_GET_UNSAFE                                                 ( 483|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_VSQ_WRED_SET                                                      ( 484|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_VSQ_WRED_GET                                                      ( 485|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_VSQ_WRED_VERIFY                                                   ( 486|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_VSQ_WRED_SET_UNSAFE                                               ( 487|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_VSQ_WRED_GET_UNSAFE                                               ( 488|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_STAG_SET                                                          ( 489|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_STAG_GET                                                          ( 500|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_STAG_VERIFY                                                       ( 501|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_STAG_SET_UNSAFE                                                   ( 502|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_STAG_GET_UNSAFE                                                   ( 503|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_QUEUE_INFO_SET                                                    ( 504|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_QUEUE_INFO_GET                                                    ( 505|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_QUEUE_INFO_VERIFY                                                 ( 506|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_QUEUE_INFO_SET_UNSAFE                                             ( 507|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_QUEUE_INFO_GET_UNSAFE                                             ( 508|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_INGRESS_SHAPE_SET                                                 ( 509|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_DYN_TOTAL_THRESH_SET                                              ( 510|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_DYN_TOTAL_THRESH_SET_UNSAFE                                       ( 511|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_INGRESS_SHAPE_GET                                                 ( 520|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_INGRESS_SHAPE_VERIFY                                              ( 521|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_INGRESS_SHAPE_SET_UNSAFE                                          ( 522|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_INGRESS_SHAPE_GET_UNSAFE                                          ( 523|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_PRIORITY_MAP_TMPLT_SET                                            ( 524|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_PRIORITY_MAP_TMPLT_GET                                            ( 525|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_PRIORITY_MAP_TMPLT_VERIFY                                         ( 526|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_PRIORITY_MAP_TMPLT_SET_UNSAFE                                     ( 527|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_PRIORITY_MAP_TMPLT_GET_UNSAFE                                     ( 528|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_PRIORITY_MAP_TMPLT_SELECT_SET                                     ( 529|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_PRIORITY_MAP_TMPLT_SELECT_GET                                     ( 540|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_PRIORITY_MAP_TMPLT_SELECT_VERIFY                                  ( 541|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_PRIORITY_MAP_TMPLT_SELECT_SET_UNSAFE                              ( 542|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_PRIORITY_MAP_TMPLT_SELECT_GET_UNSAFE                              ( 543|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_SYS_RED_DROP_PROB_SET                                             ( 544|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_SYS_RED_DROP_PROB_GET                                             ( 545|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_SYS_RED_DROP_PROB_VERIFY                                          ( 546|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_SYS_RED_DROP_PROB_SET_UNSAFE                                      ( 547|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_SYS_RED_DROP_PROB_GET_UNSAFE                                      ( 548|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_SYS_RED_QUEUE_SIZE_BOUNDARIES_SET                                 ( 549|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_SYS_RED_QUEUE_SIZE_BOUNDARIES_GET                                 ( 558|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_SYS_RED_QUEUE_SIZE_BOUNDARIES_VERIFY                              ( 559|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_SYS_RED_QUEUE_SIZE_BOUNDARIES_SET_UNSAFE                          ( 560|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_SYS_RED_QUEUE_SIZE_BOUNDARIES_GET_UNSAFE                          ( 561|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_SYS_RED_Q_BASED_SET                                               ( 562|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_SYS_RED_Q_BASED_GET                                               ( 563|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_SYS_RED_Q_BASED_VERIFY                                            ( 564|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_SYS_RED_Q_BASED_SET_UNSAFE                                        ( 565|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_SYS_RED_Q_BASED_GET_UNSAFE                                        ( 566|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_SYS_RED_EG_SET                                                    ( 567|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_SYS_RED_EG_GET                                                    ( 568|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_SYS_RED_EG_VERIFY                                                 ( 569|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_SYS_RED_EG_SET_UNSAFE                                             ( 570|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_SYS_RED_EG_GET_UNSAFE                                             ( 571|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_SYS_RED_GLOB_RCS_SET                                              ( 572|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_SYS_RED_GLOB_RCS_GET                                              ( 573|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_SYS_RED_GLOB_RCS_VERIFY                                           ( 574|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_SYS_RED_GLOB_RCS_SET_UNSAFE                                       ( 575|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_SYS_RED_GLOB_RCS_GET_UNSAFE                                       ( 576|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_QUEUE_TEST_TMPLT_SET                                              ( 577|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_QUEUE_TEST_TMPLT_GET                                              ( 578|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_QUEUE_TEST_TMPLT_SET_UNSAFE                                       ( 579|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_QUEUE_TEST_TMPLT_VERIFY                                           ( 580|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_QUEUE_TEST_TMPLT_GET_UNSAFE                                       ( 581|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_VSQ_TAIL_DROP_SET                                                 ( 582|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_VSQ_TAIL_DROP_GET                                                 ( 583|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_VSQ_TAIL_DROP_DEFAULT_GET                                         ( 584|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_VSQ_TAIL_DROP_SET_UNSAFE                                          ( 585|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_VSQ_TAIL_DROP_GET_UNSAFE                                          ( 586|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_VSQ_TAIL_DROP_VERIFY                                              ( 587|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_VSQ_TAIL_DROP_DEFAULT_GET_UNSAFE                                  ( 588|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_VSQ_WRED_GEN_SET                                                  ( 589|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_VSQ_WRED_GEN_GET                                                  ( 590|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_VSQ_WRED_GEN_SET_UNSAFE                                           ( 591|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_VSQ_WRED_GEN_VERIFY                                               ( 592|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_VSQ_WRED_GEN_GET_UNSAFE                                           ( 593|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_GLOB_RCS_FC_SET                                                   ( 594|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_GLOB_RCS_FC_GET                                                   ( 595|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_GLOB_RCS_DROP_SET                                                 ( 596|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_GLOB_RCS_DROP_GET                                                 ( 597|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_GLOB_RCS_FC_SET_UNSAFE                                            ( 598|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_GLOB_RCS_FC_VERIFY                                                ( 599|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_GLOB_RCS_FC_GET_UNSAFE                                            ( 600|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_GLOB_RCS_DROP_SET_UNSAFE                                          ( 601|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_GLOB_RCS_DROP_VERIFY                                              ( 602|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_GLOB_RCS_DROP_GET_UNSAFE                                          ( 603|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_INGRESS_SCHEDULER_MESH_SET                                            ( 611|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_INGRESS_SCHEDULER_MESH_GET                                            ( 612|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_INGRESS_SCHEDULER_MESH_VERIFY                                         ( 613|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_INGRESS_SCHEDULER_MESH_SET_UNSAFE                                     ( 614|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_INGRESS_SCHEDULER_MESH_GET_UNSAFE                                     ( 615|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_INGRESS_SCHEDULER_CLOS_SET                                            ( 616|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_INGRESS_SCHEDULER_CLOS_GET                                            ( 617|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_INGRESS_SCHEDULER_CLOS_VERIFY                                         ( 618|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_INGRESS_SCHEDULER_CLOS_SET_UNSAFE                                     ( 619|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_INGRESS_SCHEDULER_CLOS_GET_UNSAFE                                     ( 620|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGRESS_SCH_QUEUE_TYPE_SET                                             ( 621|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGRESS_SCH_QUEUE_TYPE_GET                                             ( 622|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGRESS_SCH_QUEUE_TYPE_VERIFY                                          ( 623|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGRESS_SCH_QUEUE_TYPE_SET_UNSAFE                                      ( 624|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGRESS_SCH_QUEUE_TYPE_GET_UNSAFE                                      ( 625|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGRESS_QUEUES_CONF_SET                                                ( 626|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGRESS_QUEUES_CONF_GET                                                ( 627|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGRESS_QUEUES_CONF_VERIFY                                             ( 628|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGRESS_QUEUES_CONF_SET_UNSAFE                                         ( 629|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGRESS_QUEUES_CONF_GET_UNSAFE                                         ( 630|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_DEVICE_RATE_ENTRY_SET                                             ( 631|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_DEVICE_RATE_ENTRY_GET                                             ( 640|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_DEVICE_RATE_ENTRY_VERIFY                                          ( 641|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_DEVICE_RATE_ENTRY_SET_UNSAFE                                      ( 642|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_DEVICE_RATE_ENTRY_GET_UNSAFE                                      ( 643|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_EGRESS_PORTS_QOS_SET                                              ( 669|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_EGRESS_PORTS_QOS_GET                                              ( 680|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_EGRESS_PORTS_QOS_VERIFY                                           ( 681|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_EGRESS_PORTS_QOS_SET_UNSAFE                                       ( 682|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_EGRESS_PORTS_QOS_GET_UNSAFE                                       ( 683|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_ONE_EGRESS_PORT_QOS_SET                                           ( 684|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_ONE_EGRESS_PORT_QOS_GET                                           ( 685|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_ONE_EGRESS_PORT_QOS_VERIFY                                        ( 686|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_ONE_EGRESS_PORT_QOS_SET_UNSAFE                                    ( 687|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_ONE_EGRESS_PORT_QOS_GET_UNSAFE                                    ( 688|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_CLASS_TYPE_PARAMS_SET                                             ( 689|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_CLASS_TYPE_PARAMS_GET                                             ( 700|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_CLASS_TYPE_PARAMS_VERIFY                                          ( 701|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_CLASS_TYPE_PARAMS_SET_UNSAFE                                      ( 702|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_CLASS_TYPE_PARAMS_GET_UNSAFE                                      ( 703|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_CLASS_TYPE_PARAMS_TABLE_SET                                       ( 704|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_CLASS_TYPE_PARAMS_TABLE_GET                                       ( 705|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_CLASS_TYPE_PARAMS_TABLE_VERIFY                                    ( 706|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_CLASS_TYPE_PARAMS_TABLE_SET_UNSAFE                                ( 707|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_CLASS_TYPE_PARAMS_TABLE_GET_UNSAFE                                ( 708|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_SLOW_MAX_RATES_SET                                                ( 709|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_SLOW_MAX_RATES_GET                                                ( 720|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_SLOW_MAX_RATES_VERIFY                                             ( 721|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_SLOW_MAX_RATES_SET_UNSAFE                                         ( 722|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_SLOW_MAX_RATES_GET_UNSAFE                                         ( 723|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_PORT_SET                                                          ( 724|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_PORT_GET                                                          ( 725|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_PORT_VERIFY                                                       ( 726|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_PORT_SET_UNSAFE                                                   ( 727|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_PORT_GET_UNSAFE                                                   ( 728|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_PORT_SCHED_VERIFY                                                 ( 729|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_PORT_SCHED_SET_UNSAFE                                             ( 730|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_PORT_SCHED_GET_UNSAFE                                             ( 731|JER2_ARAD_PROC_BITS)

#define JER2_ARAD_SCH_AGGREGATE_SET                                                     ( 749|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_AGGREGATE_GROUP_SET                                               ( 750|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_AGGREGATE_GET                                                     ( 760|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_AGGREGATE_VERIFY                                                  ( 761|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_AGGREGATE_SET_UNSAFE                                              ( 762|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_AGGREGATE_GET_UNSAFE                                              ( 763|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_FLOW_DELETE_UNSAFE                                                ( 764|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_FLOW_DELETE                                                       ( 765|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_AGGREGATE_GROUP_SET_UNSAFE                                        ( 766|JER2_ARAD_PROC_BITS)

#define JER2_ARAD_MULT_ING_TRAFFIC_CLASS_MAP_SET                                        ( 800|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_ING_TRAFFIC_CLASS_MAP_GET                                        ( 801|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_ING_TRAFFIC_CLASS_MAP_VERIFY                                     ( 802|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_ING_TRAFFIC_CLASS_MAP_SET_UNSAFE                                 ( 803|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_ING_TRAFFIC_CLASS_MAP_GET_UNSAFE                                 ( 804|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_ING_GROUP_OPEN                                                   ( 805|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_ING_GROUP_UPDATE                                                 ( 806|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_ING_GROUP_CLOSE                                                  ( 807|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_ING_DESTINATION_ADD                                              ( 808|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_ING_DESTINATION_REMOVE                                           ( 809|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_ING_GROUP_SIZE_GET                                               ( 810|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_ING_GET_GROUP                                                    ( 811|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_ING_ALL_GROUPS_CLOSE                                             ( 812|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_DOES_GROUP_EXIST                                                 ( 813|JER2_ARAD_PROC_BITS)

#define JER2_ARAD_MULT_CUD_TO_PORT_MAP_SET                                              ( 814|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_CUD_TO_PORT_MAP_GET                                              ( 815|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_CUD_TO_PORT_MAP_SET_UNSAFE                                       ( 816|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_CUD_TO_PORT_MAP_GET_UNSAFE                                       ( 817|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_CUD_TO_PORT_MAP_SET_VERIFY                                       ( 818|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_CUD_TO_PORT_MAP_GET_VERIFY                                       ( 819|JER2_ARAD_PROC_BITS)

#define JER2_ARAD_MULT_ING_GROUP_OPEN_UNSAFE                                            ( 820|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_ING_GROUP_UPDATE_UNSAFE                                          ( 821|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_ING_GROUP_CLOSE_UNSAFE                                           ( 822|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_ING_DESTINATION_ADD_UNSAFE                                       ( 823|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_ING_DESTINATION_REMOVE_UNSAFE                                    ( 824|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_ING_GROUP_SIZE_GET_UNSAFE                                        ( 825|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_ING_GET_GROUP_UNSAFE                                             ( 826|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_ING_ALL_GROUPS_CLOSE_UNSAFE                                      ( 827|JER2_ARAD_PROC_BITS)

#define JER2_ARAD_MULT_EG_VLAN_MEMBERSHIP_MULTICAST_ID_NDX_VERIFY                       ( 828|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_EG_VLAN_MEMBERSHIP_GROUP_RANGE_SET                               ( 829|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_EG_VLAN_MEMBERSHIP_GROUP_RANGE_GET                               ( 830|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_EG_VLAN_MEMBERSHIP_GROUP_RANGE_VERIFY                            ( 831|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_EG_VLAN_MEMBERSHIP_GROUP_RANGE_SET_UNSAFE                        ( 832|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_EG_VLAN_MEMBERSHIP_GROUP_RANGE_GET_UNSAFE                        ( 833|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_EG_GROUP_SET                                                     ( 839|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_EG_GROUP_UPDATE                                                  ( 840|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_EG_GROUP_CLOSE                                                   ( 841|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_EG_PORT_ADD                                                      ( 842|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_EG_PORT_REMOVE                                                   ( 843|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_EG_GROUP_SIZE_GET                                                ( 844|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_EG_GET_GROUP                                                     ( 845|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_EG_ALL_GROUPS_CLOSE                                              ( 846|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_EG_VLAN_MEMBERSHIP_GROUP_OPEN                                    ( 847|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_EG_VLAN_MEMBERSHIP_GROUP_UPDATE                                  ( 848|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_EG_VLAN_MEMBERSHIP_GROUP_CLOSE                                   ( 849|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_EG_VLAN_MEMBERSHIP_PORT_ADD                                      ( 850|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_EG_VLAN_MEMBERSHIP_PORT_REMOVE                                   ( 851|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_EG_VLAN_MEMBERSHIP_GROUP_GET                                     ( 852|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_EG_VLAN_MEMBERSHIP_ALL_GROUPS_CLOSE                              ( 853|JER2_ARAD_PROC_BITS)

#define JER2_ARAD_MULT_EG_GROUP_SET_UNSAFE                                              ( 854|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_EG_GROUP_UPDATE_UNSAFE                                           ( 855|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_EG_GROUP_CLOSE_UNSAFE                                            ( 856|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_EG_PORT_ADD_UNSAFE                                               ( 857|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_EG_PORT_REMOVE_UNSAFE                                            ( 858|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_EG_GROUP_SIZE_GET_UNSAFE                                         ( 859|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_EG_GET_GROUP_UNSAFE                                              ( 860|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_EG_ALL_GROUPS_CLOSE_UNSAFE                                       ( 861|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_EG_VLAN_MEMBERSHIP_GROUP_OPEN_UNSAFE                             ( 862|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_EG_VLAN_MEMBERSHIP_GROUP_UPDATE_UNSAFE                           ( 863|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_EG_VLAN_MEMBERSHIP_GROUP_CLOSE_UNSAFE                            ( 864|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_EG_VLAN_MEMBERSHIP_PORT_ADD_UNSAFE                               ( 865|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_EG_VLAN_MEMBERSHIP_PORT_REMOVE_UNSAFE                            ( 866|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_EG_VLAN_MEMBERSHIP_GROUP_GET_UNSAFE                              ( 867|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_EG_VLAN_MEMBERSHIP_ALL_GROUPS_CLOSE_UNSAFE                       ( 868|JER2_ARAD_PROC_BITS)

#define JER2_ARAD_MULT_FABRIC_TRAFFIC_CLASS_TO_MULTICAST_CLS_MAP_SET                    ( 869|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_FABRIC_TRAFFIC_CLASS_TO_MULTICAST_CLS_MAP_GET                    ( 870|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_FABRIC_TRAFFIC_CLASS_TO_MULTICAST_CLS_MAP_VERIFY                 ( 871|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_FABRIC_TRAFFIC_CLASS_TO_MULTICAST_CLS_MAP_SET_UNSAFE             ( 872|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_FABRIC_TRAFFIC_CLASS_TO_MULTICAST_CLS_MAP_GET_UNSAFE             ( 873|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_FABRIC_BASE_QUEUE_SET                                            ( 874|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_FABRIC_BASE_QUEUE_GET                                            ( 875|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_FABRIC_BASE_QUEUE_VERIFY                                         ( 876|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_FABRIC_BASE_QUEUE_SET_UNSAFE                                     ( 877|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_FABRIC_BASE_QUEUE_GET_UNSAFE                                     ( 878|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_FABRIC_CREDIT_SOURCE_SET                                         ( 879|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_FABRIC_CREDIT_SOURCE_GET                                         ( 880|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_FABRIC_CREDIT_SOURCE_SET_UNSAFE                                  ( 881|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_FABRIC_CREDIT_SOURCE_GET_UNSAFE                                  ( 882|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_FABRIC_ACTIVE_LINKS_SET                                          ( 883|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_FABRIC_ACTIVE_LINKS_GET                                          ( 884|JER2_ARAD_PROC_BITS)

#define JER2_ARAD_FABRIC_LINKS_SERDES_PARAM_SET                                         ( 885|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FABRIC_LINKS_SERDES_PARAM_GET                                         ( 886|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FABRIC_LINKS_SERDES_PARAM_VERIFY                                      ( 887|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FABRIC_LINKS_SERDES_PARAM_SET_UNSAFE                                  ( 888|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FABRIC_LINKS_SERDES_PARAM_GET_UNSAFE                                  ( 889|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FABRIC_LINKS_LOGIC_CONF_SET                                           ( 890|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FABRIC_LINKS_LOGIC_CONF_GET                                           ( 891|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FABRIC_LINKS_LOGIC_CONF_VERIFY                                        ( 892|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FABRIC_LINKS_LOGIC_CONF_SET_UNSAFE                                    ( 893|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FABRIC_LINKS_LOGIC_CONF_GET_UNSAFE                                    ( 894|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FABRIC_MODE_SET                                                       ( 895|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FABRIC_MODE_GET                                                       ( 896|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FABRIC_MODE_VERIFY                                                    ( 897|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FABRIC_MODE_SET_UNSAFE                                                ( 898|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FABRIC_MODE_GET_UNSAFE                                                ( 899|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FABRIC_STAND_ALONE_FAP_MODE_SET                                       ( 900|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FABRIC_STAND_ALONE_FAP_MODE_GET                                       ( 920|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FABRIC_STAND_ALONE_FAP_MODE_VERIFY                                    ( 921|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FABRIC_STAND_ALONE_FAP_MODE_SET_UNSAFE                                ( 922|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FABRIC_STAND_ALONE_FAP_MODE_GET_UNSAFE                                ( 923|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FABRIC_STAND_ALONE_FAP_MODE_DETECT                                    ( 924|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_INTERRUPT_HANDLER                                                     ( 925|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_STATISTICS_IF_INFO_SET                                                ( 926|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_STATISTICS_IF_INFO_GET                                                ( 927|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_STATISTICS_IF_INFO_VERIFY                                             ( 928|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_STATISTICS_IF_INFO_SET_UNSAFE                                         ( 929|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_STATISTICS_IF_INFO_GET_UNSAFE                                         ( 940|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SEND_SR_DATA_CELL                                                     ( 941|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_RECV_SR_DATA_CELL                                                     ( 942|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SEND_CONTROL_CELL                                                     ( 943|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PACKET_SEND                                                           ( 944|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PACKET_RECV                                                           ( 945|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SSR_GET_BUFF_SIZE                                                     ( 949|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SSR_TO_BUFF                                                           ( 960|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SSR_FROM_BUFF                                                         ( 961|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_REGISTER_CALLBACK_FUNCTION                                            ( 962|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_UNREGISTER_CALLBACK_FUNCTION                                          ( 963|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_UNREGISTER_ALL_CALLBACK_FUNCTIONS                                     ( 964|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_VERIFY_ALL                                                        (1154|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_LANE_STATUS_GET_UNSAFE                                            (1158|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_LANE_TX_SET_UNSAFE                                                (1159|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_LANE_TX_VERIFY                                                    (1160|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_LANE_TX_GET_UNSAFE                                                (1161|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_LANE_POLARITY_SET_UNSAFE                                          (1162|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_LANE_POLARITY_VERIFY                                              (1163|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_LANE_POLARITY_GET_UNSAFE                                          (1164|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_LANE_DIVISOR_VERIFY                                               (1165|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_QUARTET_CMU_SET_UNSAFE                                            (1166|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_QUARTET_CMU_VERIFY                                                (1167|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_QUARTET_CMU_GET_UNSAFE                                            (1168|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_LANE_STATUS_INFO_GET                                              (1169|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_LANE_POLARITY_SET                                                 (1171|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_LANE_POLARITY_GET                                                 (1172|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_QUARTET_SET_UNSAFE                                                (1173|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_QUARTET_VERIFY                                                    (1174|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_QUARTET_GET_UNSAFE                                                (1175|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_SET_ALL_UNSAFE                                                    (1176|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_GET_ALL_UNSAFE                                                    (1177|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_PRINT_ALL_UNSAFE                                                  (1178|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_LANE_SYNC_UNSAFE                                                  (1179|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_REF_CLOCK_SET                                                     (1180|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_REF_CLOCK_GET                                                     (1181|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_QUARTET_CMU_SET                                                   (1182|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_QUARTET_CMU_GET                                                   (1183|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_SET_ALL                                                           (1184|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_GET_ALL                                                           (1185|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_PRINT_ALL                                                         (1186|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_LANE_SYNC                                                         (1187|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DEST_SYS_PORT_INFO_VERIFY                                             (1188|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DEST_INFO_VERIFY                                                      (1189|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_IF_SHAPER_RATE_SET                                                (1190|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_IF_SHAPER_RATE_GET                                                (1191|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_DEVICE_IF_WEIGHT_IDX_SET                                          (1192|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_DEVICE_IF_WEIGHT_IDX_GET                                          (1193|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_IF_WEIGHT_CONF_SET                                                (1194|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_IF_WEIGHT_CONF_GET                                                (1195|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_IF_SHAPER_RATE_VERIFY                                             (1196|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_DEVICE_IF_WEIGHT_IDX_SET_UNSAFE                                   (1197|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_DEVICE_IF_WEIGHT_IDX_VERIFY                                       (1198|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_DEVICE_IF_WEIGHT_IDX_GET_UNSAFE                                   (1199|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_IF_WEIGHT_CONF_SET_UNSAFE                                         (1200|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_IF_WEIGHT_CONF_VERIFY                                             (1201|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_IF_WEIGHT_CONF_GET_UNSAFE                                         (1202|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_INTERFACE_ID_VERIFY                                                   (1203|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SW_DB_DEVICE_SCH_INITIALIZE                                           (1204|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SW_DB_DEVICE_INIT                                                     (1205|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SW_DB_DEVICE_CLOSE                                                    (1206|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SW_DB_INIT                                                            (1207|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FAP_PORT_ID_VERIFY                                                    (1208|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DROP_PRECEDENCE_VERIFY                                                (1209|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_TRAFFIC_CLASS_VERIFY                                                  (1210|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SW_DB_DEV_EGR_PORTS_INITIALIZE                                        (1211|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SW_DB_MULTICAST_INITIALIZE                                            (1212|JER2_ARAD_PROC_BITS)

#define JER2_ARAD_SRD_LANE_REG_WRITE                                                    (1213|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_LANE_REG_WRITE_UNSAFE                                             (1214|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_LANE_REG_READ                                                     (1215|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_LANE_REG_READ_UNSAFE                                              (1216|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_QUARTET_RATE_FACTORS_SET                                          (1217|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_QUARTET_RATE_FACTORS_SET_UNSAFE                                   (1218|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_QUARTET_RATE_FACTORS_GET                                          (1219|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_QUARTET_RATE_FACTORS_VERIFY                                       (1220|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_QUARTET_RATE_FACTORS_GET_UNSAFE                                   (1221|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_LANE_STATE_SET                                                    (1222|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_LANE_STATE_SET_UNSAFE                                             (1223|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_LANE_STATE_GET                                                    (1224|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_LANE_STATE_VERIFY                                                 (1225|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_LANE_STATE_GET_UNSAFE                                             (1226|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_LANE_DIVISOR_SET                                                  (1227|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_LANE_DIVISOR_SET_UNSAFE                                           (1228|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_LANE_DIVISOR_GET                                                  (1229|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_RATE_DIVISOR_VERIFY                                               (1230|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_LANE_DIVISOR_GET_UNSAFE                                           (1231|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_LANE_PHYSICAL_PARAMS_SET                                          (1232|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_LANE_PHYSICAL_PARAMS_SET_UNSAFE                                   (1233|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_LANE_PHYSICAL_PARAMS_GET                                          (1234|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_LANE_PHYSICAL_PARAMS_VERIFY                                       (1235|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_LANE_PHYSICAL_PARAMS_GET_UNSAFE                                   (1236|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_LANE_TX_SET                                                       (1237|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_LANE_TX_GET                                                       (1238|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_IPU_REG_WRITE                                                     (1239|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_IPU_REG_WRITE_UNSAFE                                              (1240|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_IPU_REG_READ                                                      (1241|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_IPU_REG_READ_UNSAFE                                               (1242|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_IPU_INIT                                                          (1243|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_IPU_INIT_UNSAFE                                                   (1244|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_CMU_REG_WRITE                                                     (1245|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_CMU_REG_WRITE_UNSAFE                                              (1246|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_CMU_REG_READ                                                      (1247|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_CMU_REG_READ_UNSAFE                                               (1248|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_RATE_FACTORS_CALCULATE                                            (1249|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_RATE_FACTORS_CALCULATE_UNSAFE                                     (1250|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_LANE_PHYSICAL_INFO_CALCULATE                                      (1251|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_LANE_PHYSICAL_INFO_CALCULATE_UNSAFE                               (1252|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_LINK_RX_EYE_MONITOR                                               (1253|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_LINK_RX_EYE_MONITOR_UNSAFE                                        (1254|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_LANE_LOOPBACK_MODE_SET                                            (1255|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_LANE_LOOPBACK_MODE_SET_UNSAFE                                     (1256|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_LANE_LOOPBACK_MODE_GET                                            (1257|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_LANE_LOOPBACK_MODE_VERIFY                                         (1258|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_LANE_LOOPBACK_MODE_GET_UNSAFE                                     (1259|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_LANE_SET                                                          (1262|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_LANE_SET_UNSAFE                                                   (1263|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_LANE_GET                                                          (1264|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_LANE_GET_UNSAFE                                                   (1265|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_LANE_VERIFY                                                       (1266|JER2_ARAD_PROC_BITS)

#define JER2_ARAD_SRD_REGS_FIELD_FROM_REG_SET                                           (1270|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_REGS_FIELD_FROM_REG_GET                                           (1271|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_REGS_WRITE_REG_UNSAFE                                             (1272|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_REGS_WRITE_REG                                                    (1273|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_REGS_READ_REG                                                     (1274|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_REGS_WRITE_FLD                                                    (1275|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_REGS_READ_FLD                                                     (1276|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_REGS_SET_DO_PRINTS                                                (1277|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_REGS_SCIF_CNTRL                                                   (1278|JER2_ARAD_PROC_BITS)

#define JER2_ARAD_SRD_LLA_RDWR_VERIFY                                                   (1279|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_LLA_SET_DO_PRINTS                                                 (1280|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_LLA_WRITE                                                         (1281|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_LLA_READ                                                          (1282|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_LLA_EJER2_ARAD_BITSTREAM_TO_EJER2_ARAD_CMD                                      (1283|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_LLA_EJER2_ARAD_EJER2_ARAD_CMD_TO_BITSTREAM                                      (1284|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_LLA_EJER2_ARAD_CMD_WRITE                                                 (1285|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_LLA_EJER2_ARAD_CMD_READ                                                  (1286|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_LLA_EJER2_ARAD_WRITE                                                     (1287|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_LLA_EJER2_ARAD_READ                                                      (1288|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_AQFM_AGG_ACTUAL_ID_GET                                                (1289|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_AQFM_AGGS_DEFAULT_GET                                                 (1290|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_AQFM_AGG_INFO_ITEM_SET                                                (1291|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_AQFM_AGG_INFO_ITEM_GET                                                (1292|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_AQFM_AGGS_ACTUAL_IDS_SET                                              (1293|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_AQFM_ALL_AGGREGATES_OPEN_UNSAFE                                       (1294|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_AQFM_CLOSE_PORT_AGGS                                                  (1295|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_AQFM_UPDATE_PORT_AGGS                                                 (1296|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_AQFM_AGG_PORT_SCHEME_AGGREGATES_PRINT                                 (1297|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_AQFM_FLOWS_GET_DEFAULT                                                (1298|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_AQFM_FLOW_SET_PHYSICAL_IDS                                            (1299|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_AQFM_FLOW_PORT_FLOWS_OPEN                                             (1301|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_AQFM_CLOSE_PORT_FLOWS                                                 (1302|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_AQFM_UPDATE_PORT_FLOWS                                                (1303|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_AQFM_ALL_FLOWS_OPEN_UNSAFE                                            (1304|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_API_AUTO_PRINT_PORT_SCHEME_FLOWS                                      (1305|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_AQFM_SYSTEM_INFO_SAVE                                                 (1306|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_AQFM_SYSTEM_INFO_DEFAULTS_GET                                         (1307|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_AQFM_ALL_PORTS_OPEN                                                   (1309|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_AQFM_ALL_AGGREGATES_OPEN                                              (1310|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_AQFM_ALL_QUEUES_OPEN                                                  (1311|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_AQFM_ALL_FLOWS_OPEN                                                   (1312|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_AQFM_PORT_OPEN                                                        (1313|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_AQFM_PORT_UPDATE                                                      (1314|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_AQFM_PORT_CLOSE                                                       (1315|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_AQFM_AGG_PORT_AGGS_OPEN                                               (1316|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_AQFM_AGG_PORT_AGGS_CLOSE                                              (1317|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_AQFM_AGG_PORT_AGGS_UPDATE                                             (1318|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_AQFM_AGG_PORT_SINGLE_AGGS_UPDATE                                      (1319|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_AQFM_QUEUES_OPEN                                                      (1320|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_AQFM_QUEUES_UPDATE                                                    (1321|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_AQFM_QUEUES_CLOSE                                                     (1322|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_AQFM_FLOW_OPEN                                                        (1323|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_AQFM_FLOW_UPDATE                                                      (1324|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_AQFM_FLOW_CLOSE                                                       (1325|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_AQFM_SYSTEM_INFO_SAVE_UNSAFE                                          (1326|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_AQFM_PORT_FIRST_RELATIVE_ID_GET                                       (1327|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_AQFM_PORT_ACTUAL2RELATIVE_GET                                         (1328|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_AQFM_QUEUE_INFO_DEFAULT_GET                                           (1329|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_AQFM_PORT_RELATIVE2ACTUAL_GET                                         (1330|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_AQFM_AGG_RELATIVE_ID_GET                                              (1331|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_AQFM_QUEUE_ID_GET                                                     (1332|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_AQFM_FLOW_ID_GET                                                      (1333|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_AQFM_SYSTEM_PHYSICAL_PORT_ID_GET                                      (1334|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_AQFM_DESTINATION_ID_GET                                               (1335|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_AQFM_AGG_BASE_IDS_GET                                                 (1336|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_AQFM_QUEUE_PORT_SCHEME_QUEUES_PRINT                                   (1337|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_AQFM_ALL_QUEUES_OPEN_UNSAFE                                           (1338|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_AQFM_OPEN_PORT_AGGS                                                   (1339|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_AQFM_PORT_DEFAULT_GET                                                 (1340|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_AQFM_PORT_SCHEDULER_PORT_DEFAULT_GET                                  (1341|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_AQFM_ALL_PORTS_OPEN_UNSAFE                                            (1342|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_AQFM_PORT_UPDATE_UNSAFE                                               (1343|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_AQFM_PORT_PORT_SCHEME_PORT_PRINT                                      (1344|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_AQFM_GET_CREDIT_SOURCES                                               (1345|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_AQFM_NOF_RELATIVE_PORT_GET                                            (1346|JER2_ARAD_PROC_BITS)

#define JER2_ARAD_SW_DB_AUTO_SCHEME_INITIALIZE                                          (1350|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PKT_PACKET_SEND_UNSAFE                                                (1351|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PKT_PACKET_RECV_UNSAFE                                                (1352|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PKT_PACKET_SEND                                                       (1353|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PKT_PACKET_RECV                                                       (1354|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PKT_PACKET_TX_LOAD_MEM                                                (1355|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PKT_PACKET_TX_LOAD_MEM_UNSAFE                                         (1356|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_STAT_PKT_COUNTERS_COLLECT                                             (1357|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_STAT_COUNTER_GET                                                      (1358|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_STAT_ALL_COUNTERS_GET                                                 (1359|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_STAT_PKT_COUNTERS_COLLECT_UNSAFE                                      (1362|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_STAT_COUNTER_GET_UNSAFE                                               (1363|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_STAT_ALL_COUNTERS_GET_UNSAFE                                          (1364|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_COUNTER_TO_DEVICE_COUNTERS_ADD                                        (1367|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_STAT_STATISTICS_MODULE_INITIALIZE                                     (1368|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_STAT_STATISTICS_DEVICE_INITIALIZE                                     (1369|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_STAT_STATISTICS_DEFERRED_COUNTER_CLEAR                                (1370|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_STAT_COUNTER_INFO_GET                                                 (1371|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_STAT_PKT_CNT_CALLBACK_GET                                             (1372|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_TODO_ERR_DEF                                                          (1399|JER2_ARAD_PROC_BITS)
/*
 * jer2_arad_reg_access.c
 * jer2_arad_tbl_access.c
 */
#define JER2_ARAD_READ_FLD                                                              (1400|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_WRITE_FLD                                                             (1401|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_READ_REG                                                              (1402|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_WRITE_REG                                                             (1403|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_READ_FLD_UNSAFE                                                       (1404|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_WRITE_FLD_UNSAFE                                                      (1405|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_READ_REG_UNSAFE                                                       (1406|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_WRITE_REG_UNSAFE                                                      (1407|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_GET_BLKN_TBLN_ENTRY                                                   (1408|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_GET_BLKN_TBLN_ENTRY_UNSAFE                                            (1409|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_WRITE_REG_BUFFER_UNSAFE                                               (1410|JER2_ARAD_PROC_BITS)

/*
 * jer2_arad_chip_regs.c
 * jer2_arad_chip_tbls.c
 */
#define JER2_ARAD_REGS_GET                                                              (1420|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_REGS_INIT                                                             (1421|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_TBLS_GET                                                              (1422|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_TBLS_INIT                                                             (1423|JER2_ARAD_PROC_BITS)

#define JER2_ARAD_WRITE_ARRAY_OF_FLDS                                                   (1424|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_READ_ARRAY_OF_FLDS                                                    (1425|JER2_ARAD_PROC_BITS)

/*
 * jer2_arad_tbl_access.c {
 */
#define JER2_ARAD_FIELD_IN_PLACE_SET                                                    (1489|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FIELD_IN_PLACE_GET                                                    (1490|JER2_ARAD_PROC_BITS)

#define JER2_ARAD_MGMT_INDIRECT_MEMORY_MAP_GET                                          (1491|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MGMT_INDIRECT_MEMORY_MAP_INIT                                         (1492|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MGMT_INDIRECT_TABLE_MAP_GET                                           (1493|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MGMT_INDIRECT_TABLE_MAP_INIT                                          (1494|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MGMT_INDIRECT_MODULE_INFO_GET                                         (1495|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MGMT_INDIRECT_MODULE_INFO_INIT                                        (1496|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MGMT_INDIRECT_MODULE_INIT                                             (1497|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ACCESS_DB_INIT                                                        (1498|JER2_ARAD_PROC_BITS)

#define JER2_ARAD_OLP_PGE_MEM_TBL_GET_UNSAFE                                            (1501|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_OLP_PGE_MEM_TBL_SET_UNSAFE                                            (1503|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IRE_NIF_CTXT_MAP_TBL_GET_UNSAFE                                       (1505|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IRE_NIF_CTXT_MAP_TBL_SET_UNSAFE                                       (1507|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IRE_NIF_PORT_TO_CTXT_BIT_MAP_TBL_GET_UNSAFE                           (1509|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IRE_NIF_PORT_TO_CTXT_BIT_MAP_TBL_SET_UNSAFE                           (1511|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IRE_RCY_CTXT_MAP_TBL_GET_UNSAFE                                       (1513|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IRE_RCY_CTXT_MAP_TBL_SET_UNSAFE                                       (1515|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IDR_COMPLETE_PC_TBL_GET_UNSAFE                                        (1517|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IDR_COMPLETE_PC_TBL_SET_UNSAFE                                        (1519|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PP_IRR_MCDB_TBL_GET_UNSAFE                                            (1520|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PP_IRR_MCDB_TBL_SET_UNSAFE                                            (1521|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PP_IRR_MCDB_TBL_GET_RAW_UNSAFE                                        (1522|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PP_IRR_MCDB_TBL_SET_RAW_UNSAFE                                        (1523|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IRR_MIRROR_TABLE_TBL_GET_UNSAFE                                       (1525|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IRR_MIRROR_TABLE_TBL_SET_UNSAFE                                       (1527|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IRR_SNOOP_TABLE_TBL_GET_UNSAFE                                        (1529|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IRR_SNOOP_TABLE_TBL_SET_UNSAFE                                        (1531|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IRR_GLAG_TO_LAG_RANGE_TBL_GET_UNSAFE                                  (1533|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IRR_GLAG_TO_LAG_RANGE_TBL_SET_UNSAFE                                  (1535|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IRR_SMOOTH_DIVISION_TBL_GET_UNSAFE                                    (1537|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IRR_SMOOTH_DIVISION_TBL_SET_UNSAFE                                    (1539|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IRR_GLAG_MAPPING_TBL_GET_UNSAFE                                       (1541|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IRR_GLAG_MAPPING_TBL_SET_UNSAFE                                       (1543|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IRR_DESTINATION_TABLE_TBL_GET_UNSAFE                                  (1545|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IRR_DESTINATION_TABLE_TBL_SET_UNSAFE                                  (1547|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IRR_GLAG_NEXT_MEMBER_TBL_GET_UNSAFE                                   (1549|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IRR_GLAG_NEXT_MEMBER_TBL_SET_UNSAFE                                   (1551|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IRR_RLAG_NEXT_MEMBER_TBL_GET_UNSAFE                                   (1553|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IRR_RLAG_NEXT_MEMBER_TBL_SET_UNSAFE                                   (1555|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IHP_PORT_INFO_TBL_GET_UNSAFE                                          (1557|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IHP_PORT_INFO_TBL_SET_UNSAFE                                          (1559|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IHP_PORT_TO_SYSTEM_PORT_ID_TBL_GET_UNSAFE                             (1561|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IHP_PORT_TO_SYSTEM_PORT_ID_TBL_SET_UNSAFE                             (1563|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IHP_STATIC_HEADER_TBL_GET_UNSAFE                                      (1565|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IHP_STATIC_HEADER_TBL_SET_UNSAFE                                      (1567|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IHP_SYSTEM_PORT_MY_PORT_TABLE_TBL_GET_UNSAFE                          (1569|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IHP_SYSTEM_PORT_MY_PORT_TABLE_TBL_SET_UNSAFE                          (1571|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IHP_PTC_COMMANDS1_TBL_GET_UNSAFE                                      (1573|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IHP_PTC_COMMANDS1_TBL_SET_UNSAFE                                      (1575|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IHP_PTC_COMMANDS2_TBL_GET_UNSAFE                                      (1577|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IHP_PTC_COMMANDS2_TBL_SET_UNSAFE                                      (1579|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IHP_PTC_KEY_PROGRAM_LUT_TBL_GET_UNSAFE                                (1581|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IHP_PTC_KEY_PROGRAM_LUT_TBL_SET_UNSAFE                                (1583|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IHP_KEY_PROGRAM0_TBL_GET_UNSAFE                                       (1585|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IHP_KEY_PROGRAM0_TBL_SET_UNSAFE                                       (1587|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IHP_KEY_PROGRAM1_TBL_GET_UNSAFE                                       (1589|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IHP_KEY_PROGRAM1_TBL_SET_UNSAFE                                       (1591|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IHP_KEY_PROGRAM2_TBL_GET_UNSAFE                                       (1593|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IHP_KEY_PROGRAM2_TBL_SET_UNSAFE                                       (1595|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IHP_KEY_PROGRAM3_TBL_GET_UNSAFE                                       (1597|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IHP_KEY_PROGRAM3_TBL_SET_UNSAFE                                       (1599|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IHP_KEY_PROGRAM4_TBL_GET_UNSAFE                                       (1601|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IHP_KEY_PROGRAM4_TBL_SET_UNSAFE                                       (1603|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IHP_PROGRAMMABLE_COS_TBL_GET_UNSAFE                                   (1605|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IHP_PROGRAMMABLE_COS_TBL_SET_UNSAFE                                   (1607|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IHP_PROGRAMMABLE_COS1_TBL_GET_UNSAFE                                  (1609|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IHP_PROGRAMMABLE_COS1_TBL_SET_UNSAFE                                  (1611|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_EYE_SCAN_RUN                                                      (1612|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_EYE_SCAN_RUN_MULTIPLE_ALLOC                                       (1613|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_EYE_SCAN_RUN_MULTIPLE_ALLOC_UNSAFE                                (1614|JER2_ARAD_PROC_BITS)

#define JER2_ARAD_IQM_BDB_LINK_LIST_TBL_GET_UNSAFE                                      (1621|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IQM_BDB_LINK_LIST_TBL_SET_UNSAFE                                      (1623|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IQM_DYNAMIC_TBL_GET_UNSAFE                                            (1625|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IQM_DYNAMIC_TBL_SET_UNSAFE                                            (1627|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IQM_STATIC_TBL_GET_UNSAFE                                             (1629|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IQM_STATIC_TBL_SET_UNSAFE                                             (1631|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IQM_PACKET_QUEUE_TAIL_POINTER_TBL_GET_UNSAFE                          (1633|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IQM_PACKET_QUEUE_TAIL_POINTER_TBL_SET_UNSAFE                          (1635|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IQM_PACKET_QUEUE_RED_WEIGHT_TABLE_TBL_GET_UNSAFE                      (1637|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IQM_PACKET_QUEUE_RED_WEIGHT_TABLE_TBL_SET_UNSAFE                      (1639|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IQM_CREDIT_DISCOUNT_TABLE_TBL_GET_UNSAFE                              (1641|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IQM_CREDIT_DISCOUNT_TABLE_TBL_SET_UNSAFE                              (1643|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IQM_FULL_USER_COUNT_MEMORY_TBL_GET_UNSAFE                             (1645|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IQM_FULL_USER_COUNT_MEMORY_TBL_SET_UNSAFE                             (1647|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IQM_MINI_MULTICAST_USER_COUNT_MEMORY_TBL_GET_UNSAFE                   (1649|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IQM_MINI_MULTICAST_USER_COUNT_MEMORY_TBL_SET_UNSAFE                   (1651|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IQM_PACKET_QUEUE_RED_PARAMETERS_TAIL_DROP_MANTISSA_NOF_BITS           (1652|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IQM_PACKET_QUEUE_RED_PARAMETERS_TABLE_TBL_GET_UNSAFE                  (1653|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IQM_PACKET_QUEUE_RED_PARAMETERS_TABLE_TBL_SET_UNSAFE                  (1655|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IQM_VSQ_DESCRIPTOR_RATE_CLASS_GROUP_A_TBL_GET_UNSAFE                  (1657|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IQM_VSQ_DESCRIPTOR_RATE_CLASS_GROUP_A_TBL_SET_UNSAFE                  (1659|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IQM_VSQ_DESCRIPTOR_RATE_CLASS_GROUP_B_TBL_GET_UNSAFE                  (1661|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IQM_VSQ_DESCRIPTOR_RATE_CLASS_GROUP_B_TBL_SET_UNSAFE                  (1663|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IQM_VSQ_DESCRIPTOR_RATE_CLASS_GROUP_C_TBL_GET_UNSAFE                  (1665|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IQM_VSQ_DESCRIPTOR_RATE_CLASS_GROUP_C_TBL_SET_UNSAFE                  (1667|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IQM_VSQ_DESCRIPTOR_RATE_CLASS_GROUP_D_TBL_GET_UNSAFE                  (1669|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IQM_VSQ_DESCRIPTOR_RATE_CLASS_GROUP_D_TBL_SET_UNSAFE                  (1671|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IQM_VSQ_QSIZE_MEMORY_GROUP_A_TBL_GET_UNSAFE                           (1673|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IQM_VSQ_QSIZE_MEMORY_GROUP_A_TBL_SET_UNSAFE                           (1675|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IQM_VSQ_QSIZE_MEMORY_GROUP_B_TBL_GET_UNSAFE                           (1677|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IQM_VSQ_QSIZE_MEMORY_GROUP_B_TBL_SET_UNSAFE                           (1679|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IQM_VSQ_QSIZE_MEMORY_GROUP_C_TBL_GET_UNSAFE                           (1681|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IQM_VSQ_QSIZE_MEMORY_GROUP_C_TBL_SET_UNSAFE                           (1683|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IQM_VSQ_QSIZE_MEMORY_GROUP_D_TBL_GET_UNSAFE                           (1685|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IQM_VSQ_QSIZE_MEMORY_GROUP_D_TBL_SET_UNSAFE                           (1687|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IQM_VSQ_AVERAGE_QSIZE_MEMORY_GROUP_A_TBL_GET_UNSAFE                   (1689|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IQM_VSQ_AVERAGE_QSIZE_MEMORY_GROUP_A_TBL_SET_UNSAFE                   (1691|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IQM_VSQ_AVERAGE_QSIZE_MEMORY_GROUP_B_TBL_GET_UNSAFE                   (1693|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IQM_VSQ_AVERAGE_QSIZE_MEMORY_GROUP_B_TBL_SET_UNSAFE                   (1695|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IQM_VSQ_AVERAGE_QSIZE_MEMORY_GROUP_C_TBL_GET_UNSAFE                   (1697|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IQM_VSQ_AVERAGE_QSIZE_MEMORY_GROUP_C_TBL_SET_UNSAFE                   (1699|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IQM_VSQ_AVERAGE_QSIZE_MEMORY_GROUP_D_TBL_GET_UNSAFE                   (1701|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IQM_VSQ_AVERAGE_QSIZE_MEMORY_GROUP_D_TBL_SET_UNSAFE                   (1703|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IQM_VSQ_FLOW_CONTROL_PARAMETERS_TABLE_GROUP_TBL_GET_UNSAFE            (1705|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IQM_VSQ_FLOW_CONTROL_PARAMETERS_TABLE_GROUP_TBL_SET_UNSAFE            (1707|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IQM_VSQ_QUEUE_PARAMETERS_TABLE_GROUP_TBL_GET_UNSAFE                   (1721|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IQM_VSQ_QUEUE_PARAMETERS_TABLE_GROUP_TBL_SET_UNSAFE                   (1723|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IQM_SYSTEM_RED_PARAMETERS_TABLE_TBL_GET_UNSAFE                        (1737|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IQM_SYSTEM_RED_PARAMETERS_TABLE_TBL_SET_UNSAFE                        (1739|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IQM_SYSTEM_RED_DROP_PROBABILITY_VALUES_TBL_GET_UNSAFE                 (1741|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IQM_SYSTEM_RED_DROP_PROBABILITY_VALUES_TBL_SET_UNSAFE                 (1743|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IQM_SYSTEM_RED_TBL_GET_UNSAFE                                         (1745|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IQM_SYSTEM_RED_TBL_SET_UNSAFE                                         (1746|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_QDR_MEMORY_TBL_GET_UNSAFE                                             (1747|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_QDR_MEMORY_TBL_SET_UNSAFE                                             (1748|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPS_SYSTEM_PHYSICAL_PORT_LOOKUP_TABLE_TBL_GET_UNSAFE                  (1749|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPS_SYSTEM_PHYSICAL_PORT_LOOKUP_TABLE_TBL_SET_UNSAFE                  (1751|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPS_DESTINATION_DEVICE_AND_PORT_LOOKUP_TABLE_TBL_GET_UNSAFE           (1753|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPS_DESTINATION_DEVICE_AND_PORT_LOOKUP_TABLE_TBL_SET_UNSAFE           (1755|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPS_FLOW_ID_LOOKUP_TABLE_TBL_GET_UNSAFE                               (1757|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPS_FLOW_ID_LOOKUP_TABLE_TBL_SET_UNSAFE                               (1759|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPS_QUEUE_TYPE_LOOKUP_TABLE_TBL_GET_UNSAFE                            (1761|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPS_QUEUE_TYPE_LOOKUP_TABLE_TBL_SET_UNSAFE                            (1763|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPS_QUEUE_PRIORITY_MAP_SELECT_TBL_GET_UNSAFE                          (1765|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPS_QUEUE_PRIORITY_MAP_SELECT_TBL_SET_UNSAFE                          (1767|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPS_QUEUE_PRIORITY_MAPS_TABLE_TBL_GET_UNSAFE                          (1769|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPS_QUEUE_PRIORITY_MAPS_TABLE_TBL_SET_UNSAFE                          (1771|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPS_QUEUE_SIZE_BASED_THRESHOLDS_TABLE_TBL_GET_UNSAFE                  (1773|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPS_QUEUE_SIZE_BASED_THRESHOLDS_TABLE_TBL_SET_UNSAFE                  (1775|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPS_CREDIT_BALANCE_BASED_THRESHOLDS_TABLE_TBL_GET_UNSAFE              (1777|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPS_CREDIT_BALANCE_BASED_THRESHOLDS_TABLE_TBL_SET_UNSAFE              (1779|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPS_EMPTY_QUEUE_CREDIT_BALANCE_TABLE_TBL_GET_UNSAFE                   (1781|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPS_EMPTY_QUEUE_CREDIT_BALANCE_TABLE_TBL_SET_UNSAFE                   (1783|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPS_CREDIT_WATCHDOG_THRESHOLDS_TABLE_TBL_GET_UNSAFE                   (1785|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPS_CREDIT_WATCHDOG_THRESHOLDS_TABLE_TBL_SET_UNSAFE                   (1787|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPS_QUEUE_DESCRIPTOR_TABLE_TBL_GET_UNSAFE                             (1789|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPS_QUEUE_DESCRIPTOR_TABLE_TBL_SET_UNSAFE                             (1791|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPS_QUEUE_SIZE_TABLE_TBL_GET_UNSAFE                                   (1793|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPS_QUEUE_SIZE_TABLE_TBL_SET_UNSAFE                                   (1795|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPS_SYSTEM_RED_MAX_QUEUE_SIZE_TABLE_TBL_GET_UNSAFE                    (1797|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPS_SYSTEM_RED_MAX_QUEUE_SIZE_TABLE_TBL_SET_UNSAFE                    (1799|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DPI_DLL_RAM_TBL_GET_UNSAFE                                            (1805|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DPI_DLL_RAM_TBL_SET_UNSAFE                                            (1807|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_RTP_UNICAST_DISTRIBUTION_MEMORY_FOR_DATA_CELLS_TBL_GET_UNSAFE         (1809|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_RTP_UNICAST_DISTRIBUTION_MEMORY_FOR_DATA_CELLS_TBL_SET_UNSAFE         (1811|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_RTP_UNICAST_DISTRIBUTION_MEMORY_FOR_CONTROL_CELLS_TBL_GET_UNSAFE      (1813|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_RTP_UNICAST_DISTRIBUTION_MEMORY_FOR_CONTROL_CELLS_TBL_SET_UNSAFE      (1815|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_NIFA_CH0_SCM_TBL_GET_UNSAFE                                       (1817|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_NIFA_CH0_SCM_TBL_SET_UNSAFE                                       (1819|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_NIFA_CH1_SCM_TBL_GET_UNSAFE                                       (1821|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_NIFA_CH1_SCM_TBL_SET_UNSAFE                                       (1823|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_NIFA_CH2_SCM_TBL_GET_UNSAFE                                       (1825|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_NIFA_CH2_SCM_TBL_SET_UNSAFE                                       (1827|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_NIFA_CH3_SCM_TBL_GET_UNSAFE                                       (1829|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_NIFA_CH3_SCM_TBL_SET_UNSAFE                                       (1831|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_NIFB_CH0_SCM_TBL_GET_UNSAFE                                       (1833|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_NIFB_CH0_SCM_TBL_SET_UNSAFE                                       (1835|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_NIFB_CH1_SCM_TBL_GET_UNSAFE                                       (1837|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_NIFB_CH1_SCM_TBL_SET_UNSAFE                                       (1839|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_NIFB_CH2_SCM_TBL_GET_UNSAFE                                       (1841|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_NIFB_CH2_SCM_TBL_SET_UNSAFE                                       (1843|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_NIFB_CH3_SCM_TBL_GET_UNSAFE                                       (1845|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_NIFB_CH3_SCM_TBL_SET_UNSAFE                                       (1847|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_NIFAB_NCH_SCM_TBL_GET_UNSAFE                                      (1849|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_NIFAB_NCH_SCM_TBL_SET_UNSAFE                                      (1851|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_RCY_SCM_TBL_GET_UNSAFE                                            (1853|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_RCY_SCM_TBL_SET_UNSAFE                                            (1855|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_CPU_SCM_TBL_GET_UNSAFE                                            (1857|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_CPU_SCM_TBL_SET_UNSAFE                                            (1859|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_CCM_TBL_GET_UNSAFE                                                (1861|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_CCM_TBL_SET_UNSAFE                                                (1863|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_PMC_TBL_GET_UNSAFE                                                (1865|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_PMC_TBL_SET_UNSAFE                                                (1867|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_FBM_TBL_GET_UNSAFE                                                (1869|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_FBM_TBL_SET_UNSAFE                                                (1871|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_FDM_TBL_GET_UNSAFE                                                (1873|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_FDM_TBL_SET_UNSAFE                                                (1875|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_DWM_TBL_GET_UNSAFE                                                (1877|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_DWM_TBL_SET_UNSAFE                                                (1879|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_RRDM_TBL_GET_UNSAFE                                               (1881|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_RRDM_TBL_SET_UNSAFE                                               (1883|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_RPDM_TBL_GET_UNSAFE                                               (1885|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_RPDM_TBL_SET_UNSAFE                                               (1887|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_PCT_TBL_GET_UNSAFE                                                (1889|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_PCT_TBL_SET_UNSAFE                                                (1891|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_VLAN_TABLE_TBL_GET_UNSAFE                                         (1893|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_VLAN_TABLE_TBL_SET_UNSAFE                                         (1895|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_TDMMCID_TBL_GET_UNSAFE                                            (1897|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_TDMMCID_TBL_SET_UNSAFE                                            (1899|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EPNI_NIFA_CH0_SCM_TBL_GET_UNSAFE                                      (1901|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EPNI_NIFA_CH0_SCM_TBL_SET_UNSAFE                                      (1903|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EPNI_NIFA_CH1_SCM_TBL_GET_UNSAFE                                      (1905|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EPNI_NIFA_CH1_SCM_TBL_SET_UNSAFE                                      (1907|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EPNI_NIFA_CH2_SCM_TBL_GET_UNSAFE                                      (1909|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EPNI_NIFA_CH2_SCM_TBL_SET_UNSAFE                                      (1911|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EPNI_NIFA_CH3_SCM_TBL_GET_UNSAFE                                      (1913|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EPNI_NIFA_CH3_SCM_TBL_SET_UNSAFE                                      (1915|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EPNI_NIFB_CH0_SCM_TBL_GET_UNSAFE                                      (1917|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EPNI_NIFB_CH0_SCM_TBL_SET_UNSAFE                                      (1919|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EPNI_NIFB_CH1_SCM_TBL_GET_UNSAFE                                      (1921|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EPNI_NIFB_CH1_SCM_TBL_SET_UNSAFE                                      (1923|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EPNI_NIFB_CH2_SCM_TBL_GET_UNSAFE                                      (1925|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EPNI_NIFB_CH2_SCM_TBL_SET_UNSAFE                                      (1927|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EPNI_NIFB_CH3_SCM_TBL_GET_UNSAFE                                      (1929|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EPNI_NIFB_CH3_SCM_TBL_SET_UNSAFE                                      (1931|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EPNI_NIFAB_NCH_SCM_TBL_GET_UNSAFE                                     (1933|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EPNI_NIFAB_NCH_SCM_TBL_SET_UNSAFE                                     (1935|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EPNI_RCY_SCM_TBL_GET_UNSAFE                                           (1937|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EPNI_RCY_SCM_TBL_SET_UNSAFE                                           (1939|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EPNI_CPU_SCM_TBL_GET_UNSAFE                                           (1941|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EPNI_CPU_SCM_TBL_SET_UNSAFE                                           (1943|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EPNI_CCM_TBL_GET_UNSAFE                                               (1945|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EPNI_CCM_TBL_SET_UNSAFE                                               (1947|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EPNI_PMC_TBL_GET_UNSAFE                                               (1949|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EPNI_PMC_TBL_SET_UNSAFE                                               (1951|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EPNI_CBM_TBL_GET_UNSAFE                                               (1953|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EPNI_CBM_TBL_SET_UNSAFE                                               (1955|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EPNI_FBM_TBL_GET_UNSAFE                                               (1957|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EPNI_FBM_TBL_SET_UNSAFE                                               (1959|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EPNI_FDM_TBL_GET_UNSAFE                                               (1961|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EPNI_FDM_TBL_SET_UNSAFE                                               (1963|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EPNI_DWM_TBL_GET_UNSAFE                                               (1965|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EPNI_DWM_TBL_SET_UNSAFE                                               (1967|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EPNI_RRDM_TBL_GET_UNSAFE                                              (1969|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EPNI_RRDM_TBL_SET_UNSAFE                                              (1971|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EPNI_RPDM_TBL_GET_UNSAFE                                              (1973|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EPNI_RPDM_TBL_SET_UNSAFE                                              (1975|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EPNI_PCT_TBL_GET_UNSAFE                                               (1977|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EPNI_PCT_TBL_SET_UNSAFE                                               (1979|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EPNI_VLAN_TABLE_TBL_GET_UNSAFE                                        (1981|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EPNI_VLAN_TABLE_TBL_SET_UNSAFE                                        (1983|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_CFC_RECYCLE_TO_OUT_GOING_FAP_PORT_MAPPING_TBL_GET_UNSAFE              (1985|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_CFC_RECYCLE_TO_OUT_GOING_FAP_PORT_MAPPING_TBL_SET_UNSAFE              (1987|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_CFC_NIF_A_CLASS_BASED_TO_OFP_MAPPING_TBL_GET_UNSAFE                   (1989|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_CFC_NIF_A_CLASS_BASED_TO_OFP_MAPPING_TBL_SET_UNSAFE                   (1991|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_CFC_NIF_B_CLASS_BASED_TO_OFP_MAPPING_TBL_GET_UNSAFE                   (1993|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_CFC_NIF_B_CLASS_BASED_TO_OFP_MAPPING_TBL_SET_UNSAFE                   (1995|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_CFC_A_SCHEDULERS_BASED_FLOW_CONTROL_TO_OFP_MAPPING_TBL_GET_UNSAFE     (1997|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_CFC_A_SCHEDULERS_BASED_FLOW_CONTROL_TO_OFP_MAPPING_TBL_SET_UNSAFE     (1999|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_CFC_B_SCHEDULERS_BASED_FLOW_CONTROL_TO_OFP_MAPPING_TBL_GET_UNSAFE     (2001|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_CFC_B_SCHEDULERS_BASED_FLOW_CONTROL_TO_OFP_MAPPING_TBL_SET_UNSAFE     (2003|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_CFC_OUT_OF_BAND_RX_A_CALENDAR_MAPPING_TBL_GET_UNSAFE                  (2005|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_CFC_OUT_OF_BAND_RX_A_CALENDAR_MAPPING_TBL_SET_UNSAFE                  (2007|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_CFC_OUT_OF_BAND_RX_B_CALENDAR_MAPPING_TBL_GET_UNSAFE                  (2009|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_CFC_OUT_OF_BAND_RX_B_CALENDAR_MAPPING_TBL_SET_UNSAFE                  (2011|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_CFC_OUT_OF_BAND_TX_CALENDAR_MAPPING_TBL_GET_UNSAFE                    (2013|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_CFC_OUT_OF_BAND_TX_CALENDAR_MAPPING_TBL_SET_UNSAFE                    (2015|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_CAL_TBL_GET_UNSAFE                                                (2017|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_CAL_TBL_SET_UNSAFE                                                (2019|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_DRM_TBL_GET_UNSAFE                                                (2021|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_DRM_TBL_SET_UNSAFE                                                (2023|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_DSM_TBL_GET_UNSAFE                                                (2025|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_DSM_TBL_SET_UNSAFE                                                (2027|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_FDMS_TBL_GET_UNSAFE                                               (2029|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_FDMS_TBL_SET_UNSAFE                                               (2031|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_SHDS_TBL_GET_UNSAFE                                               (2033|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_SHDS_TBL_SET_UNSAFE                                               (2035|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_SEM_TBL_GET_UNSAFE                                                (2037|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_SEM_TBL_SET_UNSAFE                                                (2039|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_FSF_TBL_GET_UNSAFE                                                (2041|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_FSF_TBL_SET_UNSAFE                                                (2043|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_FGM_TBL_GET_UNSAFE                                                (2045|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_FGM_TBL_SET_UNSAFE                                                (2047|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_SHC_TBL_GET_UNSAFE                                                (2049|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_SHC_TBL_SET_UNSAFE                                                (2051|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_SCC_TBL_GET_UNSAFE                                                (2053|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_SCC_TBL_SET_UNSAFE                                                (2055|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_SCT_TBL_GET_UNSAFE                                                (2057|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_SCT_TBL_SET_UNSAFE                                                (2059|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_FQM_TBL_GET_UNSAFE                                                (2061|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_FQM_TBL_SET_UNSAFE                                                (2063|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_FFM_TBL_GET_UNSAFE                                                (2065|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_FFM_TBL_SET_UNSAFE                                                (2067|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_TMC_TBL_GET_UNSAFE                                                (2069|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_TMC_TBL_SET_UNSAFE                                                (2071|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_PQS_TBL_GET_UNSAFE                                                (2073|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_PQS_TBL_SET_UNSAFE                                                (2075|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_SCHEDULER_INIT_TBL_GET_UNSAFE                                     (2077|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_SCHEDULER_INIT_TBL_SET_UNSAFE                                     (2079|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_FORCE_STATUS_MESSAGE_TBL_GET_UNSAFE                               (2081|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_FORCE_STATUS_MESSAGE_TBL_SET_UNSAFE                               (2083|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_CBM_TBL_GET_UNSAFE                                                (2085|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_CBM_TBL_SET_UNSAFE                                                (2087|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPT_BDQ_TBL_GET_UNSAFE                                                (2091|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPT_BDQ_TBL_SET_UNSAFE                                                (2093|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPT_PCQ_TBL_GET_UNSAFE                                                (2095|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPT_PCQ_TBL_SET_UNSAFE                                                (2097|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPT_SOP_MMU_TBL_GET_UNSAFE                                            (2099|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPT_SOP_MMU_TBL_SET_UNSAFE                                            (2101|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPT_MOP_MMU_TBL_GET_UNSAFE                                            (2103|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPT_MOP_MMU_TBL_SET_UNSAFE                                            (2105|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPT_FDTCTL_TBL_GET_UNSAFE                                             (2107|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPT_FDTCTL_TBL_SET_UNSAFE                                             (2109|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPT_FDTDATA_TBL_GET_UNSAFE                                            (2111|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPT_FDTDATA_TBL_SET_UNSAFE                                            (2113|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPT_EGQCTL_TBL_GET_UNSAFE                                             (2115|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPT_EGQCTL_TBL_SET_UNSAFE                                             (2117|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPT_EGQDATA_TBL_GET_UNSAFE                                            (2119|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPT_EGQDATA_TBL_SET_UNSAFE                                            (2121|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_CFC_FLOW_CONTROL_TBL_GET_UNSAFE                                   (2123|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_CFC_FLOW_CONTROL_TBL_SET_UNSAFE                                   (2125|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_NIFA_FLOW_CONTROL_TBL_GET_UNSAFE                                  (2127|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_NIFA_FLOW_CONTROL_TBL_SET_UNSAFE                                  (2129|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_NIFB_FLOW_CONTROL_TBL_GET_UNSAFE                                  (2131|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_NIFB_FLOW_CONTROL_TBL_SET_UNSAFE                                  (2133|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_CPU_LAST_HEADER_TBL_GET_UNSAFE                                    (2135|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_CPU_LAST_HEADER_TBL_SET_UNSAFE                                    (2137|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_IPT_LAST_HEADER_TBL_GET_UNSAFE                                    (2139|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_IPT_LAST_HEADER_TBL_SET_UNSAFE                                    (2141|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_FDR_LAST_HEADER_TBL_GET_UNSAFE                                    (2143|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_FDR_LAST_HEADER_TBL_SET_UNSAFE                                    (2145|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_CPU_PACKET_COUNTER_TBL_GET_UNSAFE                                 (2147|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_CPU_PACKET_COUNTER_TBL_SET_UNSAFE                                 (2149|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_IPT_PACKET_COUNTER_TBL_GET_UNSAFE                                 (2151|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_IPT_PACKET_COUNTER_TBL_SET_UNSAFE                                 (2153|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_FDR_PACKET_COUNTER_TBL_GET_UNSAFE                                 (2155|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_FDR_PACKET_COUNTER_TBL_SET_UNSAFE                                 (2157|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_RQP_PACKET_COUNTER_TBL_GET_UNSAFE                                 (2159|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_RQP_PACKET_COUNTER_TBL_SET_UNSAFE                                 (2161|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_RQP_DISCARD_PACKET_COUNTER_TBL_GET_UNSAFE                         (2163|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_RQP_DISCARD_PACKET_COUNTER_TBL_SET_UNSAFE                         (2165|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_EHP_UNICAST_PACKET_COUNTER_TBL_GET_UNSAFE                         (2167|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_EHP_UNICAST_PACKET_COUNTER_TBL_SET_UNSAFE                         (2169|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_EHP_MULTICAST_HIGH_PACKET_COUNTER_TBL_GET_UNSAFE                  (2171|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_EHP_MULTICAST_HIGH_PACKET_COUNTER_TBL_SET_UNSAFE                  (2173|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_EHP_MULTICAST_LOW_PACKET_COUNTER_TBL_GET_UNSAFE                   (2175|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_EHP_MULTICAST_LOW_PACKET_COUNTER_TBL_SET_UNSAFE                   (2177|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_EHP_DISCARD_PACKET_COUNTER_TBL_GET_UNSAFE                         (2179|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_EHP_DISCARD_PACKET_COUNTER_TBL_SET_UNSAFE                         (2181|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_PQP_UNICAST_HIGH_PACKET_COUNTER_TBL_GET_UNSAFE                    (2183|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_PQP_UNICAST_HIGH_PACKET_COUNTER_TBL_SET_UNSAFE                    (2185|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_PQP_UNICAST_LOW_PACKET_COUNTER_TBL_GET_UNSAFE                     (2187|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_PQP_UNICAST_LOW_PACKET_COUNTER_TBL_SET_UNSAFE                     (2189|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_PQP_MULTICAST_HIGH_PACKET_COUNTER_TBL_GET_UNSAFE                  (2191|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_PQP_MULTICAST_HIGH_PACKET_COUNTER_TBL_SET_UNSAFE                  (2193|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_PQP_MULTICAST_LOW_PACKET_COUNTER_TBL_GET_UNSAFE                   (2195|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_PQP_MULTICAST_LOW_PACKET_COUNTER_TBL_SET_UNSAFE                   (2197|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_PQP_UNICAST_HIGH_BYTES_COUNTER_TBL_GET_UNSAFE                     (2199|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_PQP_UNICAST_HIGH_BYTES_COUNTER_TBL_SET_UNSAFE                     (2201|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_PQP_UNICAST_LOW_BYTES_COUNTER_TBL_GET_UNSAFE                      (2203|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_PQP_UNICAST_LOW_BYTES_COUNTER_TBL_SET_UNSAFE                      (2205|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_PQP_MULTICAST_HIGH_BYTES_COUNTER_TBL_GET_UNSAFE                   (2207|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_PQP_MULTICAST_HIGH_BYTES_COUNTER_TBL_SET_UNSAFE                   (2209|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_PQP_MULTICAST_LOW_BYTES_COUNTER_TBL_GET_UNSAFE                    (2211|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_PQP_MULTICAST_LOW_BYTES_COUNTER_TBL_SET_UNSAFE                    (2213|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_PQP_DISCARD_UNICAST_PACKET_COUNTER_TBL_GET_UNSAFE                 (2215|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_PQP_DISCARD_UNICAST_PACKET_COUNTER_TBL_SET_UNSAFE                 (2217|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_PQP_DISCARD_MULTICAST_PACKET_COUNTER_TBL_GET_UNSAFE               (2219|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_PQP_DISCARD_MULTICAST_PACKET_COUNTER_TBL_SET_UNSAFE               (2221|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_FQP_PACKET_COUNTER_TBL_GET_UNSAFE                                 (2223|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_FQP_PACKET_COUNTER_TBL_SET_UNSAFE                                 (2225|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MMU_DRAM_ADDRESS_SPACE_TBL_GET_UNSAFE                                 (2231|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MMU_DRAM_ADDRESS_SPACE_TBL_SET_UNSAFE                                 (2233|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MMU_DRAM_ADDRESS_SPACE_INFO_GET_UNSAFE                                (2234|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MMU_IDF_TBL_GET_UNSAFE                                                (2235|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MMU_IDF_TBL_SET_UNSAFE                                                (2237|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MMU_FDF_TBL_GET_UNSAFE                                                (2239|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MMU_FDF_TBL_SET_UNSAFE                                                (2241|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MMU_RDFA_WADDR_STATUS_TBL_GET_UNSAFE                                  (2243|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MMU_RDFA_WADDR_STATUS_TBL_SET_UNSAFE                                  (2245|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MMU_RDFB_WADDR_STATUS_TBL_GET_UNSAFE                                  (2247|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MMU_RDFB_WADDR_STATUS_TBL_SET_UNSAFE                                  (2249|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MMU_RDFC_WADDR_STATUS_TBL_GET_UNSAFE                                  (2251|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MMU_RDFC_WADDR_STATUS_TBL_SET_UNSAFE                                  (2253|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MMU_RDFD_WADDR_STATUS_TBL_GET_UNSAFE                                  (2255|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MMU_RDFD_WADDR_STATUS_TBL_SET_UNSAFE                                  (2257|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MMU_RDFE_WADDR_STATUS_TBL_GET_UNSAFE                                  (2259|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MMU_RDFE_WADDR_STATUS_TBL_SET_UNSAFE                                  (2261|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MMU_RDFF_WADDR_STATUS_TBL_GET_UNSAFE                                  (2263|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MMU_RDFF_WADDR_STATUS_TBL_SET_UNSAFE                                  (2265|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MMU_RDF_RADDR_TBL_GET_UNSAFE                                          (2267|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MMU_RDF_RADDR_TBL_SET_UNSAFE                                          (2269|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MMU_WAF_HALFA_WADDR_TBL_GET_UNSAFE                                    (2271|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MMU_WAF_HALFA_WADDR_TBL_SET_UNSAFE                                    (2273|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MMU_WAF_HALFB_WADDR_TBL_GET_UNSAFE                                    (2275|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MMU_WAF_HALFB_WADDR_TBL_SET_UNSAFE                                    (2277|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MMU_WAFA_HALFA_RADDR_STATUS_TBL_GET_UNSAFE                            (2279|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MMU_WAFA_HALFA_RADDR_STATUS_TBL_SET_UNSAFE                            (2281|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MMU_WAFB_HALFA_RADDR_STATUS_TBL_GET_UNSAFE                            (2283|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MMU_WAFB_HALFA_RADDR_STATUS_TBL_SET_UNSAFE                            (2285|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MMU_WAFC_HALFA_RADDR_STATUS_TBL_GET_UNSAFE                            (2287|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MMU_WAFC_HALFA_RADDR_STATUS_TBL_SET_UNSAFE                            (2289|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MMU_WAFD_HALFA_RADDR_STATUS_TBL_GET_UNSAFE                            (2291|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MMU_WAFD_HALFA_RADDR_STATUS_TBL_SET_UNSAFE                            (2293|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MMU_WAFE_HALFA_RADDR_STATUS_TBL_GET_UNSAFE                            (2295|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MMU_WAFE_HALFA_RADDR_STATUS_TBL_SET_UNSAFE                            (2297|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MMU_WAFF_HALFA_RADDR_STATUS_TBL_GET_UNSAFE                            (2299|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MMU_WAFF_HALFA_RADDR_STATUS_TBL_SET_UNSAFE                            (2301|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MMU_WAFA_HALFB_RADDR_STATUS_TBL_GET_UNSAFE                            (2303|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MMU_WAFA_HALFB_RADDR_STATUS_TBL_SET_UNSAFE                            (2305|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MMU_WAFB_HALFB_RADDR_STATUS_TBL_GET_UNSAFE                            (2307|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MMU_WAFB_HALFB_RADDR_STATUS_TBL_SET_UNSAFE                            (2309|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MMU_WAFC_HALFB_RADDR_STATUS_TBL_GET_UNSAFE                            (2311|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MMU_WAFC_HALFB_RADDR_STATUS_TBL_SET_UNSAFE                            (2313|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MMU_WAFD_HALFB_RADDR_STATUS_TBL_GET_UNSAFE                            (2315|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MMU_WAFD_HALFB_RADDR_STATUS_TBL_SET_UNSAFE                            (2317|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MMU_WAFE_HALFB_RADDR_STATUS_TBL_GET_UNSAFE                            (2319|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MMU_WAFE_HALFB_RADDR_STATUS_TBL_SET_UNSAFE                            (2321|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MMU_WAFF_HALFB_RADDR_STATUS_TBL_GET_UNSAFE                            (2323|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MMU_WAFF_HALFB_RADDR_STATUS_TBL_SET_UNSAFE                            (2325|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MMU_RAF_WADDR_TBL_GET_UNSAFE                                          (2327|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MMU_RAF_WADDR_TBL_SET_UNSAFE                                          (2329|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MMU_RAFA_RADDR_STATUS_TBL_GET_UNSAFE                                  (2331|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MMU_RAFA_RADDR_STATUS_TBL_SET_UNSAFE                                  (2333|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MMU_RAFB_RADDR_STATUS_TBL_GET_UNSAFE                                  (2335|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MMU_RAFB_RADDR_STATUS_TBL_SET_UNSAFE                                  (2337|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MMU_RAFC_RADDR_STATUS_TBL_GET_UNSAFE                                  (2339|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MMU_RAFC_RADDR_STATUS_TBL_SET_UNSAFE                                  (2341|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MMU_RAFD_RADDR_STATUS_TBL_GET_UNSAFE                                  (2343|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MMU_RAFD_RADDR_STATUS_TBL_SET_UNSAFE                                  (2345|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MMU_RAFE_RADDR_STATUS_TBL_GET_UNSAFE                                  (2347|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MMU_RAFE_RADDR_STATUS_TBL_SET_UNSAFE                                  (2349|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MMU_RAFF_RADDR_STATUS_TBL_GET_UNSAFE                                  (2351|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MMU_RAFF_RADDR_STATUS_TBL_SET_UNSAFE                                  (2353|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPT_SELECT_SOURCE_SUM_TBL_SET_UNSAFE                                  (2354|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IQM_PACKET_DESCRIPTOR_FIFOS_MEMORY_TBL_GET_UNSAFE                     (2355|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IQM_PACKET_DESCRIPTOR_FIFOS_MEMORY_TBL_SET_UNSAFE                     (2356|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IQM_TX_DESCRIPTOR_FIFOS_MEMORY_TBL_GET_UNSAFE                         (2357|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IQM_TX_DESCRIPTOR_FIFOS_MEMORY_TBL_SET_UNSAFE                         (2358|JER2_ARAD_PROC_BITS)

/*
 * jer2_arad_tbl_access.c }
 */

/*
 * access tables
 */
#define JER2_ARAD_TBL_READ_LOW                                                          (2426|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_TBL_WRITE_LOW                                                         (2427|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_TBL_READ_LOW_UNSAFE                                                   (2428|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_TBL_WRITE_LOW_UNSAFE                                                  (2429|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_TBL_FIELD_MAX_VALUE_GET                                               (2430|JER2_ARAD_PROC_BITS)

/*
 * jer2_arad_mgmt {
 */
#define JER2_ARAD_MGMT_SYSTEM_FAP_ID_GET                                                (2431|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MGMT_SYSTEM_FAP_ID_GET_UNSAFE                                         (2432|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MGMT_SYSTEM_FAP_ID_SET                                                (2433|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MGMT_SYSTEM_FAP_ID_SET_UNSAFE                                         (2434|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MGMT_SYSTEM_FAP_ID_VERIFY                                             (2435|JER2_ARAD_PROC_BITS)
/*
 * jer2_arad_mgmt }
 */

/*
 * jer2_arad_api_framework.c
 */
#define JER2_ARAD_GET_ERR_TEXT                                                          (2445|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PROC_ID_TO_STRING                                                     (2446|JER2_ARAD_PROC_BITS)

#define JER2_ARAD_MULT_FABRIC_CREDIT_SOURCE_VERIFY                                      (2450|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PP_IRR_MCDB_EGRESS_FORMAT_0_TBL_GET_UNSAFE                            (2451|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PP_IRR_MCDB_EGRESS_FORMAT_0_TBL_SET_UNSAFE                            (2452|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PP_IRR_MCDB_EGRESS_FORMAT_1_TBL_GET_UNSAFE                            (2453|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PP_IRR_MCDB_EGRESS_FORMAT_1_TBL_SET_UNSAFE                            (2454|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PP_IRR_MCDB_EGRESS_FORMAT_2_TBL_GET_UNSAFE                            (2455|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PP_IRR_MCDB_EGRESS_FORMAT_2_TBL_SET_UNSAFE                            (2456|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PP_IRR_MCDB_EGRESS_FORMAT_3_TBL_GET_UNSAFE                            (2457|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PP_IRR_MCDB_EGRESS_FORMAT_3_TBL_SET_UNSAFE                            (2458|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PP_IRR_MCDB_EGRESS_FORMAT_4_TBL_GET_UNSAFE                            (2459|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PP_IRR_MCDB_EGRESS_FORMAT_4_TBL_SET_UNSAFE                            (2460|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PP_IRR_MCDB_EGRESS_FORMAT_5_TBL_GET_UNSAFE                            (2461|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PP_IRR_MCDB_EGRESS_FORMAT_5_TBL_SET_UNSAFE                            (2462|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PP_IRR_MCDB_EGRESS_FORMAT_6_TBL_GET_UNSAFE                            (2463|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PP_IRR_MCDB_EGRESS_FORMAT_6_TBL_SET_UNSAFE                            (2464|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PP_IRR_MCDB_EGRESS_FORMAT_7_TBL_GET_UNSAFE                            (2465|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PP_IRR_MCDB_EGRESS_FORMAT_7_TBL_SET_UNSAFE                            (2466|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PP_IRR_MCDB_EGRESS_SPECIAL_FORMAT_TBL_GET_UNSAFE                      (2467|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PP_IRR_MCDB_EGRESS_SPECIAL_FORMAT_TBL_SET_UNSAFE                      (2468|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PP_IRR_MCDB_EGRESS_TDM_FORMAT_TBL_GET_UNSAFE                          (2469|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PP_IRR_MCDB_EGRESS_TDM_FORMAT_TBL_SET_UNSAFE                          (2470|JER2_ARAD_PROC_BITS)

/*
 * jer2_arad_sw_db {
 */

/*
 * jer2_arad_sw_db }
 */

#define JER2_ARAD_INTERNAL_FUNCS_BASE                                                   ((2500))

#define JER2_ARAD_INGRESS_SCHEDULER_CLOS_WEIGHTS_SET                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE +    1)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_INGRESS_SCHEDULER_CLOS_GLOBAL_SHAPERS_SET                             ((JER2_ARAD_INTERNAL_FUNCS_BASE +    2)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_INGRESS_SCHEDULER_CLOS_HP_SHAPERS_SET                                 ((JER2_ARAD_INTERNAL_FUNCS_BASE +    3)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_INGRESS_SCHEDULER_SHAPER_VALUES_SET                                   ((JER2_ARAD_INTERNAL_FUNCS_BASE +    4)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_INGRESS_SCHEDULER_WEIGHT_SET                                          ((JER2_ARAD_INTERNAL_FUNCS_BASE +    5)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_INGRESS_SCHEDULER_RATE_TO_DELAY_CAL_FORM                              ((JER2_ARAD_INTERNAL_FUNCS_BASE +    6)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_INGRESS_SCHEDULER_MESH_REG_FLDS_DB_GET                                ((JER2_ARAD_INTERNAL_FUNCS_BASE +    7)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_INGRESS_SCHEDULER_WEIGHT_GET                                          ((JER2_ARAD_INTERNAL_FUNCS_BASE +    8)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_INGRESS_SCHEDULER_SHAPER_VALUES_GET                                   ((JER2_ARAD_INTERNAL_FUNCS_BASE +    9)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_INGRESS_SCHEDULER_DELAY_CAL_TO_MAX_RATE_FORM                          ((JER2_ARAD_INTERNAL_FUNCS_BASE +   10)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_INGRESS_SCHEDULER_CLOS_WEIGHTS_GET                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE +   11)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_INGRESS_SCHEDULER_CLOS_GLOBAL_SHAPERS_GET                             ((JER2_ARAD_INTERNAL_FUNCS_BASE +   12)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_INGRESS_SCHEDULER_CLOS_HP_SHAPERS_GET                                 ((JER2_ARAD_INTERNAL_FUNCS_BASE +   13)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_INGRESS_SCHEDULER_CLOS_LP_SHAPERS_SET                                 ((JER2_ARAD_INTERNAL_FUNCS_BASE +   14)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_INGRESS_SCHEDULER_CLOS_LP_SHAPERS_GET                                 ((JER2_ARAD_INTERNAL_FUNCS_BASE +   15)|JER2_ARAD_PROC_BITS)

#define JER2_ARAD_SCH_PORT_SCHED_SET                                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE +   16)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_PORT_HP_CLASS_CONF_SET                                            ((JER2_ARAD_INTERNAL_FUNCS_BASE +   17)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_PORT_SCHED_GET                                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE +   18)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_FLOW_SET                                                          ((JER2_ARAD_INTERNAL_FUNCS_BASE +   19)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_FLOW_GET                                                          ((JER2_ARAD_INTERNAL_FUNCS_BASE +   20)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_FLOW_STATUS_SET                                                   ((JER2_ARAD_INTERNAL_FUNCS_BASE +   21)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_PER1K_INFO_SET                                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE +   22)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_PER1K_INFO_GET                                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE +   23)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_FLOW_TO_QUEUE_MAPPING_SET                                         ((JER2_ARAD_INTERNAL_FUNCS_BASE +   24)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_FLOW_TO_QUEUE_MAPPING_GET                                         ((JER2_ARAD_INTERNAL_FUNCS_BASE +   25)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_SIMPLE_FLOW_ID_VERIFY_UNSAFE                                      ((JER2_ARAD_INTERNAL_FUNCS_BASE +   26)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_SE_ID_VERIFY_UNSAFE                                               ((JER2_ARAD_INTERNAL_FUNCS_BASE +   27)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_PORT_ID_VERIFY_UNSAFE                                             ((JER2_ARAD_INTERNAL_FUNCS_BASE +   28)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_K_FLOW_ID_VERIFY_UNSAFE                                           ((JER2_ARAD_INTERNAL_FUNCS_BASE +   29)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_QUARTET_ID_VERIFY_UNSAFE                                          ((JER2_ARAD_INTERNAL_FUNCS_BASE +   30)|JER2_ARAD_PROC_BITS)

#define JER2_ARAD_SCH_FLOW_SLOW_ENABLE_SET                                              ((JER2_ARAD_INTERNAL_FUNCS_BASE +   33)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_IF_SHAPER_RATE_GET_UNSAFE                                         ((JER2_ARAD_INTERNAL_FUNCS_BASE +   34)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_IF_SHAPER_RATE_SET_UNSAFE                                         ((JER2_ARAD_INTERNAL_FUNCS_BASE +   35)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_DEVICE_IF_WEIGHT_GET_UNSAFE                                       ((JER2_ARAD_INTERNAL_FUNCS_BASE +   36)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_FLOW_GET_UNSAFE                                                   ((JER2_ARAD_INTERNAL_FUNCS_BASE +   37)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_FLOW_NOF_SUBFLOWS_GET                                             ((JER2_ARAD_INTERNAL_FUNCS_BASE +   38)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_FLOW_NOF_SUBFLOWS_SET                                             ((JER2_ARAD_INTERNAL_FUNCS_BASE +   39)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_FLOW_SET_UNSAFE                                                   ((JER2_ARAD_INTERNAL_FUNCS_BASE +   40)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_FLOW_SLOW_ENABLE_GET                                              ((JER2_ARAD_INTERNAL_FUNCS_BASE +   41)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_FLOW_STATUS_SET_UNSAFE                                            ((JER2_ARAD_INTERNAL_FUNCS_BASE +   42)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_FLOW_STATUS_VERIFY                                                ((JER2_ARAD_INTERNAL_FUNCS_BASE +   43)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_FLOW_SUBFLOW_GET                                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE +   44)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_FLOW_SUBFLOW_SET                                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE +   45)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_FLOW_TO_QUEUE_MAPPING_GET_UNSAFE                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE +   46)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_FLOW_TO_QUEUE_MAPPING_SET_UNSAFE                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE +   47)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_FLOW_TO_QUEUE_MAPPING_VERIFY                                      ((JER2_ARAD_INTERNAL_FUNCS_BASE +   48)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_FLOW_VERIFY_UNSAFE                                                ((JER2_ARAD_INTERNAL_FUNCS_BASE +   49)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_FROM_INTERNAL_CL_SUBFLOW_CONVERT                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE +   50)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_FROM_INTERNAL_CL_WEIGHT_CONVERT                                   ((JER2_ARAD_INTERNAL_FUNCS_BASE +   51)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_FROM_INTERNAL_HR_SUBFLOW_CONVERT                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE +   52)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_FROM_INTERNAL_HR_WEIGHT_CONVERT                                   ((JER2_ARAD_INTERNAL_FUNCS_BASE +   53)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_GROUP_TO_PORT_ASSIGN                                              ((JER2_ARAD_INTERNAL_FUNCS_BASE +   54)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_GROUP_TO_SE_ASSIGN                                                ((JER2_ARAD_INTERNAL_FUNCS_BASE +   55)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_HR_LOWEST_HP_CLASS_SELECT_GET                                     ((JER2_ARAD_INTERNAL_FUNCS_BASE +   56)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_HR_LOWEST_HP_CLASS_SELECT_SET                                     ((JER2_ARAD_INTERNAL_FUNCS_BASE +   57)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_INTERNAL_HR_MODE_TO_HR_MODE_CONVERT                               ((JER2_ARAD_INTERNAL_FUNCS_BASE +   58)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_INTERNAL_SUB_FLOW_TO_SUB_FLOW_CONVERT                             ((JER2_ARAD_INTERNAL_FUNCS_BASE +   59)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_IS_SUBFLOW_VALID                                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE +   60)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_NOF_QUARTETS_TO_MAP_GET                                           ((JER2_ARAD_INTERNAL_FUNCS_BASE +   61)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_PER1K_INFO_GET_UNSAFE                                             ((JER2_ARAD_INTERNAL_FUNCS_BASE +   62)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_PER1K_INFO_SET_UNSAFE                                             ((JER2_ARAD_INTERNAL_FUNCS_BASE +   63)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_PER1K_INFO_VERIFY                                                 ((JER2_ARAD_INTERNAL_FUNCS_BASE +   64)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_PORT_HP_CLASS_CONF_GET_UNSAFE                                     ((JER2_ARAD_INTERNAL_FUNCS_BASE +   65)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_PORT_HP_CLASS_CONF_SET_UNSAFE                                     ((JER2_ARAD_INTERNAL_FUNCS_BASE +   66)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_PORT_HP_CLASS_CONF_VERIFY                                         ((JER2_ARAD_INTERNAL_FUNCS_BASE +   67)|JER2_ARAD_PROC_BITS)

#define JER2_ARAD_SCH_SE_CONFIG_GET                                                     ((JER2_ARAD_INTERNAL_FUNCS_BASE +   71)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_SE_CONFIG_SET                                                     ((JER2_ARAD_INTERNAL_FUNCS_BASE +   72)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_SE_DUAL_SHAPER_GET                                                ((JER2_ARAD_INTERNAL_FUNCS_BASE +   73)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_SE_GET_UNSAFE                                                     ((JER2_ARAD_INTERNAL_FUNCS_BASE +   74)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_SE_GROUP_GET                                                      ((JER2_ARAD_INTERNAL_FUNCS_BASE +   75)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_SE_GROUP_SET                                                      ((JER2_ARAD_INTERNAL_FUNCS_BASE +   76)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_SE_ID_AND_TYPE_MATCH_VERIFY                                       ((JER2_ARAD_INTERNAL_FUNCS_BASE +   77)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_SE_SET_UNSAFE                                                     ((JER2_ARAD_INTERNAL_FUNCS_BASE +   78)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_SE_STATE_GET                                                      ((JER2_ARAD_INTERNAL_FUNCS_BASE +   79)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_SE_STATE_SET                                                      ((JER2_ARAD_INTERNAL_FUNCS_BASE +   80)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_SE_VERIFY_UNSAFE                                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE +   81)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_SUB_FLOW_TO_INTERNAL_SUB_FLOW_CONVERT                             ((JER2_ARAD_INTERNAL_FUNCS_BASE +   82)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_SUBFLOWS_VERIFY_UNSAFE                                            ((JER2_ARAD_INTERNAL_FUNCS_BASE +   83)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_TO_INTERNAL_CL_SUBFLOW_CONVERT                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE +   84)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_TO_INTERNAL_CL_WEIGHT_CONVERT                                     ((JER2_ARAD_INTERNAL_FUNCS_BASE +   85)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_TO_INTERNAL_HR_WEIGHT_CONVERT                                     ((JER2_ARAD_INTERNAL_FUNCS_BASE +   86)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_TO_INTERNAL_SUBFLOW_SHAPER_CONVERT                                ((JER2_ARAD_INTERNAL_FUNCS_BASE +   87)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_DEVICE_IF_WEIGHT_SET_UNSAFE                                       ((JER2_ARAD_INTERNAL_FUNCS_BASE +   88)|JER2_ARAD_PROC_BITS)

#define JER2_ARAD_INTERN_RATE2CLOCK                                                     ((JER2_ARAD_INTERNAL_FUNCS_BASE +  121)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_INTERN_CLOCK2RATE                                                     ((JER2_ARAD_INTERNAL_FUNCS_BASE +  122)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_FABRIC_CREDIT_SOURCE_PORT_SET                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE +  127)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_FABRIC_CREDIT_SOURCE_PORT_GET                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE +  128)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_FABRIC_CREDIT_SOURCE_SHAPER_SET                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE +  129)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_FABRIC_CREDIT_SOURCE_SHAPER_GET                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE +  130)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_FABRIC_CREDIT_SOURCE_BE_WFQ_SET                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE +  131)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_FABRIC_CREDIT_SOURCE_BE_WFQ_GET                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE +  132)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_FABRIC_CREDIT_SOURCE_BE_SET                                      ((JER2_ARAD_INTERNAL_FUNCS_BASE +  133)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_FABRIC_CREDIT_SOURCE_BE_GET                                      ((JER2_ARAD_INTERNAL_FUNCS_BASE +  134)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_FABRIC_CREDIT_SOURCE_GU_SET                                      ((JER2_ARAD_INTERNAL_FUNCS_BASE +  135)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_FABRIC_CREDIT_SOURCE_GU_GET                                      ((JER2_ARAD_INTERNAL_FUNCS_BASE +  136)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_FABRIC_ACTIVE_LINKS_VERIFY                                       ((JER2_ARAD_INTERNAL_FUNCS_BASE +  137)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_FABRIC_ACTIVE_LINKS_SET_UNSAFE                                   ((JER2_ARAD_INTERNAL_FUNCS_BASE +  138)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_FABRIC_ACTIVE_LINKS_GET_UNSAFE                                   ((JER2_ARAD_INTERNAL_FUNCS_BASE +  139)|JER2_ARAD_PROC_BITS)

#define JER2_ARAD_MULT_MULTICAST_GROUP_INPUT                                            ((JER2_ARAD_INTERNAL_FUNCS_BASE +  140)|JER2_ARAD_PROC_BITS)

#define JER2_ARAD_MULT_MULTICAST_GROUP_ENTRY_TO_TBL                                     ((JER2_ARAD_INTERNAL_FUNCS_BASE +  141)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_MULTICAST_ASSERT_INGRESS_REPLICATION_HW_TABLE                    ((JER2_ARAD_INTERNAL_FUNCS_BASE +  142)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_MULTICAST_ID_RELOCATION                                          ((JER2_ARAD_INTERNAL_FUNCS_BASE +  143)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_ENTRY_CONTENT_GET                                                ((JER2_ARAD_INTERNAL_FUNCS_BASE +  144)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_ING_ENTRY_CONTENT_GET                                            ((JER2_ARAD_INTERNAL_FUNCS_BASE +  145)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_EG_ENTRY_CONTENT_GET                                             ((JER2_ARAD_INTERNAL_FUNCS_BASE +  146)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_ING_PTR_TO_OLD_LIST_GET                                          ((JER2_ARAD_INTERNAL_FUNCS_BASE +  147)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_GROUP_UPDATE_UNSAFE_JOINT                                        ((JER2_ARAD_INTERNAL_FUNCS_BASE +  149)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_LINK_LIST_PTR_GET                                                ((JER2_ARAD_INTERNAL_FUNCS_BASE +  150)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_ERASE_MULTICAST_GROUP                                            ((JER2_ARAD_INTERNAL_FUNCS_BASE +  151)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_ERASE_ONE_ENTRY                                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE +  152)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_GROUP_CLOSE_UNSAFE_JOINT                                         ((JER2_ARAD_INTERNAL_FUNCS_BASE +  153)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_EG_FILL_IN_LAST_ENTRY                                            ((JER2_ARAD_INTERNAL_FUNCS_BASE +  154)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_ING_FILL_IN_LAST_ENTRY                                           ((JER2_ARAD_INTERNAL_FUNCS_BASE +  155)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_ING_DESTINATION_ADD_UNSAFE_INNER                                 ((JER2_ARAD_INTERNAL_FUNCS_BASE +  156)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_LAST_ENTRY_IN_LIST_GET                                           ((JER2_ARAD_INTERNAL_FUNCS_BASE +  157)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_ADD_ENTRY_IN_END_OF_LINK_LIST                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE +  158)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_FILL_IN_LAST_ENTRY                                               ((JER2_ARAD_INTERNAL_FUNCS_BASE +  159)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_EG_NOF_OCCUPIED_ELEMENTS_IN_TBL_ENTRY                            ((JER2_ARAD_INTERNAL_FUNCS_BASE +  160)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_ING_NOF_OCCUPIED_ELEMENTS_IN_TBL_ENTRY                           ((JER2_ARAD_INTERNAL_FUNCS_BASE +  161)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_ING_GROUP_SIZE_GET_UNSAFE_INNER                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE +  162)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_NOF_OCCUPIED_ELEMENTS_IN_TBL_ENTRY                               ((JER2_ARAD_INTERNAL_FUNCS_BASE +  163)|JER2_ARAD_PROC_BITS)

#define JER2_ARAD_NIF_MAL_BASIC_CONF_SET                                                ((JER2_ARAD_INTERNAL_FUNCS_BASE +  164)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_MAL_BASIC_CONF_GET                                                ((JER2_ARAD_INTERNAL_FUNCS_BASE +  165)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_TOPOLOGY_VALIDATE                                                 ((JER2_ARAD_INTERNAL_FUNCS_BASE +  166)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_MAL_ENABLE_SET                                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE +  167)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_MAL_ENABLE_GET                                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE +  168)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_MIN_PACKET_SIZE_SET                                               ((JER2_ARAD_INTERNAL_FUNCS_BASE +  169)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_MIN_PACKET_SIZE_GET                                               ((JER2_ARAD_INTERNAL_FUNCS_BASE +  170)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_MAL_BASIC_CONF_GET_UNSAFE                                         ((JER2_ARAD_INTERNAL_FUNCS_BASE +  171)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_TOPOLOGY_VALIDATE_UNSAFE                                          ((JER2_ARAD_INTERNAL_FUNCS_BASE +  172)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_MAL_ENABLE_SET_UNSAFE                                             ((JER2_ARAD_INTERNAL_FUNCS_BASE +  173)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_MAL_ENABLE_VERIFY                                                 ((JER2_ARAD_INTERNAL_FUNCS_BASE +  174)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_MAL_ENABLE_GET_UNSAFE                                             ((JER2_ARAD_INTERNAL_FUNCS_BASE +  175)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_MIN_PACKET_SIZE_SET_UNSAFE                                        ((JER2_ARAD_INTERNAL_FUNCS_BASE +  176)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_MIN_PACKET_SIZE_VERIFY                                            ((JER2_ARAD_INTERNAL_FUNCS_BASE +  177)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_MIN_PACKET_SIZE_GET_UNSAFE                                        ((JER2_ARAD_INTERNAL_FUNCS_BASE +  178)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_MAL_BASIC_CONF_SET_UNSAFE                                         ((JER2_ARAD_INTERNAL_FUNCS_BASE +  179)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_MAL_BASIC_CONF_VERIFY                                             ((JER2_ARAD_INTERNAL_FUNCS_BASE +  180)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_MAL_BASIC_CONF_XAUI_SET_UNSAFE                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE +  181)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_MAL_BASIC_CONF_SGMII_SET_UNSAFE                                   ((JER2_ARAD_INTERNAL_FUNCS_BASE +  182)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_SPAUI_DEFAULTS_SET_UNSAFE                                         ((JER2_ARAD_INTERNAL_FUNCS_BASE +  183)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_SPAUI_DEFAULTS_GET_UNSAFE                                         ((JER2_ARAD_INTERNAL_FUNCS_BASE +  184)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_FAT_PIPE_SET                                                      ((JER2_ARAD_INTERNAL_FUNCS_BASE +  185)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_FAT_PIPE_GET                                                      ((JER2_ARAD_INTERNAL_FUNCS_BASE +  186)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_SGMII_CLASS_BASED_FC_SET                                          ((JER2_ARAD_INTERNAL_FUNCS_BASE +  187)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_SGMII_CLASS_BASED_FC_GET                                          ((JER2_ARAD_INTERNAL_FUNCS_BASE +  188)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_INBOUND_FC_SET                                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE +  189)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_INBOUND_FC_GET                                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE +  190)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_FAT_PIPE_SET_UNSAFE                                               ((JER2_ARAD_INTERNAL_FUNCS_BASE +  191)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_FAT_PIPE_VERIFY                                                   ((JER2_ARAD_INTERNAL_FUNCS_BASE +  192)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_FAT_PIPE_GET_UNSAFE                                               ((JER2_ARAD_INTERNAL_FUNCS_BASE +  193)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_SGMII_CLASS_BASED_FC_SET_UNSAFE                                   ((JER2_ARAD_INTERNAL_FUNCS_BASE +  194)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_SGMII_CLASS_BASED_FC_VERIFY                                       ((JER2_ARAD_INTERNAL_FUNCS_BASE +  195)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_SGMII_CLASS_BASED_FC_GET_UNSAFE                                   ((JER2_ARAD_INTERNAL_FUNCS_BASE +  196)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_INBOUND_FC_SET_UNSAFE                                             ((JER2_ARAD_INTERNAL_FUNCS_BASE +  197)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_INBOUND_FC_VERIFY                                                 ((JER2_ARAD_INTERNAL_FUNCS_BASE +  198)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_INBOUND_FC_GET_UNSAFE                                             ((JER2_ARAD_INTERNAL_FUNCS_BASE +  199)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_MDIO22_WRITE                                                      ((JER2_ARAD_INTERNAL_FUNCS_BASE +  200)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_MDIO22_READ                                                       ((JER2_ARAD_INTERNAL_FUNCS_BASE +  201)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_MDIO45_WRITE                                                      ((JER2_ARAD_INTERNAL_FUNCS_BASE +  202)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_MDIO45_READ                                                       ((JER2_ARAD_INTERNAL_FUNCS_BASE +  203)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_MDIO22_WRITE_UNSAFE                                               ((JER2_ARAD_INTERNAL_FUNCS_BASE +  204)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_MDIO22_READ_UNSAFE                                                ((JER2_ARAD_INTERNAL_FUNCS_BASE +  205)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_MDIO45_WRITE_UNSAFE                                               ((JER2_ARAD_INTERNAL_FUNCS_BASE +  206)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_MDIO45_READ_UNSAFE                                                ((JER2_ARAD_INTERNAL_FUNCS_BASE +  207)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_TYPE_GET_UNSAFE                                                   ((JER2_ARAD_INTERNAL_FUNCS_BASE +  213)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_TYPE_GET                                                          ((JER2_ARAD_INTERNAL_FUNCS_BASE +  214)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_TYPE_FROM_EGQ_GET                                                 ((JER2_ARAD_INTERNAL_FUNCS_BASE +  215)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FABRIC_FC_ENABLE_SET                                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE +  217)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FABRIC_FC_ENABLE_GET                                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE +  218)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FABRIC_CELL_FORMAT_SET                                                ((JER2_ARAD_INTERNAL_FUNCS_BASE +  219)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FABRIC_CELL_FORMAT_GET                                                ((JER2_ARAD_INTERNAL_FUNCS_BASE +  220)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FABRIC_COEXIST_SET                                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE +  221)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FABRIC_COEXIST_GET                                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE +  222)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FABRIC_CONNECTIVITY_MAP_GET                                           ((JER2_ARAD_INTERNAL_FUNCS_BASE +  223)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FABRIC_STANDALONE_FAP_MODE_DETECT                                     ((JER2_ARAD_INTERNAL_FUNCS_BASE +  224)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FABRIC_CONNECT_MODE_SET                                               ((JER2_ARAD_INTERNAL_FUNCS_BASE +  225)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FABRIC_CONNECT_MODE_GET                                               ((JER2_ARAD_INTERNAL_FUNCS_BASE +  226)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FABRIC_FAP20_MAP_SET                                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE +  227)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FABRIC_FAP20_MAP_GET                                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE +  228)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FABRIC_FC_ENABLE_SET_UNSAFE                                           ((JER2_ARAD_INTERNAL_FUNCS_BASE +  229)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FABRIC_FC_ENABLE_VERIFY                                               ((JER2_ARAD_INTERNAL_FUNCS_BASE +  230)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FABRIC_FC_ENABLE_GET_UNSAFE                                           ((JER2_ARAD_INTERNAL_FUNCS_BASE +  231)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FABRIC_CELL_FORMAT_SET_UNSAFE                                         ((JER2_ARAD_INTERNAL_FUNCS_BASE +  232)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FABRIC_CELL_FORMAT_VERIFY                                             ((JER2_ARAD_INTERNAL_FUNCS_BASE +  233)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FABRIC_CELL_FORMAT_GET_UNSAFE                                         ((JER2_ARAD_INTERNAL_FUNCS_BASE +  234)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FABRIC_COEXIST_SET_UNSAFE                                             ((JER2_ARAD_INTERNAL_FUNCS_BASE +  235)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FABRIC_COEXIST_VERIFY                                                 ((JER2_ARAD_INTERNAL_FUNCS_BASE +  236)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FABRIC_COEXIST_GET_UNSAFE                                             ((JER2_ARAD_INTERNAL_FUNCS_BASE +  237)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FABRIC_CONNECTIVITY_MAP_GET_UNSAFE                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE +  238)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FABRIC_STANDALONE_FAP_MODE_DETECT_UNSAFE                              ((JER2_ARAD_INTERNAL_FUNCS_BASE +  239)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FABRIC_CONNECT_MODE_SET_UNSAFE                                        ((JER2_ARAD_INTERNAL_FUNCS_BASE +  240)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FABRIC_CONNECT_MODE_VERIFY                                            ((JER2_ARAD_INTERNAL_FUNCS_BASE +  241)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FABRIC_CONNECT_MODE_GET_UNSAFE                                        ((JER2_ARAD_INTERNAL_FUNCS_BASE +  242)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FABRIC_FAP20_MAP_SET_UNSAFE                                           ((JER2_ARAD_INTERNAL_FUNCS_BASE +  243)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FABRIC_FAP20_MAP_VERIFY                                               ((JER2_ARAD_INTERNAL_FUNCS_BASE +  244)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FABRIC_FAP20_MAP_GET_UNSAFE                                           ((JER2_ARAD_INTERNAL_FUNCS_BASE +  245)|JER2_ARAD_PROC_BITS)

#define JER2_ARAD_MULT_EG_MULTICAST_GROUP_ENTRY_TO_TBL                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE +  246)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_ING_MULTICAST_GROUP_ENTRY_TO_TBL                                 ((JER2_ARAD_INTERNAL_FUNCS_BASE +  247)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_ING_NEXT_LINK_LIST_PTR_SET                                       ((JER2_ARAD_INTERNAL_FUNCS_BASE +  249)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_ING_NEXT_LINK_LIST_PTR_GET                                       ((JER2_ARAD_INTERNAL_FUNCS_BASE +  250)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_RPLCT_TBL_ENTRY_OCCUPIED_BUT_EMPTY_SET                           ((JER2_ARAD_INTERNAL_FUNCS_BASE +  251)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MGMT_INIT_SEQUENCE_PHASE1_UNSAFE                                      ((JER2_ARAD_INTERNAL_FUNCS_BASE +  252)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MGMT_INIT_SEQUENCE_PHASE2_UNSAFE                                      ((JER2_ARAD_INTERNAL_FUNCS_BASE +  253)|JER2_ARAD_PROC_BITS)

#define JER2_ARAD_FABRIC_LINK_STATUS_GET                                                ((JER2_ARAD_INTERNAL_FUNCS_BASE +  254)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FABRIC_LINK_STATUS_GET_UNSAFE                                         ((JER2_ARAD_INTERNAL_FUNCS_BASE +  255)|JER2_ARAD_PROC_BITS)

#define JER2_ARAD_ITM_VSQ_CONVERT_GROUP_INDX_TO_GLOBAL_INDX                             ((JER2_ARAD_INTERNAL_FUNCS_BASE +  257)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_VSQ_GROUP_SET_FC_INFO                                             ((JER2_ARAD_INTERNAL_FUNCS_BASE +  258)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_VSQ_GROUP_GET_FC_INFO                                             ((JER2_ARAD_INTERNAL_FUNCS_BASE +  259)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_CONVERT_ADMIT_ONE_TEST_TMPLT_TO_U32                               ((JER2_ARAD_INTERNAL_FUNCS_BASE +  260)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_CONVERT_U32_TO_ADMIT_ONE_TEST_TMPLT                               ((JER2_ARAD_INTERNAL_FUNCS_BASE +  261)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_CR_REQUEST_INFO_HUNGRY_TABLE_FIELD_SET                            ((JER2_ARAD_INTERNAL_FUNCS_BASE +  262)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_CR_REQUEST_INFO_SATISFIED_MNT_EXP_TABLE_FIELD_SET                 ((JER2_ARAD_INTERNAL_FUNCS_BASE +  263)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_MAN_EXP_BUFFER_SET                                                ((JER2_ARAD_INTERNAL_FUNCS_BASE +  264)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_CR_REQUEST_INFO_HUNGRY_TABLE_FIELD_GET                            ((JER2_ARAD_INTERNAL_FUNCS_BASE +  265)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_CR_REQUEST_INFO_SATISFIED_MNT_EXP_TABLE_FIELD_GET                 ((JER2_ARAD_INTERNAL_FUNCS_BASE +  266)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_MAN_EXP_BUFFER_GET                                                ((JER2_ARAD_INTERNAL_FUNCS_BASE +  267)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_WRED_QT_DP_INFO_TO_WRED_TBL_DATA                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE +  268)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_WRED_TBL_DATA_TO_WRED_QT_DP_INFO                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE +  269)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_VSQ_CONVERT_GLOBAL_INDX_TO_CTGRY_INDX                             ((JER2_ARAD_INTERNAL_FUNCS_BASE +  270)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_VSQ_GROUP_A_SET_RT_CLASS                                          ((JER2_ARAD_INTERNAL_FUNCS_BASE +  271)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_VSQ_GROUP_B_SET_RT_CLASS                                          ((JER2_ARAD_INTERNAL_FUNCS_BASE +  272)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_VSQ_GROUP_C_SET_RT_CLASS                                          ((JER2_ARAD_INTERNAL_FUNCS_BASE +  273)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_VSQ_GROUP_D_SET_RT_CLASS                                          ((JER2_ARAD_INTERNAL_FUNCS_BASE +  274)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_VSQ_GROUP_A_GET_RT_CLASS                                          ((JER2_ARAD_INTERNAL_FUNCS_BASE +  275)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_VSQ_GROUP_B_GET_RT_CLASS                                          ((JER2_ARAD_INTERNAL_FUNCS_BASE +  276)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_VSQ_GROUP_C_GET_RT_CLASS                                          ((JER2_ARAD_INTERNAL_FUNCS_BASE +  277)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_VSQ_GROUP_D_GET_RT_CLASS                                          ((JER2_ARAD_INTERNAL_FUNCS_BASE +  278)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_VSQ_WRED_QT_DP_INFO_TO_WRED_TBL_DATA                              ((JER2_ARAD_INTERNAL_FUNCS_BASE +  279)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_VSQ_WRED_TBL_DATA_TO_WRED_QT_DP_INFO                              ((JER2_ARAD_INTERNAL_FUNCS_BASE +  280)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_VSQ_WRED_GROUP_SET_INFO                                           ((JER2_ARAD_INTERNAL_FUNCS_BASE +  281)|JER2_ARAD_PROC_BITS)

#define JER2_ARAD_SCH_SE_DUAL_SHAPER_SET                                                ((JER2_ARAD_INTERNAL_FUNCS_BASE +  282)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FIELD_FROM_REG_GET                                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE +  283)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FIELD_FROM_REG_SET                                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE +  284)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGR_OFP_THRESH_TYPE_SET                                               ((JER2_ARAD_INTERNAL_FUNCS_BASE +  285)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGR_OFP_THRESH_TYPE_GET                                               ((JER2_ARAD_INTERNAL_FUNCS_BASE +  286)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGR_SCHED_DROP_SET                                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE +  287)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGR_SCHED_DROP_GET                                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE +  288)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGR_UNSCHED_DROP_SET                                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE +  289)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGR_UNSCHED_DROP_GET                                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE +  290)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGR_DEV_FC_SET                                                        ((JER2_ARAD_INTERNAL_FUNCS_BASE +  291)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGR_DEV_FC_GET                                                        ((JER2_ARAD_INTERNAL_FUNCS_BASE +  292)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGR_CHNIF_FC_SET                                                      ((JER2_ARAD_INTERNAL_FUNCS_BASE +  293)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGR_CHNIF_FC_GET                                                      ((JER2_ARAD_INTERNAL_FUNCS_BASE +  294)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGR_OFP_FC_SET                                                        ((JER2_ARAD_INTERNAL_FUNCS_BASE +  295)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGR_OFP_FC_GET                                                        ((JER2_ARAD_INTERNAL_FUNCS_BASE +  296)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGR_MCI_FC_SET                                                        ((JER2_ARAD_INTERNAL_FUNCS_BASE +  297)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGR_MCI_FC_GET                                                        ((JER2_ARAD_INTERNAL_FUNCS_BASE +  298)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGR_MCI_FC_ENABLE_SET                                                 ((JER2_ARAD_INTERNAL_FUNCS_BASE +  299)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGR_MCI_FC_ENABLE_GET                                                 ((JER2_ARAD_INTERNAL_FUNCS_BASE +  300)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGR_OFP_SCHEDULING_SET                                                ((JER2_ARAD_INTERNAL_FUNCS_BASE +  303)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGR_OFP_SCHEDULING_GET                                                ((JER2_ARAD_INTERNAL_FUNCS_BASE +  304)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGR_OFP_THRESH_TYPE_SET_UNSAFE                                        ((JER2_ARAD_INTERNAL_FUNCS_BASE +  314)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGR_OFP_THRESH_TYPE_VERIFY                                            ((JER2_ARAD_INTERNAL_FUNCS_BASE +  315)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGR_OFP_THRESH_TYPE_GET_UNSAFE                                        ((JER2_ARAD_INTERNAL_FUNCS_BASE +  316)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGR_SCHED_DROP_SET_UNSAFE                                             ((JER2_ARAD_INTERNAL_FUNCS_BASE +  317)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGR_SCHED_DROP_VERIFY                                                 ((JER2_ARAD_INTERNAL_FUNCS_BASE +  318)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGR_SCHED_DROP_GET_UNSAFE                                             ((JER2_ARAD_INTERNAL_FUNCS_BASE +  319)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGR_UNSCHED_DROP_SET_UNSAFE                                           ((JER2_ARAD_INTERNAL_FUNCS_BASE +  320)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGR_UNSCHED_DROP_VERIFY                                               ((JER2_ARAD_INTERNAL_FUNCS_BASE +  321)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGR_UNSCHED_DROP_GET_UNSAFE                                           ((JER2_ARAD_INTERNAL_FUNCS_BASE +  322)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGR_DEV_FC_SET_UNSAFE                                                 ((JER2_ARAD_INTERNAL_FUNCS_BASE +  323)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGR_DEV_FC_VERIFY                                                     ((JER2_ARAD_INTERNAL_FUNCS_BASE +  324)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGR_DEV_FC_GET_UNSAFE                                                 ((JER2_ARAD_INTERNAL_FUNCS_BASE +  325)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGR_CHNIF_FC_SET_UNSAFE                                               ((JER2_ARAD_INTERNAL_FUNCS_BASE +  326)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGR_CHNIF_FC_VERIFY                                                   ((JER2_ARAD_INTERNAL_FUNCS_BASE +  327)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGR_CHNIF_FC_GET_UNSAFE                                               ((JER2_ARAD_INTERNAL_FUNCS_BASE +  328)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGR_OFP_FC_SET_UNSAFE                                                 ((JER2_ARAD_INTERNAL_FUNCS_BASE +  329)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGR_OFP_FC_VERIFY                                                     ((JER2_ARAD_INTERNAL_FUNCS_BASE +  330)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGR_OFP_FC_GET_UNSAFE                                                 ((JER2_ARAD_INTERNAL_FUNCS_BASE +  331)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGR_MCI_FC_SET_UNSAFE                                                 ((JER2_ARAD_INTERNAL_FUNCS_BASE +  332)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGR_MCI_FC_VERIFY                                                     ((JER2_ARAD_INTERNAL_FUNCS_BASE +  333)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGR_MCI_FC_GET_UNSAFE                                                 ((JER2_ARAD_INTERNAL_FUNCS_BASE +  334)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGR_MCI_FC_ENABLE_SET_UNSAFE                                          ((JER2_ARAD_INTERNAL_FUNCS_BASE +  335)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGR_MCI_FC_ENABLE_VERIFY                                              ((JER2_ARAD_INTERNAL_FUNCS_BASE +  336)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGR_MCI_FC_ENABLE_GET_UNSAFE                                          ((JER2_ARAD_INTERNAL_FUNCS_BASE +  337)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGR_OFP_SCHEDULING_SET_UNSAFE                                         ((JER2_ARAD_INTERNAL_FUNCS_BASE +  341)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGR_OFP_SCHEDULING_VERIFY                                             ((JER2_ARAD_INTERNAL_FUNCS_BASE +  342)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGR_OFP_SCHEDULING_GET_UNSAFE                                         ((JER2_ARAD_INTERNAL_FUNCS_BASE +  343)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGR_THRESH_TO_MNT_EXP                                                 ((JER2_ARAD_INTERNAL_FUNCS_BASE +  347)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGR_MNT_EXP_TO_THRESH                                                 ((JER2_ARAD_INTERNAL_FUNCS_BASE +  348)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGR_THRESH_FLD_TO_MNT_EXP                                             ((JER2_ARAD_INTERNAL_FUNCS_BASE +  349)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGR_MNT_EXP_TO_THRESH_FLD                                             ((JER2_ARAD_INTERNAL_FUNCS_BASE +  350)|JER2_ARAD_PROC_BITS)

#define JER2_ARAD_NIF_PAUSE_QUANTA_SET                                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE +  352)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_PAUSE_QUANTA_GET                                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE +  353)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_PAUSE_FRAME_SRC_ADDR_SET                                          ((JER2_ARAD_INTERNAL_FUNCS_BASE +  354)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_PAUSE_FRAME_SRC_ADDR_GET                                          ((JER2_ARAD_INTERNAL_FUNCS_BASE +  355)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_PAUSE_QUANTA_SET_UNSAFE                                           ((JER2_ARAD_INTERNAL_FUNCS_BASE +  356)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_PAUSE_QUANTA_VERIFY                                               ((JER2_ARAD_INTERNAL_FUNCS_BASE +  357)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_PAUSE_QUANTA_GET_UNSAFE                                           ((JER2_ARAD_INTERNAL_FUNCS_BASE +  358)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_PAUSE_FRAME_SRC_ADDR_SET_UNSAFE                                   ((JER2_ARAD_INTERNAL_FUNCS_BASE +  359)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_PAUSE_FRAME_SRC_ADDR_VERIFY                                       ((JER2_ARAD_INTERNAL_FUNCS_BASE +  360)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_PAUSE_FRAME_SRC_ADDR_GET_UNSAFE                                   ((JER2_ARAD_INTERNAL_FUNCS_BASE +  361)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FC_INGR_GEN_VSQ_VIA_NIF_SET                                           ((JER2_ARAD_INTERNAL_FUNCS_BASE +  362)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FC_INGR_GEN_VSQ_VIA_NIF_GET                                           ((JER2_ARAD_INTERNAL_FUNCS_BASE +  363)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FC_INGR_GEN_GLB_HP_VIA_NIF_SET                                        ((JER2_ARAD_INTERNAL_FUNCS_BASE +  364)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FC_INGR_GEN_GLB_HP_VIA_NIF_GET                                        ((JER2_ARAD_INTERNAL_FUNCS_BASE +  365)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FC_INGR_GEN_GLB_LP_VIA_NIF_CB_SET                                     ((JER2_ARAD_INTERNAL_FUNCS_BASE +  366)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FC_INGR_GEN_GLB_LP_VIA_NIF_CB_GET                                     ((JER2_ARAD_INTERNAL_FUNCS_BASE +  367)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FC_INGR_GEN_OOB_SET                                                   ((JER2_ARAD_INTERNAL_FUNCS_BASE +  368)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FC_INGR_GEN_OOB_GET                                                   ((JER2_ARAD_INTERNAL_FUNCS_BASE +  369)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FC_EGR_REC_VIA_NIF_SET                                                ((JER2_ARAD_INTERNAL_FUNCS_BASE +  371)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FC_EGR_REC_VIA_NIF_GET                                                ((JER2_ARAD_INTERNAL_FUNCS_BASE +  372)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FC_EGR_REC_OOB_SET                                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE +  373)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FC_EGR_REC_OOB_GET                                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE +  374)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FC_RCY_VSQ_OFP_SET                                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE +  376)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FC_RCY_VSQ_OFP_GET                                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE +  377)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FC_RCY_GLB_OFP_HR_SET                                                 ((JER2_ARAD_INTERNAL_FUNCS_BASE +  378)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FC_RCY_GLB_OFP_HR_GET                                                 ((JER2_ARAD_INTERNAL_FUNCS_BASE +  379)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FC_RCY_HR_ENABLE_SET                                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE +  380)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FC_RCY_HR_ENABLE_GET                                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE +  381)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FC_INGR_GEN_VSQ_VIA_NIF_SET_UNSAFE                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE +  384)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FC_INGR_GEN_VSQ_VIA_NIF_VERIFY                                        ((JER2_ARAD_INTERNAL_FUNCS_BASE +  385)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FC_INGR_GEN_VSQ_VIA_NIF_GET_UNSAFE                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE +  386)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FC_INGR_GEN_GLB_HP_VIA_NIF_SET_UNSAFE                                 ((JER2_ARAD_INTERNAL_FUNCS_BASE +  387)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FC_INGR_GEN_GLB_HP_VIA_NIF_VERIFY                                     ((JER2_ARAD_INTERNAL_FUNCS_BASE +  388)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FC_INGR_GEN_GLB_HP_VIA_NIF_GET_UNSAFE                                 ((JER2_ARAD_INTERNAL_FUNCS_BASE +  389)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FC_INGR_GEN_GLB_LP_VIA_NIF_CB_SET_UNSAFE                              ((JER2_ARAD_INTERNAL_FUNCS_BASE +  390)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FC_INGR_GEN_GLB_LP_VIA_NIF_CB_VERIFY                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE +  391)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FC_INGR_GEN_GLB_LP_VIA_NIF_CB_GET_UNSAFE                              ((JER2_ARAD_INTERNAL_FUNCS_BASE +  392)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FC_INGR_GEN_OOB_SET_UNSAFE                                            ((JER2_ARAD_INTERNAL_FUNCS_BASE +  393)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FC_INGR_GEN_OOB_VERIFY                                                ((JER2_ARAD_INTERNAL_FUNCS_BASE +  394)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FC_INGR_GEN_OOB_GET_UNSAFE                                            ((JER2_ARAD_INTERNAL_FUNCS_BASE +  395)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FC_EGR_REC_VIA_NIF_SET_UNSAFE                                         ((JER2_ARAD_INTERNAL_FUNCS_BASE +  397)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FC_EGR_REC_VIA_NIF_VERIFY                                             ((JER2_ARAD_INTERNAL_FUNCS_BASE +  398)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FC_EGR_REC_VIA_NIF_GET_UNSAFE                                         ((JER2_ARAD_INTERNAL_FUNCS_BASE +  399)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FC_EGR_REC_OOB_SET_UNSAFE                                             ((JER2_ARAD_INTERNAL_FUNCS_BASE +  400)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FC_EGR_REC_OOB_VERIFY                                                 ((JER2_ARAD_INTERNAL_FUNCS_BASE +  401)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FC_EGR_REC_OOB_GET_UNSAFE                                             ((JER2_ARAD_INTERNAL_FUNCS_BASE +  402)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FC_RCY_VSQ_OFP_SET_UNSAFE                                             ((JER2_ARAD_INTERNAL_FUNCS_BASE +  404)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FC_RCY_VSQ_OFP_VERIFY                                                 ((JER2_ARAD_INTERNAL_FUNCS_BASE +  405)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FC_RCY_VSQ_OFP_GET_UNSAFE                                             ((JER2_ARAD_INTERNAL_FUNCS_BASE +  406)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FC_RCY_GLB_OFP_HR_SET_UNSAFE                                          ((JER2_ARAD_INTERNAL_FUNCS_BASE +  407)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FC_RCY_GLB_OFP_HR_VERIFY                                              ((JER2_ARAD_INTERNAL_FUNCS_BASE +  408)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FC_RCY_GLB_OFP_HR_GET_UNSAFE                                          ((JER2_ARAD_INTERNAL_FUNCS_BASE +  409)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FC_RCY_HR_ENABLE_SET_UNSAFE                                           ((JER2_ARAD_INTERNAL_FUNCS_BASE +  410)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FC_RCY_HR_ENABLE_VERIFY                                               ((JER2_ARAD_INTERNAL_FUNCS_BASE +  411)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FC_RCY_HR_ENABLE_GET_UNSAFE                                           ((JER2_ARAD_INTERNAL_FUNCS_BASE +  412)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FC_RCY_ON_GLB_RCS_OVERRIDE_SET                                        ((JER2_ARAD_INTERNAL_FUNCS_BASE +  413)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FC_RCY_ON_GLB_RCS_OVERRIDE_GET                                        ((JER2_ARAD_INTERNAL_FUNCS_BASE +  414)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FC_RCY_ON_GLB_RCS_OVERRIDE_SET_UNSAFE                                 ((JER2_ARAD_INTERNAL_FUNCS_BASE +  415)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FC_RCY_ON_GLB_RCS_OVERRIDE_VERIFY                                     ((JER2_ARAD_INTERNAL_FUNCS_BASE +  416)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FC_RCY_ON_GLB_RCS_OVERRIDE_GET_UNSAFE                                 ((JER2_ARAD_INTERNAL_FUNCS_BASE +  417)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_FC_SET                                                            ((JER2_ARAD_INTERNAL_FUNCS_BASE +  418)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_FC_GET                                                            ((JER2_ARAD_INTERNAL_FUNCS_BASE +  419)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_FC_SET_UNSAFE                                                     ((JER2_ARAD_INTERNAL_FUNCS_BASE +  420)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_FC_LL_VERIFY                                                      ((JER2_ARAD_INTERNAL_FUNCS_BASE +  421)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_FC_GET_UNSAFE                                                     ((JER2_ARAD_INTERNAL_FUNCS_BASE +  422)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IHP_PROG_N00_LOAD_UNSAFE                                              ((JER2_ARAD_INTERNAL_FUNCS_BASE +  423)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IHP_PROG_N00_LOAD                                                     ((JER2_ARAD_INTERNAL_FUNCS_BASE +  424)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IHP_PORT_SET_UNSAFE                                                   ((JER2_ARAD_INTERNAL_FUNCS_BASE +  425)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IHP_PORT_VERIFY                                                       ((JER2_ARAD_INTERNAL_FUNCS_BASE +  426)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IHP_PORT_GET_UNSAFE                                                   ((JER2_ARAD_INTERNAL_FUNCS_BASE +  427)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IHP_PORT_SET                                                          ((JER2_ARAD_INTERNAL_FUNCS_BASE +  428)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IHP_PORT_GET                                                          ((JER2_ARAD_INTERNAL_FUNCS_BASE +  429)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IHP_PROG_PTC_CMD_SET_UNSAFE                                           ((JER2_ARAD_INTERNAL_FUNCS_BASE +  430)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IHP_SHAPING_SET_UNSAFE                                                ((JER2_ARAD_INTERNAL_FUNCS_BASE +  431)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IHP_SHAPING_GET_UNSAFE                                                ((JER2_ARAD_INTERNAL_FUNCS_BASE +  432)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_COUNTER_GET                                                       ((JER2_ARAD_INTERNAL_FUNCS_BASE +  433)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_COUNTER_GET_UNSAFE                                                ((JER2_ARAD_INTERNAL_FUNCS_BASE +  434)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_ALL_COUNTERS_GET                                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE +  435)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_ALL_COUNTERS_GET_UNSAFE                                           ((JER2_ARAD_INTERNAL_FUNCS_BASE +  436)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_LINK_STATUS_GET                                                   ((JER2_ARAD_INTERNAL_FUNCS_BASE +  437)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_LINK_STATUS_GET_UNSAFE                                            ((JER2_ARAD_INTERNAL_FUNCS_BASE +  438)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_INTERFACE_VERIFY                                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE +  439)|JER2_ARAD_PROC_BITS)

#define JER2_ARAD_SRD_LANE_PHYSICAL_PARAMS_FOR_FDR_SET                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE +  440)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_LANE_PHYSICAL_PARAMS_FOR_HDR_SET                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE +  441)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_LANE_PHYSICAL_PARAMS_FOR_QDR_SET                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE +  442)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_LANE_PHYSICAL_PARAMS_FOR_FDR_GET                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE +  443)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_LANE_PHYSICAL_PARAMS_FOR_HDR_GET                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE +  444)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_LANE_PHYSICAL_PARAMS_FOR_QDR_GET                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE +  445)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_LINK_RX_EYE_MONITOR_MODE_SET                                      ((JER2_ARAD_INTERNAL_FUNCS_BASE +  449)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_LINK_RX_EYE_MONITOR_MODE_GET                                      ((JER2_ARAD_INTERNAL_FUNCS_BASE +  450)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_LINK_RX_EYE_MONITOR_RUN                                           ((JER2_ARAD_INTERNAL_FUNCS_BASE +  451)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_LINK_RX_EYE_MONITOR_HEIGHT_GET                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE +  452)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_IPU_MEM_LOAD                                                      ((JER2_ARAD_INTERNAL_FUNCS_BASE +  453)|JER2_ARAD_PROC_BITS)

/*
 *  jer2_arad_mgmt
 */
#define JER2_ARAD_MGMT_OPERATION_MODE_SET                                               ((JER2_ARAD_INTERNAL_FUNCS_BASE +  454)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MGMT_OPERATION_MODE_GET                                               ((JER2_ARAD_INTERNAL_FUNCS_BASE +  455)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_REGISTER_DEVICE_UNSAFE                                                ((JER2_ARAD_INTERNAL_FUNCS_BASE +  456)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MGMT_OPERATION_MODE_SET_UNSAFE                                        ((JER2_ARAD_INTERNAL_FUNCS_BASE +  457)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MGMT_OPERATION_MODE_VERIFY                                            ((JER2_ARAD_INTERNAL_FUNCS_BASE +  458)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MGMT_OPERATION_MODE_GET_UNSAFE                                        ((JER2_ARAD_INTERNAL_FUNCS_BASE +  459)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MGMT_OPERATION_MODE_DEVICE_TYPE_SET                                   ((JER2_ARAD_INTERNAL_FUNCS_BASE +  460)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MGMT_OPERATION_MODE_DEVICE_TYPE_GET                                   ((JER2_ARAD_INTERNAL_FUNCS_BASE +  461)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SW_DB_REVISION_INIT                                                   ((JER2_ARAD_INTERNAL_FUNCS_BASE +  462)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MGMT_MODULE_INIT                                                      ((JER2_ARAD_INTERNAL_FUNCS_BASE +  463)|JER2_ARAD_PROC_BITS)

#define JER2_ARAD_IHP_STAG_HDR_DATA_SET_UNSAFE                                          ((JER2_ARAD_INTERNAL_FUNCS_BASE +  464)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IHP_STAG_HDR_DATA_VERIFY                                              ((JER2_ARAD_INTERNAL_FUNCS_BASE +  465)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IHP_STAG_HDR_DATA_GET_UNSAFE                                          ((JER2_ARAD_INTERNAL_FUNCS_BASE +  466)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IHP_TMLAG_HUSH_FIELD_SET_UNSAFE                                       ((JER2_ARAD_INTERNAL_FUNCS_BASE +  467)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IHP_TMLAG_HUSH_FIELD_VERIFY                                           ((JER2_ARAD_INTERNAL_FUNCS_BASE +  468)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IHP_TMLAG_HUSH_FIELD_GET_UNSAFE                                       ((JER2_ARAD_INTERNAL_FUNCS_BASE +  469)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IHP_STAG_HDR_DATA_SET                                                 ((JER2_ARAD_INTERNAL_FUNCS_BASE +  470)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IHP_STAG_HDR_DATA_GET                                                 ((JER2_ARAD_INTERNAL_FUNCS_BASE +  471)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IHP_TMLAG_HUSH_FIELD_SET                                              ((JER2_ARAD_INTERNAL_FUNCS_BASE +  472)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IHP_TMLAG_HUSH_FIELD_GET                                              ((JER2_ARAD_INTERNAL_FUNCS_BASE +  473)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DEVICE_CLOSE                                                          ((JER2_ARAD_INTERNAL_FUNCS_BASE +  474)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_UNREGISTER_DEVICE_UNSAFE                                              ((JER2_ARAD_INTERNAL_FUNCS_BASE +  475)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SYS_PHYS_TO_LOCAL_PORT_MAP_SET                                        ((JER2_ARAD_INTERNAL_FUNCS_BASE +  476)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SYS_PHYS_TO_LOCAL_PORT_MAP_GET                                        ((JER2_ARAD_INTERNAL_FUNCS_BASE +  477)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_LOCAL_TO_SYS_PHYS_PORT_MAP_GET                                        ((JER2_ARAD_INTERNAL_FUNCS_BASE +  478)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PORT_TO_INTERFACE_MAP_SET                                             ((JER2_ARAD_INTERNAL_FUNCS_BASE +  479)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PORT_TO_DYNAMIC_INTERFACE_MAP_SET                                     ((JER2_ARAD_INTERNAL_FUNCS_BASE +  480)|JER2_ARAD_PROC_BITS)  
#define JER2_ARAD_PORT_TO_INTERFACE_MAP_GET                                             ((JER2_ARAD_INTERNAL_FUNCS_BASE +  481)|JER2_ARAD_PROC_BITS) 
#define JER2_ARAD_PORTS_LAG_SET                                                         ((JER2_ARAD_INTERNAL_FUNCS_BASE +  482)|JER2_ARAD_PROC_BITS) 
#define JER2_ARAD_PORTS_LAG_GET                                                         ((JER2_ARAD_INTERNAL_FUNCS_BASE +  483)|JER2_ARAD_PROC_BITS) 
#define JER2_ARAD_PORT_HEADER_TYPE_SET                                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE +  484)|JER2_ARAD_PROC_BITS) 
#define JER2_ARAD_PORT_HEADER_TYPE_GET                                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE +  485)|JER2_ARAD_PROC_BITS) 
#define JER2_ARAD_PORTS_MIRROR_INBOUND_SET                                              ((JER2_ARAD_INTERNAL_FUNCS_BASE +  486)|JER2_ARAD_PROC_BITS) 
#define JER2_ARAD_PORTS_MIRROR_INBOUND_GET                                              ((JER2_ARAD_INTERNAL_FUNCS_BASE +  487)|JER2_ARAD_PROC_BITS) 
#define JER2_ARAD_PORTS_MIRROR_OUTBOUND_SET                                             ((JER2_ARAD_INTERNAL_FUNCS_BASE +  488)|JER2_ARAD_PROC_BITS) 
#define JER2_ARAD_PORTS_MIRROR_OUTBOUND_GET                                             ((JER2_ARAD_INTERNAL_FUNCS_BASE +  489)|JER2_ARAD_PROC_BITS) 
#define JER2_ARAD_PORTS_SNOOP_SET                                                       ((JER2_ARAD_INTERNAL_FUNCS_BASE +  490)|JER2_ARAD_PROC_BITS) 
#define JER2_ARAD_PORTS_SNOOP_GET                                                       ((JER2_ARAD_INTERNAL_FUNCS_BASE +  491)|JER2_ARAD_PROC_BITS) 
#define JER2_ARAD_SYS_PHYS_TO_LOCAL_PORT_MAP_SET_UNSAFE                                 ((JER2_ARAD_INTERNAL_FUNCS_BASE +  492)|JER2_ARAD_PROC_BITS) 
#define JER2_ARAD_SYS_PHYS_TO_LOCAL_PORT_MAP_VERIFY                                     ((JER2_ARAD_INTERNAL_FUNCS_BASE +  493)|JER2_ARAD_PROC_BITS) 
#define JER2_ARAD_SYS_PHYS_TO_LOCAL_PORT_MAP_GET_UNSAFE                                 ((JER2_ARAD_INTERNAL_FUNCS_BASE +  494)|JER2_ARAD_PROC_BITS) 
#define JER2_ARAD_LOCAL_TO_SYS_PHYS_PORT_MAP_GET_UNSAFE                                 ((JER2_ARAD_INTERNAL_FUNCS_BASE +  495)|JER2_ARAD_PROC_BITS) 
#define JER2_ARAD_PORT_TO_DYNAMIC_INTERFACE_MAP_SET_UNSAFE                              ((JER2_ARAD_INTERNAL_FUNCS_BASE +  496)|JER2_ARAD_PROC_BITS) 
#define JER2_ARAD_PORT_TO_INTERFACE_MAP_SET_UNSAFE                                      ((JER2_ARAD_INTERNAL_FUNCS_BASE +  497)|JER2_ARAD_PROC_BITS) 
#define JER2_ARAD_PORT_TO_INTERFACE_MAP_VERIFY                                          ((JER2_ARAD_INTERNAL_FUNCS_BASE +  498)|JER2_ARAD_PROC_BITS) 
#define JER2_ARAD_PORT_TO_INTERFACE_MAP_GET_UNSAFE                                      ((JER2_ARAD_INTERNAL_FUNCS_BASE +  499)|JER2_ARAD_PROC_BITS) 
#define JER2_ARAD_PORTS_LAG_SET_UNSAFE                                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE +  500)|JER2_ARAD_PROC_BITS) 
#define JER2_ARAD_PORTS_LAG_VERIFY                                                      ((JER2_ARAD_INTERNAL_FUNCS_BASE +  501)|JER2_ARAD_PROC_BITS) 
#define JER2_ARAD_PORTS_LAG_GET_UNSAFE                                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE +  502)|JER2_ARAD_PROC_BITS) 
#define JER2_ARAD_PORT_HEADER_TYPE_SET_UNSAFE                                           ((JER2_ARAD_INTERNAL_FUNCS_BASE +  503)|JER2_ARAD_PROC_BITS) 
#define JER2_ARAD_PORT_HEADER_TYPE_VERIFY                                               ((JER2_ARAD_INTERNAL_FUNCS_BASE +  504)|JER2_ARAD_PROC_BITS) 
#define JER2_ARAD_PORT_HEADER_TYPE_GET_UNSAFE                                           ((JER2_ARAD_INTERNAL_FUNCS_BASE +  505)|JER2_ARAD_PROC_BITS) 
#define JER2_ARAD_PORTS_MIRROR_INBOUND_SET_UNSAFE                                       ((JER2_ARAD_INTERNAL_FUNCS_BASE +  506)|JER2_ARAD_PROC_BITS) 
#define JER2_ARAD_PORTS_MIRROR_INBOUND_VERIFY                                           ((JER2_ARAD_INTERNAL_FUNCS_BASE +  507)|JER2_ARAD_PROC_BITS) 
#define JER2_ARAD_PORTS_MIRROR_INBOUND_GET_UNSAFE                                       ((JER2_ARAD_INTERNAL_FUNCS_BASE +  508)|JER2_ARAD_PROC_BITS) 
#define JER2_ARAD_PORTS_MIRROR_OUTBOUND_SET_UNSAFE                                      ((JER2_ARAD_INTERNAL_FUNCS_BASE +  509)|JER2_ARAD_PROC_BITS) 
#define JER2_ARAD_PORTS_MIRROR_OUTBOUND_VERIFY                                          ((JER2_ARAD_INTERNAL_FUNCS_BASE +  510)|JER2_ARAD_PROC_BITS) 
#define JER2_ARAD_PORTS_MIRROR_OUTBOUND_GET_UNSAFE                                      ((JER2_ARAD_INTERNAL_FUNCS_BASE +  511)|JER2_ARAD_PROC_BITS) 
#define JER2_ARAD_PORTS_SNOOP_SET_UNSAFE                                                ((JER2_ARAD_INTERNAL_FUNCS_BASE +  512)|JER2_ARAD_PROC_BITS) 
#define JER2_ARAD_PORTS_SNOOP_VERIFY                                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE +  513)|JER2_ARAD_PROC_BITS) 
#define JER2_ARAD_PORTS_SNOOP_GET_UNSAFE                                                ((JER2_ARAD_INTERNAL_FUNCS_BASE +  514)|JER2_ARAD_PROC_BITS) 
#define JER2_ARAD_EGQ_PPCT_TBL_GET_UNSAFE                                               ((JER2_ARAD_INTERNAL_FUNCS_BASE +  515)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_PPCT_TBL_SET_UNSAFE                                               ((JER2_ARAD_INTERNAL_FUNCS_BASE +  516)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PORTS_LOGICAL_SYS_ID_BUILD                                            ((JER2_ARAD_INTERNAL_FUNCS_BASE +  520)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PORTS_LOGICAL_SYS_ID_PARSE                                            ((JER2_ARAD_INTERNAL_FUNCS_BASE +  521)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PORTS_ITMH_EXTENSION_SET                                              ((JER2_ARAD_INTERNAL_FUNCS_BASE +  522)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PORTS_ITMH_EXTENSION_GET                                              ((JER2_ARAD_INTERNAL_FUNCS_BASE +  523)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PORTS_SHAPING_HEADER_SET                                              ((JER2_ARAD_INTERNAL_FUNCS_BASE +  524)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PORTS_SHAPING_HEADER_GET                                              ((JER2_ARAD_INTERNAL_FUNCS_BASE +  525)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PORTS_FORWARDING_HEADER_SET                                           ((JER2_ARAD_INTERNAL_FUNCS_BASE +  526)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PORTS_FORWARDING_HEADER_GET                                           ((JER2_ARAD_INTERNAL_FUNCS_BASE +  527)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PORTS_STAG_FIELDS_SET                                                 ((JER2_ARAD_INTERNAL_FUNCS_BASE +  528)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PORTS_STAG_FIELDS_GET                                                 ((JER2_ARAD_INTERNAL_FUNCS_BASE +  529)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PORTS_FTMH_EXTENSION_SET                                              ((JER2_ARAD_INTERNAL_FUNCS_BASE +  530)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PORTS_FTMH_EXTENSION_GET                                              ((JER2_ARAD_INTERNAL_FUNCS_BASE +  531)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PORTS_OTMH_EXTENSION_SET                                              ((JER2_ARAD_INTERNAL_FUNCS_BASE +  532)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PORTS_OTMH_EXTENSION_GET                                              ((JER2_ARAD_INTERNAL_FUNCS_BASE +  533)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PORTS_STAG_FIELDS_SET_UNSAFE                                          ((JER2_ARAD_INTERNAL_FUNCS_BASE +  534)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PORTS_STAG_FIELDS_VERIFY                                              ((JER2_ARAD_INTERNAL_FUNCS_BASE +  535)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PORTS_STAG_FIELDS_GET_UNSAFE                                          ((JER2_ARAD_INTERNAL_FUNCS_BASE +  536)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PORTS_FTMH_EXTENSION_SET_UNSAFE                                       ((JER2_ARAD_INTERNAL_FUNCS_BASE +  537)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PORTS_FTMH_EXTENSION_VERIFY                                           ((JER2_ARAD_INTERNAL_FUNCS_BASE +  538)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PORTS_FTMH_EXTENSION_GET_UNSAFE                                       ((JER2_ARAD_INTERNAL_FUNCS_BASE +  539)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PORTS_OTMH_EXTENSION_SET_UNSAFE                                       ((JER2_ARAD_INTERNAL_FUNCS_BASE +  540)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PORTS_OTMH_EXTENSION_VERIFY                                           ((JER2_ARAD_INTERNAL_FUNCS_BASE +  541)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PORTS_OTMH_EXTENSION_GET_UNSAFE                                       ((JER2_ARAD_INTERNAL_FUNCS_BASE +  542)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PORTS_FORWARDING_HEADER_VERIFY                                        ((JER2_ARAD_INTERNAL_FUNCS_BASE +  543)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PORTS_FORWARDING_HEADER_GET_UNSAFE                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE +  544)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PORTS_ITMH_EXTENSION_SET_UNSAFE                                       ((JER2_ARAD_INTERNAL_FUNCS_BASE +  545)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PORTS_ITMH_EXTENSION_VERIFY                                           ((JER2_ARAD_INTERNAL_FUNCS_BASE +  546)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PORTS_ITMH_EXTENSION_GET_UNSAFE                                       ((JER2_ARAD_INTERNAL_FUNCS_BASE +  547)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PORTS_SHAPING_HEADER_SET_UNSAFE                                       ((JER2_ARAD_INTERNAL_FUNCS_BASE +  548)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PORTS_SHAPING_HEADER_VERIFY                                           ((JER2_ARAD_INTERNAL_FUNCS_BASE +  549)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PORTS_SHAPING_HEADER_GET_UNSAFE                                       ((JER2_ARAD_INTERNAL_FUNCS_BASE +  550)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PORTS_FORWARDING_HEADER_SET_UNSAFE                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE +  551)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_HPU_ITMH_BUILD_VERIFY                                                 ((JER2_ARAD_INTERNAL_FUNCS_BASE +  560)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_HPU_ITMH_BUILD                                                        ((JER2_ARAD_INTERNAL_FUNCS_BASE +  561)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_HPU_ITMH_PARSE                                                        ((JER2_ARAD_INTERNAL_FUNCS_BASE +  562)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_HPU_FTMH_BUILD_VERIFY                                                 ((JER2_ARAD_INTERNAL_FUNCS_BASE +  563)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_HPU_FTMH_BUILD                                                        ((JER2_ARAD_INTERNAL_FUNCS_BASE +  564)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_HPU_FTMH_PARSE                                                        ((JER2_ARAD_INTERNAL_FUNCS_BASE +  565)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_HPU_OTMH_BUILD_VERIFY                                                 ((JER2_ARAD_INTERNAL_FUNCS_BASE +  566)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_HPU_OTMH_BUILD                                                        ((JER2_ARAD_INTERNAL_FUNCS_BASE +  567)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_HPU_OTMH_PARSE                                                        ((JER2_ARAD_INTERNAL_FUNCS_BASE +  568)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_DRAM_BUFFS_SET                                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE +  569)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_DRAM_BUFFS_GET                                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE +  570)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MGMT_HW_WAIT_FOR_INIT                                                 ((JER2_ARAD_INTERNAL_FUNCS_BASE +  571)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_DRAM_BUFFS_SET_UNSAFE                                             ((JER2_ARAD_INTERNAL_FUNCS_BASE +  573)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_DRAM_BUFFS_VERIFY                                                 ((JER2_ARAD_INTERNAL_FUNCS_BASE +  574)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_DRAM_BUFFS_GET_UNSAFE                                             ((JER2_ARAD_INTERNAL_FUNCS_BASE +  575)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MGMT_BLOCKS_INIT_UNSAFE                                               ((JER2_ARAD_INTERNAL_FUNCS_BASE +  576)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGR_QUEUING_REGS_INIT                                                 ((JER2_ARAD_INTERNAL_FUNCS_BASE +  577)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGR_QUEUING_INIT                                                      ((JER2_ARAD_INTERNAL_FUNCS_BASE +  578)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FABRIC_REGS_INIT                                                      ((JER2_ARAD_INTERNAL_FUNCS_BASE +  579)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FABRIC_INIT                                                           ((JER2_ARAD_INTERNAL_FUNCS_BASE +  580)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FLOW_CONTROL_REGS_INIT                                                ((JER2_ARAD_INTERNAL_FUNCS_BASE +  581)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FLOW_CONTROL_INIT                                                     ((JER2_ARAD_INTERNAL_FUNCS_BASE +  582)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_INGRESS_HEADER_PARSING_REGS_INIT                                      ((JER2_ARAD_INTERNAL_FUNCS_BASE +  583)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_INGRESS_HEADER_PARSING_INIT                                           ((JER2_ARAD_INTERNAL_FUNCS_BASE +  584)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_INGRESS_SCHEDULER_REGS_INIT                                           ((JER2_ARAD_INTERNAL_FUNCS_BASE +  585)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_INGRESS_SCHEDULER_INIT                                                ((JER2_ARAD_INTERNAL_FUNCS_BASE +  586)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_REGS_INIT                                                         ((JER2_ARAD_INTERNAL_FUNCS_BASE +  587)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_INIT                                                              ((JER2_ARAD_INTERNAL_FUNCS_BASE +  588)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PORTS_REGS_INIT                                                       ((JER2_ARAD_INTERNAL_FUNCS_BASE +  589)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PORTS_INIT                                                            ((JER2_ARAD_INTERNAL_FUNCS_BASE +  590)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCHEDULER_END2END_REGS_INIT                                           ((JER2_ARAD_INTERNAL_FUNCS_BASE +  591)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCHEDULER_END2END_INIT                                                ((JER2_ARAD_INTERNAL_FUNCS_BASE +  592)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SERDES_REGS_INIT                                                      ((JER2_ARAD_INTERNAL_FUNCS_BASE +  593)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SERDES_INIT                                                           ((JER2_ARAD_INTERNAL_FUNCS_BASE +  594)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MGMT_TBLS_INIT                                                        ((JER2_ARAD_INTERNAL_FUNCS_BASE +  595)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MGMT_FUNCTIONAL_INIT                                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE +  596)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MGMT_HW_SET_DEFAULTS                                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE +  597)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_OLP_SET_REPS_FOR_TBL                                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE +  598)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IRE_SET_REPS_FOR_TBL                                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE +  599)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IDR_SET_REPS_FOR_TBL                                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE +  600)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IRR_SET_REPS_FOR_TBL                                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE +  601)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IHP_SET_REPS_FOR_TBL                                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE +  602)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IQM_SET_REPS_FOR_TBL                                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE +  603)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPS_SET_REPS_FOR_TBL                                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE +  604)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPT_SET_REPS_FOR_TBL                                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE +  605)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DPI_SET_REPS_FOR_TBL                                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE +  606)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_RTP_SET_REPS_FOR_TBL                                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE +  611)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_SET_REPS_FOR_TBL                                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE +  612)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_CFC_SET_REPS_FOR_TBL                                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE +  613)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_SET_REPS_FOR_TBL                                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE +  614)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_GEN_SET_REPS_FOR_TBL_UNSAFE                                           ((JER2_ARAD_INTERNAL_FUNCS_BASE +  615)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_OLP_SET_REPS_FOR_TBL_UNSAFE                                           ((JER2_ARAD_INTERNAL_FUNCS_BASE +  616)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IRE_SET_REPS_FOR_TBL_UNSAFE                                           ((JER2_ARAD_INTERNAL_FUNCS_BASE +  617)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IDR_SET_REPS_FOR_TBL_UNSAFE                                           ((JER2_ARAD_INTERNAL_FUNCS_BASE +  618)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IRR_SET_REPS_FOR_TBL_UNSAFE                                           ((JER2_ARAD_INTERNAL_FUNCS_BASE +  619)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IHP_SET_REPS_FOR_TBL_UNSAFE                                           ((JER2_ARAD_INTERNAL_FUNCS_BASE +  620)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IQM_SET_REPS_FOR_TBL_UNSAFE                                           ((JER2_ARAD_INTERNAL_FUNCS_BASE +  621)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPS_SET_REPS_FOR_TBL_UNSAFE                                           ((JER2_ARAD_INTERNAL_FUNCS_BASE +  622)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPT_SET_REPS_FOR_TBL_UNSAFE                                           ((JER2_ARAD_INTERNAL_FUNCS_BASE +  623)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DPI_SET_REPS_FOR_TBL_UNSAFE                                           ((JER2_ARAD_INTERNAL_FUNCS_BASE +  624)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_RTP_SET_REPS_FOR_TBL_UNSAFE                                           ((JER2_ARAD_INTERNAL_FUNCS_BASE +  629)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_SET_REPS_FOR_TBL_UNSAFE                                           ((JER2_ARAD_INTERNAL_FUNCS_BASE +  630)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_CFC_SET_REPS_FOR_TBL_UNSAFE                                           ((JER2_ARAD_INTERNAL_FUNCS_BASE +  631)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_SET_REPS_FOR_TBL_UNSAFE                                           ((JER2_ARAD_INTERNAL_FUNCS_BASE +  632)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EPNI_SET_REPS_FOR_TBL                                                 ((JER2_ARAD_INTERNAL_FUNCS_BASE +  633)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EPNI_SET_REPS_FOR_TBL_UNSAFE                                          ((JER2_ARAD_INTERNAL_FUNCS_BASE +  634)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MGMT_OLP_TBLS_INIT                                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE +  635)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MGMT_IRE_TBLS_INIT                                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE +  636)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MGMT_IDR_TBLS_INIT                                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE +  637)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MGMT_IRR_TBLS_INIT                                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE +  638)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MGMT_IHP_TBLS_INIT                                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE +  639)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MGMT_IQM_TBLS_INIT                                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE +  640)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MGMT_IPS_TBLS_INIT                                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE +  641)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MGMT_DPI_TBLS_INIT                                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE +  642)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MGMT_RTP_TBLS_INIT                                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE +  643)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MGMT_EGQ_TBLS_INIT                                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE +  644)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MGMT_CFC_TBLS_INIT                                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE +  645)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MGMT_SCH_TBLS_INIT                                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE +  646)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MGMT_IPT_TBLS_INIT                                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE +  647)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PORTS_LAG_SYS_PORT_ADD_VERIFY                                         ((JER2_ARAD_INTERNAL_FUNCS_BASE +  648)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PORTS_LAG_SYS_PORT_ADD_UNSAFE                                         ((JER2_ARAD_INTERNAL_FUNCS_BASE +  649)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PORTS_LAG_SYS_PORT_ADD                                                ((JER2_ARAD_INTERNAL_FUNCS_BASE +  650)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PORTS_LAG_SYS_PORT_REMOVE_UNSAFE                                      ((JER2_ARAD_INTERNAL_FUNCS_BASE +  651)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PORTS_LAG_SYS_PORT_REMOVE                                             ((JER2_ARAD_INTERNAL_FUNCS_BASE +  652)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PORTS_LAG_SYS_PORT_INFO_GET                                           ((JER2_ARAD_INTERNAL_FUNCS_BASE +  655)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PORTS_LAG_SYS_PORT_INFO_GET_UNSAFE                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE +  656)|JER2_ARAD_PROC_BITS)

#define JER2_ARAD_DRAM_INIT_DDR                                                         ((JER2_ARAD_INTERNAL_FUNCS_BASE +  666)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MGMT_HW_ADJUST_QDR                                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE +  667)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MGMT_HW_ADJUST_SERDES                                                 ((JER2_ARAD_INTERNAL_FUNCS_BASE +  668)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MGMT_HW_ADJUST_CPU                                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE +  669)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MGMT_HW_ADJUST_FABRIC                                                 ((JER2_ARAD_INTERNAL_FUNCS_BASE +  670)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MGMT_ALL_CTRL_CELLS_FCT_DISABLE_POLLING                               ((JER2_ARAD_INTERNAL_FUNCS_BASE +  671)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MGMT_ALL_CTRL_CELLS_ENABLE_WRITE                                      ((JER2_ARAD_INTERNAL_FUNCS_BASE +  672)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MGMT_ALL_CTRL_CELLS_FCT_ENABLE_POLLING                                ((JER2_ARAD_INTERNAL_FUNCS_BASE +  673)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MGMT_INIT_SEQUENCE_PHASE1_VERIFY                                      ((JER2_ARAD_INTERNAL_FUNCS_BASE +  674)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MGMT_INIT_SEQUENCE_PHASE2_VERIFY                                      ((JER2_ARAD_INTERNAL_FUNCS_BASE +  675)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PORTS_FAP_AND_NIF_TYPE_MATCH_VERIFY                                   ((JER2_ARAD_INTERNAL_FUNCS_BASE +  677)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_MAL_BASIC_CONF_SET_ALL                                            ((JER2_ARAD_INTERNAL_FUNCS_BASE +  678)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_MAL_BASIC_CONF_GET_ALL                                            ((JER2_ARAD_INTERNAL_FUNCS_BASE +  679)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_LANE_SET_ALL                                                      ((JER2_ARAD_INTERNAL_FUNCS_BASE +  680)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_LANE_GET_ALL                                                      ((JER2_ARAD_INTERNAL_FUNCS_BASE +  681)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_MAL_BASIC_CONF_VERIFY_ALL                                         ((JER2_ARAD_INTERNAL_FUNCS_BASE +  682)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_MAL_BASIC_CONF_SET_ALL_UNSAFE                                     ((JER2_ARAD_INTERNAL_FUNCS_BASE +  683)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_MAL_BASIC_CONF_GET_ALL_UNSAFE                                     ((JER2_ARAD_INTERNAL_FUNCS_BASE +  684)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_LANE_VERIFY_ALL                                                   ((JER2_ARAD_INTERNAL_FUNCS_BASE +  685)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_LANE_SET_ALL_UNSAFE                                               ((JER2_ARAD_INTERNAL_FUNCS_BASE +  686)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_LANE_GET_ALL_UNSAFE                                               ((JER2_ARAD_INTERNAL_FUNCS_BASE +  687)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_OFP_RATES_SET                                                         ((JER2_ARAD_INTERNAL_FUNCS_BASE +  688)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_OFP_RATES_GET                                                         ((JER2_ARAD_INTERNAL_FUNCS_BASE +  689)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_OFP_RATES_UPDATE                                                      ((JER2_ARAD_INTERNAL_FUNCS_BASE +  690)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_OFP_RATES_SINGLE_PORT_GET                                             ((JER2_ARAD_INTERNAL_FUNCS_BASE +  691)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_OFP_RATES_INTERFACE_SHAPER_SET                                        ((JER2_ARAD_INTERNAL_FUNCS_BASE +  692)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_OFP_RATES_INTERFACE_SHAPER_GET                                        ((JER2_ARAD_INTERNAL_FUNCS_BASE +  693)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_OFP_RATES_SET_UNSAFE                                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE +  694)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_OFP_RATES_VERIFY                                                      ((JER2_ARAD_INTERNAL_FUNCS_BASE +  695)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_OFP_RATES_GET_UNSAFE                                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE +  696)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_OFP_RATES_UPDATE_UNSAFE                                               ((JER2_ARAD_INTERNAL_FUNCS_BASE +  697)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_OFP_RATES_SINGLE_PORT_GET_UNSAFE                                      ((JER2_ARAD_INTERNAL_FUNCS_BASE +  698)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_OFP_RATES_INTERFACE_SHAPER_SET_UNSAFE                                 ((JER2_ARAD_INTERNAL_FUNCS_BASE +  699)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_OFP_RATES_INTERFACE_SHAPER_VERIFY                                     ((JER2_ARAD_INTERNAL_FUNCS_BASE +  700)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_OFP_RATES_INTERFACE_SHAPER_GET_UNSAFE                                 ((JER2_ARAD_INTERNAL_FUNCS_BASE +  701)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_OFP_RATES_UPDATE_VERIFY                                               ((JER2_ARAD_INTERNAL_FUNCS_BASE +  703)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_OFP_RATES_CAL_PER_LEN_BUILD                                           ((JER2_ARAD_INTERNAL_FUNCS_BASE +  704)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_OFP_RATES_CAL_IMPROVE_NOF_SLOTS                                       ((JER2_ARAD_INTERNAL_FUNCS_BASE +  705)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_OFP_RATES_CAL_LEN_CALCULATE                                           ((JER2_ARAD_INTERNAL_FUNCS_BASE +  707)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_OFP_RATES_FIXED_LEN_CAL_BUILD                                         ((JER2_ARAD_INTERNAL_FUNCS_BASE +  708)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_OFP_RATES_DEVICE_INTERFACE_RATE_SET                                   ((JER2_ARAD_INTERNAL_FUNCS_BASE +  710)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_IS_CHANNELIZED_UNSAFE                                             ((JER2_ARAD_INTERNAL_FUNCS_BASE +  711)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_IS_CHANNELIZED                                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE +  712)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_OFP_RATES_FROM_SCH_PORT_RATES_TO_CALENDAR                             ((JER2_ARAD_INTERNAL_FUNCS_BASE +  713)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_OFP_RATES_FILL_SHAPER_CALENDAR_CREDITS                                ((JER2_ARAD_INTERNAL_FUNCS_BASE +  714)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_OFP_RATES_FROM_EGQ_PORT_RATES_TO_CALENDAR                             ((JER2_ARAD_INTERNAL_FUNCS_BASE +  715)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_OFP_RATES_FROM_CALENDAR_TO_PORTS_SCH_RATE                             ((JER2_ARAD_INTERNAL_FUNCS_BASE +  716)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_OFP_RATES_FROM_CALENDAR_TO_PORTS_EGQ_RATE                             ((JER2_ARAD_INTERNAL_FUNCS_BASE +  718)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_SCM_TBL_GET_UNSAFE                                                ((JER2_ARAD_INTERNAL_FUNCS_BASE +  720)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGQ_SCM_TBL_SET_UNSAFE                                                ((JER2_ARAD_INTERNAL_FUNCS_BASE +  722)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_OFP_RATES_ACTIVE_CALENDARS_CONFIG                                     ((JER2_ARAD_INTERNAL_FUNCS_BASE +  723)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_LANE_TX_RATE_BELOW_3_2_PARAMS_TO_DEVICE_CALC                      ((JER2_ARAD_INTERNAL_FUNCS_BASE +  724)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_LANE_TX_RATE_ABOVE_3_2_PARAMS_TO_DEVICE_CALC                      ((JER2_ARAD_INTERNAL_FUNCS_BASE +  725)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_LANE_TX_RATE_BELOW_3_2_PARAMS_FROM_DEVICE_CALC                    ((JER2_ARAD_INTERNAL_FUNCS_BASE +  726)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_LANE_TX_RATE_ABOVE_3_2_PARAMS_FROM_DEVICE_CALC                    ((JER2_ARAD_INTERNAL_FUNCS_BASE +  727)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DRAM_INIT                                                             ((JER2_ARAD_INTERNAL_FUNCS_BASE +  728)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DRAM_WAIT_FOR_INIT                                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE +  729)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_PORT_HP_CLASS_CONF_GET                                            ((JER2_ARAD_INTERNAL_FUNCS_BASE +  730)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_FLOW_TO_QUEUE_MAPPING_UNBIND                                      ((JER2_ARAD_INTERNAL_FUNCS_BASE +  731)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_FLOW_TO_QUEUE_MAPPING_UNBIND_VERIFY                               ((JER2_ARAD_INTERNAL_FUNCS_BASE +  732)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_FLOW_IS_RESET_GET_UNSAFE                                          ((JER2_ARAD_INTERNAL_FUNCS_BASE +  733)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FLOW_STATUS_INFO_GET                                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE +  734)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_AGG_STATUS_INFO_GET                                                   ((JER2_ARAD_INTERNAL_FUNCS_BASE +  735)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FLOW_AND_UP_PRINT_UNSAFE                                              ((JER2_ARAD_INTERNAL_FUNCS_BASE +  736)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_IS_HR_SUBFLOW_VALID                                               ((JER2_ARAD_INTERNAL_FUNCS_BASE +  737)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_IS_CL_SUBFLOW_VALID                                               ((JER2_ARAD_INTERNAL_FUNCS_BASE +  738)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_HR_TO_PORT_ASSIGN_SET                                             ((JER2_ARAD_INTERNAL_FUNCS_BASE +  739)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_HR_TO_PORT_ASSIGN_GET                                             ((JER2_ARAD_INTERNAL_FUNCS_BASE +  740)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_OFP_EGQ_SHAPER_RATE_TO_INTERNAL                                       ((JER2_ARAD_INTERNAL_FUNCS_BASE +  741)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_OFP_RATES_SW_DB_WRITE                                                 ((JER2_ARAD_INTERNAL_FUNCS_BASE +  742)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_OFP_RATES_EGQ_SHAPER_CONFIG                                           ((JER2_ARAD_INTERNAL_FUNCS_BASE +  743)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_OFP_EGQ_SHAPER_RATE_FROM_INTERNAL                                     ((JER2_ARAD_INTERNAL_FUNCS_BASE +  744)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_OFP_RATES_EXACT_TBL_INIT                                              ((JER2_ARAD_INTERNAL_FUNCS_BASE +  745)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_OFP_RATES_TABLE_CONSTRUCT                                             ((JER2_ARAD_INTERNAL_FUNCS_BASE +  746)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_OFP_RATES_ACTIVE_CALENDARS_RETRIEVE                                   ((JER2_ARAD_INTERNAL_FUNCS_BASE +  747)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_OFP_RATES_EGQ_SHAPER_RETRIEVE                                         ((JER2_ARAD_INTERNAL_FUNCS_BASE +  748)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_AQFM_MAL_DFLT_SHAPER_GET                                              ((JER2_ARAD_INTERNAL_FUNCS_BASE +  749)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PORT_OFP_MAL_GET_UNSAFE                                               ((JER2_ARAD_INTERNAL_FUNCS_BASE +  750)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_OFP_RATES_ACTIVE_MAL_BUILD                                            ((JER2_ARAD_INTERNAL_FUNCS_BASE +  751)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SW_DB_EGR_PORTS_ACTIVE_MALS_GET                                       ((JER2_ARAD_INTERNAL_FUNCS_BASE +  752)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_FLD_FROM_REG_GET                                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE +  753)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_FLD_FROM_REG_SET                                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE +  754)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_EJER2_ARAD_STATUS_PARSE                                                ((JER2_ARAD_INTERNAL_FUNCS_BASE +  755)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_EJER2_ARAD_CMD_BUILD                                                   ((JER2_ARAD_INTERNAL_FUNCS_BASE +  756)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_ACCESS_VERIFY                                                     ((JER2_ARAD_INTERNAL_FUNCS_BASE +  757)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_REG_READ_UNSAFE                                                   ((JER2_ARAD_INTERNAL_FUNCS_BASE +  758)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_FLD_WRITE_UNSAFE                                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE +  759)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_FLD_READ_UNSAFE                                                   ((JER2_ARAD_INTERNAL_FUNCS_BASE +  760)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_REG_WRITE                                                         ((JER2_ARAD_INTERNAL_FUNCS_BASE +  761)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_REG_READ                                                          ((JER2_ARAD_INTERNAL_FUNCS_BASE +  762)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_FLD_WRITE                                                         ((JER2_ARAD_INTERNAL_FUNCS_BASE +  763)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_FLD_READ                                                          ((JER2_ARAD_INTERNAL_FUNCS_BASE +  764)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_LANE_PHYS_PARAMS_CONFIG                                           ((JER2_ARAD_INTERNAL_FUNCS_BASE +  765)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_REGS_INIT_REGS                                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE +  767)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_OFP_RATES_CONFIG                                                      ((JER2_ARAD_INTERNAL_FUNCS_BASE +  768)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_IPU_INIT_ALL_UNSAFE                                               ((JER2_ARAD_INTERNAL_FUNCS_BASE +  769)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MGMT_INIT_BEFORE_BLOCKS_OOR                                           ((JER2_ARAD_INTERNAL_FUNCS_BASE +  770)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MGMT_INIT_AFTER_BLOCKS_OOR                                            ((JER2_ARAD_INTERNAL_FUNCS_BASE +  771)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_INIT_DRAM_FBC_BUFFS_GET                                               ((JER2_ARAD_INTERNAL_FUNCS_BASE +  772)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_INIT_DRAM_MAX_WITHOUT_FBC_GET                                         ((JER2_ARAD_INTERNAL_FUNCS_BASE +  773)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_REGS_INIT                                                         ((JER2_ARAD_INTERNAL_FUNCS_BASE +  774)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_INIT                                                              ((JER2_ARAD_INTERNAL_FUNCS_BASE +  775)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_IPU_ENABLE_UNSAFE                                                 ((JER2_ARAD_INTERNAL_FUNCS_BASE +  776)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_IPU_ENABLE_ALL_UNSAFE                                             ((JER2_ARAD_INTERNAL_FUNCS_BASE +  777)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DIAG_IDDR_SET                                                         ((JER2_ARAD_INTERNAL_FUNCS_BASE +  779)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DIAG_IDDR_GET                                                         ((JER2_ARAD_INTERNAL_FUNCS_BASE +  780)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DIAG_REGS_DUMP                                                        ((JER2_ARAD_INTERNAL_FUNCS_BASE +  781)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DIAG_TBLS_DUMP                                                        ((JER2_ARAD_INTERNAL_FUNCS_BASE +  782)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DIAG_PACKET_WALKTROUGH_GET                                            ((JER2_ARAD_INTERNAL_FUNCS_BASE +  783)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DIAG_DEV_TBLS_DUMP                                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE +  784)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPQ_ATTACHED_FLOW_PORT_GET                                            ((JER2_ARAD_INTERNAL_FUNCS_BASE +  786)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DIAG_TBLS_DUMP_TABLES_GET                                             ((JER2_ARAD_INTERNAL_FUNCS_BASE +  787)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DIAG_IDDR_SET_UNSAFE                                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE +  788)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DIAG_IDDR_GET_UNSAFE                                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE +  789)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_READ_REG_BUFFER_UNSAFE                                                ((JER2_ARAD_INTERNAL_FUNCS_BASE +  790)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPQ_ATTACHED_FLOW_PORT_GET_UNSAFE                                     ((JER2_ARAD_INTERNAL_FUNCS_BASE +  791)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DIAG_QDR_BIST_TEST_START                                              ((JER2_ARAD_INTERNAL_FUNCS_BASE +  792)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DIAG_QDR_BIST_TEST_START_UNSAFE                                       ((JER2_ARAD_INTERNAL_FUNCS_BASE +  793)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DIAG_QDR_BIST_TEST_RESULT_GET                                         ((JER2_ARAD_INTERNAL_FUNCS_BASE +  794)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DIAG_QDR_BIST_TEST_RESULT_GET_UNSAFE                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE +  795)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DIAG_DRAM_BIST_TEST_START                                             ((JER2_ARAD_INTERNAL_FUNCS_BASE +  796)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DIAG_DRAM_BIST_TEST_START_UNSAFE                                      ((JER2_ARAD_INTERNAL_FUNCS_BASE +  797)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DIAG_DRAM_BIST_TEST_RESULT_GET                                        ((JER2_ARAD_INTERNAL_FUNCS_BASE +  798)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DIAG_DRAM_BIST_TEST_RESULT_GET_UNSAFE                                 ((JER2_ARAD_INTERNAL_FUNCS_BASE +  799)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DIAG_QDR_BIST_TEST_START_VERIFY                                       ((JER2_ARAD_INTERNAL_FUNCS_BASE +  800)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DIAG_QDR_BIST_TEST_COUNTERS_GET_UNSAFE                                ((JER2_ARAD_INTERNAL_FUNCS_BASE +  801)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_QRTT_STATUS_GET                                                   ((JER2_ARAD_INTERNAL_FUNCS_BASE +  802)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_QRTT_STATUS_GET_UNSAFE                                            ((JER2_ARAD_INTERNAL_FUNCS_BASE +  803)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DIAG_DRAM_BIST_TEST_COUNTERS_GET_UNSAFE                               ((JER2_ARAD_INTERNAL_FUNCS_BASE +  804)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_DBUFF_SIZE2INTERNAL                                               ((JER2_ARAD_INTERNAL_FUNCS_BASE +  805)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_DBUFF_INTERNAL2SIZE                                               ((JER2_ARAD_INTERNAL_FUNCS_BASE +  806)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_FAT_PIPE_ENABLE_GET                                               ((JER2_ARAD_INTERNAL_FUNCS_BASE +  807)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SW_DB_FAT_PIPE_INITIALIZE                                             ((JER2_ARAD_INTERNAL_FUNCS_BASE +  808)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MGMT_OP_MODE_RELATED_INIT                                             ((JER2_ARAD_INTERNAL_FUNCS_BASE +  809)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PORT_OFP_MAL_GET                                                      ((JER2_ARAD_INTERNAL_FUNCS_BASE +  810)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_IS_FAT_PIPE_PORT                                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE +  811)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_IS_FAT_PIPE_MAL                                                   ((JER2_ARAD_INTERNAL_FUNCS_BASE +  812)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DIAG_TBLS_DUMP_ALL                                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE +  813)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_OFP_FAT_PIPE_RATE_SET_UNSAFE                                          ((JER2_ARAD_INTERNAL_FUNCS_BASE +  814)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_OFP_FAT_PIPE_RATE_VERIFY                                              ((JER2_ARAD_INTERNAL_FUNCS_BASE +  815)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_OFP_FAT_PIPE_RATE_GET_UNSAFE                                          ((JER2_ARAD_INTERNAL_FUNCS_BASE +  816)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_OFP_FAT_PIPE_RATE_SET                                                 ((JER2_ARAD_INTERNAL_FUNCS_BASE +  817)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_OFP_FAT_PIPE_RATE_GET                                                 ((JER2_ARAD_INTERNAL_FUNCS_BASE +  818)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MC_REGS_INIT                                                          ((JER2_ARAD_INTERNAL_FUNCS_BASE +  819)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IHP_KEY_PROGRAM_TBL_GET_UNSAFE                                        ((JER2_ARAD_INTERNAL_FUNCS_BASE +  820)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IHP_KEY_PROGRAM_TBL_SET_UNSAFE                                        ((JER2_ARAD_INTERNAL_FUNCS_BASE +  821)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_RATE_SET                                                          ((JER2_ARAD_INTERNAL_FUNCS_BASE +  822)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_RATE_GET                                                          ((JER2_ARAD_INTERNAL_FUNCS_BASE +  823)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_TX_PHYS_PARAMS_SET                                                ((JER2_ARAD_INTERNAL_FUNCS_BASE +  824)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_TX_PHYS_PARAMS_GET                                                ((JER2_ARAD_INTERNAL_FUNCS_BASE +  825)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_LANE_POWER_STATE_SET                                              ((JER2_ARAD_INTERNAL_FUNCS_BASE +  826)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_LANE_POWER_STATE_GET                                              ((JER2_ARAD_INTERNAL_FUNCS_BASE +  827)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_STAR_SET                                                          ((JER2_ARAD_INTERNAL_FUNCS_BASE +  828)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_STAR_GET                                                          ((JER2_ARAD_INTERNAL_FUNCS_BASE +  829)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_ALL_SET                                                           ((JER2_ARAD_INTERNAL_FUNCS_BASE +  830)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_ALL_GET                                                           ((JER2_ARAD_INTERNAL_FUNCS_BASE +  831)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_RATE_SET_UNSAFE                                                   ((JER2_ARAD_INTERNAL_FUNCS_BASE +  832)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_RATE_VERIFY                                                       ((JER2_ARAD_INTERNAL_FUNCS_BASE +  833)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_RATE_GET_UNSAFE                                                   ((JER2_ARAD_INTERNAL_FUNCS_BASE +  834)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_TX_PHYS_PARAMS_SET_UNSAFE                                         ((JER2_ARAD_INTERNAL_FUNCS_BASE +  835)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_TX_PHYS_PARAMS_VERIFY                                             ((JER2_ARAD_INTERNAL_FUNCS_BASE +  836)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_TX_PHYS_PARAMS_GET_UNSAFE                                         ((JER2_ARAD_INTERNAL_FUNCS_BASE +  837)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_LANE_POWER_STATE_SET_UNSAFE                                       ((JER2_ARAD_INTERNAL_FUNCS_BASE +  838)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_LANE_POWER_STATE_VERIFY                                           ((JER2_ARAD_INTERNAL_FUNCS_BASE +  839)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_LANE_POWER_STATE_GET_UNSAFE                                       ((JER2_ARAD_INTERNAL_FUNCS_BASE +  840)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_STAR_SET_UNSAFE                                                   ((JER2_ARAD_INTERNAL_FUNCS_BASE +  841)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_STAR_VERIFY                                                       ((JER2_ARAD_INTERNAL_FUNCS_BASE +  842)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_STAR_GET_UNSAFE                                                   ((JER2_ARAD_INTERNAL_FUNCS_BASE +  843)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_ALL_SET_UNSAFE                                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE +  844)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_ALL_VERIFY                                                        ((JER2_ARAD_INTERNAL_FUNCS_BASE +  845)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_ALL_GET_UNSAFE                                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE +  846)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_INTERNAL_RATE_CALC                                                ((JER2_ARAD_INTERNAL_FUNCS_BASE +  847)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SW_DB_DEVICE_SRD_INIT                                                 ((JER2_ARAD_INTERNAL_FUNCS_BASE +  848)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_INTERNAL_RATE_SET                                                 ((JER2_ARAD_INTERNAL_FUNCS_BASE +  849)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_INTERNAL_RATE_GET                                                 ((JER2_ARAD_INTERNAL_FUNCS_BASE +  850)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_RATE_GET_AND_VALIDATE                                             ((JER2_ARAD_INTERNAL_FUNCS_BASE +  851)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_TX_PHYS_PARAMS_CALCULATE                                          ((JER2_ARAD_INTERNAL_FUNCS_BASE +  852)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_TX_PHYS_EXPLICIT_TO_INTERN                                        ((JER2_ARAD_INTERNAL_FUNCS_BASE +  853)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_CMU_TRIM                                                          ((JER2_ARAD_INTERNAL_FUNCS_BASE +  854)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_OFP_RATES_SW_DB_READ                                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE +  855)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_MAC_RATE_SET                                                      ((JER2_ARAD_INTERNAL_FUNCS_BASE +  856)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_AUTO_EQUALIZE                                                     ((JER2_ARAD_INTERNAL_FUNCS_BASE +  859)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_AUTO_EQUALIZE_UNSAFE                                              ((JER2_ARAD_INTERNAL_FUNCS_BASE +  860)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_RX_PHYS_PARAMS_SET                                                ((JER2_ARAD_INTERNAL_FUNCS_BASE +  861)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_RX_PHYS_PARAMS_GET                                                ((JER2_ARAD_INTERNAL_FUNCS_BASE +  862)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_RX_PHYS_PARAMS_SET_UNSAFE                                         ((JER2_ARAD_INTERNAL_FUNCS_BASE +  863)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_RX_PHYS_PARAMS_GET_UNSAFE                                         ((JER2_ARAD_INTERNAL_FUNCS_BASE +  864)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_RX_PHYS_PARAMS_VERIFY                                             ((JER2_ARAD_INTERNAL_FUNCS_BASE +  865)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SW_DB_DEVICE_LBG_INIT                                                 ((JER2_ARAD_INTERNAL_FUNCS_BASE +  866)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_PRBS_MODE_VERIFY                                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE +  867)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_PRBS_MODE_GET_UNSAFE                                              ((JER2_ARAD_INTERNAL_FUNCS_BASE +  868)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_PRBS_START_UNSAFE                                                 ((JER2_ARAD_INTERNAL_FUNCS_BASE +  869)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_PRBS_STOP_UNSAFE                                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE +  870)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_PRBS_GET_AND_CLEAR_STAT_UNSAFE                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE +  871)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_PRBS_MODE_SET                                                     ((JER2_ARAD_INTERNAL_FUNCS_BASE +  872)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_PRBS_MODE_GET                                                     ((JER2_ARAD_INTERNAL_FUNCS_BASE +  873)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_PRBS_START                                                        ((JER2_ARAD_INTERNAL_FUNCS_BASE +  874)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_PRBS_STOP                                                         ((JER2_ARAD_INTERNAL_FUNCS_BASE +  875)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_PRBS_GET_AND_CLEAR_STAT                                           ((JER2_ARAD_INTERNAL_FUNCS_BASE +  876)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_PRBS_CLEAR_STATUSES                                               ((JER2_ARAD_INTERNAL_FUNCS_BASE +  877)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DIAG_LPM_PORT_STATE_SAVE                                              ((JER2_ARAD_INTERNAL_FUNCS_BASE +  878)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DIAG_LPM_PORT_STATE_LOAD_UNSAFE                                       ((JER2_ARAD_INTERNAL_FUNCS_BASE +  879)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DIAG_LBG_CONF_SET                                                     ((JER2_ARAD_INTERNAL_FUNCS_BASE +  880)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DIAG_LBG_CONF_SET_UNSAFE                                              ((JER2_ARAD_INTERNAL_FUNCS_BASE +  881)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DIAG_LBG_TRAFFIC_SEND                                                 ((JER2_ARAD_INTERNAL_FUNCS_BASE +  882)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DIAG_LBG_TRAFFIC_SEND_UNSAFE                                          ((JER2_ARAD_INTERNAL_FUNCS_BASE +  883)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DIAG_LBG_RESULT_GET                                                   ((JER2_ARAD_INTERNAL_FUNCS_BASE +  884)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DIAG_LBG_RESULT_GET_UNSAFE                                            ((JER2_ARAD_INTERNAL_FUNCS_BASE +  885)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DIAG_LBG_CLOSE                                                        ((JER2_ARAD_INTERNAL_FUNCS_BASE +  886)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DIAG_LBG_CLOSE_UNSAFE                                                 ((JER2_ARAD_INTERNAL_FUNCS_BASE +  887)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_ALL_NIFS_ALL_COUNTERS_GET                                         ((JER2_ARAD_INTERNAL_FUNCS_BASE +  888)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_ALL_NIFS_ALL_COUNTERS_GET_UNSAFE                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE +  889)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_INIT_DRAM_NOF_BUFFS_CALC                                              ((JER2_ARAD_INTERNAL_FUNCS_BASE +  890)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_INIT_DRAM_BUFF_BOUNDARIES_CALC                                        ((JER2_ARAD_INTERNAL_FUNCS_BASE +  891)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPQ_QUEUE_ID_VERIFY                                                   ((JER2_ARAD_INTERNAL_FUNCS_BASE +  892)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_SGMII_RATE_SET                                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE +  893)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_SGMII_RATE_GET                                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE +  894)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_SGMII_RATE_SET_UNSAFE                                             ((JER2_ARAD_INTERNAL_FUNCS_BASE +  895)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_SGMII_RATE_VERIFY                                                 ((JER2_ARAD_INTERNAL_FUNCS_BASE +  896)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_SGMII_RATE_GET_UNSAFE                                             ((JER2_ARAD_INTERNAL_FUNCS_BASE +  897)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_SGMII_STATUS_GET_UNSAFE                                           ((JER2_ARAD_INTERNAL_FUNCS_BASE +  898)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_SGMII_STATUS_GET                                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE +  899)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_OFP_ALL_OFP_RATES_GET                                                 ((JER2_ARAD_INTERNAL_FUNCS_BASE +  900)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_OFP_ALL_OFP_RATES_GET_UNSAFE                                          ((JER2_ARAD_INTERNAL_FUNCS_BASE +  901)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_OFP_RATES_SPECIAL_ID_DETECT                                           ((JER2_ARAD_INTERNAL_FUNCS_BASE +  902)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_INTERRUPT_MODULE_INITIALIZE                                           ((JER2_ARAD_INTERNAL_FUNCS_BASE +  903)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_INTERRUPT_INFO_GET                                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE +  904)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_INTERRUPT_MASK_ALL_UNSAFE                                             ((JER2_ARAD_INTERNAL_FUNCS_BASE +  905)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_INTERRUPT_UNMASK_ALL_UNSAFE                                           ((JER2_ARAD_INTERNAL_FUNCS_BASE +  906)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_INTERRUPT_MASK_CLEAR_UNSAFE                                           ((JER2_ARAD_INTERNAL_FUNCS_BASE +  907)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_INTERRUPT_IS_ALL_MASKED_GET                                           ((JER2_ARAD_INTERNAL_FUNCS_BASE +  908)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_INTERRUPT_SPECIFIC_CAUSE_MONITOR_START_UNSAFE                         ((JER2_ARAD_INTERNAL_FUNCS_BASE +  909)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_INTERRUPT_SPECIFIC_CAUSE_MONITOR_STOP_UNSAFE                          ((JER2_ARAD_INTERNAL_FUNCS_BASE +  910)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_INTERRUPT_SPECIFIC_CAUSE_MASK_BIT_SET_UNSAFE                          ((JER2_ARAD_INTERNAL_FUNCS_BASE +  911)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_INTERRUPT_SPECIFIC_CAUSE_MASK_BIT_GET_UNSAFE                          ((JER2_ARAD_INTERNAL_FUNCS_BASE +  912)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_INTERRUPT_MASK_GET                                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE +  913)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_INTERRUPT_LEVEL_GET                                                   ((JER2_ARAD_INTERNAL_FUNCS_BASE +  914)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_INTERRUPT_INITIAL_MASK_LIFT                                           ((JER2_ARAD_INTERNAL_FUNCS_BASE +  915)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_INTERRUPT_SINGLE_GET                                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE +  916)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_INTERRUPT_SINGLE_INFO_READ                                            ((JER2_ARAD_INTERNAL_FUNCS_BASE +  917)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_INTERRUPT_INFO_READ                                                   ((JER2_ARAD_INTERNAL_FUNCS_BASE +  918)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_INTERRUPT_GET                                                         ((JER2_ARAD_INTERNAL_FUNCS_BASE +  919)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_INTERRUPT_SPECIFIC_CAUSE_MONITOR_START                                ((JER2_ARAD_INTERNAL_FUNCS_BASE +  920)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_INTERRUPT_SPECIFIC_CAUSE_MONITOR_STOP                                 ((JER2_ARAD_INTERNAL_FUNCS_BASE +  921)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_INTERRUPT_SPECIFIC_CAUSE_MASK_BIT_SET                                 ((JER2_ARAD_INTERNAL_FUNCS_BASE +  922)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_INTERRUPT_SPECIFIC_CAUSE_MASK_BIT_GET                                 ((JER2_ARAD_INTERNAL_FUNCS_BASE +  923)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_INTERRUPT_MASK_ALL                                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE +  924)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_INTERRUPT_UNMASK_ALL                                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE +  925)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_INTERRUPT_MASK_CLEAR                                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE +  926)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_INTERRUPT_HANDLER_UNSAFE                                              ((JER2_ARAD_INTERNAL_FUNCS_BASE +  927)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_CALLBACK_ALL_FUNCTIONS_UNREGISTER_UNSAFE                              ((JER2_ARAD_INTERNAL_FUNCS_BASE +  928)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_CALLBACK_FUNCTION_REGISTER_UNSAFE                                     ((JER2_ARAD_INTERNAL_FUNCS_BASE +  929)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_CALLBACK_USER_CALLBACK_FUNCTION                                       ((JER2_ARAD_INTERNAL_FUNCS_BASE +  930)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_INTERRUPT_CLEAR_ALL                                                   ((JER2_ARAD_INTERNAL_FUNCS_BASE +  931)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGR_UNSCHED_DROP_PRIO_SET                                             ((JER2_ARAD_INTERNAL_FUNCS_BASE +  932)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGR_UNSCHED_DROP_PRIO_GET                                             ((JER2_ARAD_INTERNAL_FUNCS_BASE +  933)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGR_UNSCHED_DROP_PRIO_SET_UNSAFE                                      ((JER2_ARAD_INTERNAL_FUNCS_BASE +  934)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGR_UNSCHED_DROP_PRIO_VERIFY                                          ((JER2_ARAD_INTERNAL_FUNCS_BASE +  935)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_EGR_UNSCHED_DROP_PRIO_GET_UNSAFE                                      ((JER2_ARAD_INTERNAL_FUNCS_BASE +  936)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPQ_BASE_Q_DFLT_INVALID_SET_UNSAFE                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE +  937)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPQ_BASE_Q_IS_VALID_GET_UNSAFE                                        ((JER2_ARAD_INTERNAL_FUNCS_BASE +  938)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPQ_INIT                                                              ((JER2_ARAD_INTERNAL_FUNCS_BASE +  939)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SSR_FORBIDDEN_VER_SIZE                                                ((JER2_ARAD_INTERNAL_FUNCS_BASE +  940)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SSR_FORBIDDEN_VER_TRANS                                               ((JER2_ARAD_INTERNAL_FUNCS_BASE +  941)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SSR_MAX_SW_DB_SIZE_GET                                                ((JER2_ARAD_INTERNAL_FUNCS_BASE +  942)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SSR_SW_DB_SIZE_GET                                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE +  943)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SSR_CFG_VERSION_GET                                                   ((JER2_ARAD_INTERNAL_FUNCS_BASE +  944)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SSR_TRANSFORM_DB2CURR                                                 ((JER2_ARAD_INTERNAL_FUNCS_BASE +  945)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SSR_DATA_LOAD                                                         ((JER2_ARAD_INTERNAL_FUNCS_BASE +  946)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SSR_DATA_SAVE                                                         ((JER2_ARAD_INTERNAL_FUNCS_BASE +  947)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SSR_BUFFER_SAVE                                                       ((JER2_ARAD_INTERNAL_FUNCS_BASE +  948)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SSR_BUFFER_LOAD                                                       ((JER2_ARAD_INTERNAL_FUNCS_BASE +  949)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SSR_BUFF_SIZE_GET                                                     ((JER2_ARAD_INTERNAL_FUNCS_BASE +  950)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_LOOPBACK_SET                                                      ((JER2_ARAD_INTERNAL_FUNCS_BASE +  951)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_LOOPBACK_GET                                                      ((JER2_ARAD_INTERNAL_FUNCS_BASE +  952)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_LOOPBACK_SET_UNSAFE                                               ((JER2_ARAD_INTERNAL_FUNCS_BASE +  953)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_LOOPBACK_VERIFY                                                   ((JER2_ARAD_INTERNAL_FUNCS_BASE +  954)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_LOOPBACK_GET_UNSAFE                                               ((JER2_ARAD_INTERNAL_FUNCS_BASE +  955)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MGMT_PCKT_SIZE_RANGE_SET                                              ((JER2_ARAD_INTERNAL_FUNCS_BASE +  956)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MGMT_PCKT_SIZE_RANGE_GET                                              ((JER2_ARAD_INTERNAL_FUNCS_BASE +  957)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MGMT_PCKT_SIZE_RANGE_SET_UNSAFE                                       ((JER2_ARAD_INTERNAL_FUNCS_BASE +  958)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MGMT_PCKT_SIZE_RANGE_VERIFY                                           ((JER2_ARAD_INTERNAL_FUNCS_BASE +  959)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MGMT_PCKT_SIZE_RANGE_GET_UNSAFE                                       ((JER2_ARAD_INTERNAL_FUNCS_BASE +  960)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_TEXT_ERR_GET                                                          ((JER2_ARAD_INTERNAL_FUNCS_BASE +  961)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_STAR_RESET                                                        ((JER2_ARAD_INTERNAL_FUNCS_BASE +  962)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_TOPOLOGY_STATUS_CONNECTIVITY_GET                                      ((JER2_ARAD_INTERNAL_FUNCS_BASE +  963)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_TOPOLOGY_STATUS_CONNECTIVITY_GET_UNSAFE                               ((JER2_ARAD_INTERNAL_FUNCS_BASE +  964)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_TOPOLOGY_STATUS_CONNECTIVITY_PRINT                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE +  965)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_LINKS_STATUS_GET                                                      ((JER2_ARAD_INTERNAL_FUNCS_BASE +  966)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_TRANSACTION_WITH_FE600                                                ((JER2_ARAD_INTERNAL_FUNCS_BASE +  967)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_READ_FROM_FE600_UNSAFE                                                ((JER2_ARAD_INTERNAL_FUNCS_BASE +  968)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_WRITE_FROM_FE600_UNSAFE                                               ((JER2_ARAD_INTERNAL_FUNCS_BASE +  969)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_INDIRECT_READ_FROM_FE600_UNSAFE                                       ((JER2_ARAD_INTERNAL_FUNCS_BASE +  970)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_INDIRECT_WRITE_FROM_FE600_UNSAFE                                      ((JER2_ARAD_INTERNAL_FUNCS_BASE +  971)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SR_SEND_CELL                                                          ((JER2_ARAD_INTERNAL_FUNCS_BASE +  972)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SR_RCV_CELL                                                           ((JER2_ARAD_INTERNAL_FUNCS_BASE +  973)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SR_SEND_AND_WAIT_ACK                                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE +  974)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_READ_FROM_FE600                                                       ((JER2_ARAD_INTERNAL_FUNCS_BASE +  975)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_WRITE_FROM_FE600                                                      ((JER2_ARAD_INTERNAL_FUNCS_BASE +  976)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_INDIRECT_READ_FROM_FE600                                              ((JER2_ARAD_INTERNAL_FUNCS_BASE +  977)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_INDIRECT_WRITE_FROM_FE600                                             ((JER2_ARAD_INTERNAL_FUNCS_BASE +  978)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_WRITE_INDIRECT_OFFSET_TO_RTP_TABLE                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE +  979)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_IVCDL_SET                                                         ((JER2_ARAD_INTERNAL_FUNCS_BASE +  980)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DBG_ROUTE_FORCE_SET                                                   ((JER2_ARAD_INTERNAL_FUNCS_BASE +  981)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DBG_ROUTE_FORCE_SET_UNSAFE                                            ((JER2_ARAD_INTERNAL_FUNCS_BASE +  982)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DBG_ROUTE_FORCE_GET                                                   ((JER2_ARAD_INTERNAL_FUNCS_BASE +  983)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DBG_ROUTE_FORCE_VERIFY                                                ((JER2_ARAD_INTERNAL_FUNCS_BASE +  984)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DBG_ROUTE_FORCE_GET_UNSAFE                                            ((JER2_ARAD_INTERNAL_FUNCS_BASE +  985)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DBG_AUTOCREDIT_SET                                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE +  986)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DBG_AUTOCREDIT_SET_UNSAFE                                             ((JER2_ARAD_INTERNAL_FUNCS_BASE +  987)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DBG_AUTOCREDIT_GET                                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE +  988)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DBG_AUTOCREDIT_VERIFY                                                 ((JER2_ARAD_INTERNAL_FUNCS_BASE +  989)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DBG_AUTOCREDIT_GET_UNSAFE                                             ((JER2_ARAD_INTERNAL_FUNCS_BASE +  990)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DBG_EGRESS_SHAPING_ENABLE_SET                                         ((JER2_ARAD_INTERNAL_FUNCS_BASE +  991)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DBG_EGRESS_SHAPING_ENABLE_GET                                         ((JER2_ARAD_INTERNAL_FUNCS_BASE +  992)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DBG_EGRESS_SHAPING_ENABLE_VERIFY                                      ((JER2_ARAD_INTERNAL_FUNCS_BASE +  993)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DBG_EGRESS_SHAPING_ENABLE_GET_UNSAFE                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE +  994)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DBG_FLOW_CONTROL_ENABLE_SET                                           ((JER2_ARAD_INTERNAL_FUNCS_BASE +  995)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DBG_FLOW_CONTROL_ENABLE_SET_UNSAFE                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE +  996)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DBG_FLOW_CONTROL_ENABLE_GET                                           ((JER2_ARAD_INTERNAL_FUNCS_BASE +  997)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DBG_FLOW_CONTROL_ENABLE_VERIFY                                        ((JER2_ARAD_INTERNAL_FUNCS_BASE +  998)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DBG_FLOW_CONTROL_ENABLE_GET_UNSAFE                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE +  999)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_OFP_RATES_PORT_MAL_GET_UNSAFE                                         ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1000)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DBG_RATE2AUTOCREDIT_RATE                                              ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1001)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DBG_AUTOCREDIT_RATE2RATE                                              ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1002)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PORT_EGR_HDR_DISCOUNT_SELECT_SET_UNSAFE                               ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1003)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PORT_EGR_HDR_DISCOUNT_SELECT_VERIFY                                   ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1004)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PORT_EGR_HDR_DISCOUNT_SELECT_GET_UNSAFE                               ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1005)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PORT_EGR_HDR_DISCOUNT_TYPE_SET_UNSAFE                                 ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1006)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PORT_EGR_HDR_DISCOUNT_VERIFY                                          ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1007)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PORT_EGR_HDR_DISCOUNT_TYPE_GET_UNSAFE                                 ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1008)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PORT_EGR_HDR_DISCOUNT_SELECT_SET                                      ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1009)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PORT_EGR_HDR_DISCOUNT_SELECT_GET                                      ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1010)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PORT_EGR_HDR_DISCOUNT_TYPE_SET                                        ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1011)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PORT_EGR_HDR_DISCOUNT_TYPE_GET                                        ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1012)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FC_EGR_REC_OOB_STAT_GET_UNSAFE                                        ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1013)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FC_EGR_REC_OOB_STAT_GET                                               ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1014)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_CPU2CPU_WITH_FE600_WRITE_UNSAFE                                       ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1015)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_CPU2CPU_WITH_FE600_WRITE                                              ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1016)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_BUILD_DATA_CELL_FOR_FE600                                             ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1017)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_CPU2CPU_WITH_FE600_READ_UNSAFE                                        ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1018)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_CPU2CPU_WITH_FE600_READ                                               ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1019)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_INVERT_POLARITY_FROM_ORIG                                         ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1020)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_INIT_MEM_CORRECTION_ENABLE                                            ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1021)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_LOCAL_FAULT_OVRD_SET                                              ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1022)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_LOCAL_FAULT_OVRD_SET_UNSAFE                                       ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1023)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_LOCAL_FAULT_OVRD_GET                                              ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1024)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_LOCAL_FAULT_OVRD_GET_UNSAFE                                       ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1025)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_LOCAL_FAULT_OVRD_VERIFY                                           ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1026)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_EYE_SCAN_CALC_OPTIMUM                                             ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1027)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_SGMII_ENABLE_STATE_SET                                            ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1028)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_SGMII_ENABLE_STATE_GET                                            ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1029)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_OFP_RATES_EGQ_CALENDAR_VALIDATE                                       ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1030)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_OFP_RATES_EGQ_CALENDAR_VALIDATE_UNSAFE                                ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1031)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_OFP_RATES_TEST_SETTINGS                                               ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1032)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_OFP_RATES_TEST_SETTINGS_UNSAFE                                        ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1033)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_OFP_RATES_TEST_RANDOM                                                 ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1034)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_OFP_RATES_TEST_RANDOM_UNSAFE                                          ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1035)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_STATUS_FLD_POLL                                                       ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1036)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_STATUS_FLD_POLL_UNSAFE                                                ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1037)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DRAM_INFO_SET                                                         ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1038)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DRAM_INFO_SET_UNSAFE                                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1039)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DRAM_INFO_GET                                                         ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1040)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DRAM_INFO_VERIFY                                                      ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1041)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DRAM_INFO_GET_UNSAFE                                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1042)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FC_EGR_REC_OOB_TABLES_SET_UNSAFE                                      ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1043)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_LINK_ON_OFF_SET                                                       ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1044)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_LINK_ON_OFF_GET                                                       ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1045)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_LINK_ON_OFF_SET_UNSAFE                                                ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1046)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_LINK_ON_OFF_VERIFY                                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1047)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_LINK_ON_OFF_GET_UNSAFE                                                ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1048)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_OFP_RATES_INIT                                                        ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1049)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_PRBS_MODE_SET_UNSAFE                                              ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1050)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DIAG_BIST_BASIC_WRITE                                                 ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1051)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DIAG_BIST_PRESETTINGS_SET                                             ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1052)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DIAG_BIST_BASIC_READ_AND_COMPARE                                      ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1053)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DIAG_BIST_WRITE                                                       ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1054)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DIAG_BIST_WRITE_UNSAFE                                                ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1055)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DIAG_BIST_READ_AND_COMPARE                                            ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1056)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DIAG_BIST_READ_AND_COMPARE_UNSAFE                                     ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1057)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_STAT_IF_INFO_SET                                                      ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1058)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_STAT_IF_INFO_SET_UNSAFE                                               ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1059)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_STAT_IF_INFO_GET                                                      ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1060)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_STAT_IF_INFO_VERIFY                                                   ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1061)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_STAT_IF_INFO_GET_UNSAFE                                               ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1062)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_STAT_IF_REPORT_INFO_SET                                               ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1063)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_STAT_IF_REPORT_INFO_SET_UNSAFE                                        ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1064)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_STAT_IF_REPORT_INFO_GET                                               ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1065)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_STAT_IF_REPORT_INFO_VERIFY                                            ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1066)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_STAT_IF_REPORT_INFO_GET_UNSAFE                                        ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1067)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DIAG_INIT                                                             ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1068)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DIAG_REGS_INIT                                                        ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1069)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DIAG_OFFSET2BIST_ADDR_CONVERT                                         ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1070)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_QRTT_SET_UNSAFE                                                   ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1071)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_QRTT_VERIFY                                                       ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1072)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_QRTT_GET_UNSAFE                                                   ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1073)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_QRTT_SET                                                          ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1074)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_QRTT_GET                                                          ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1075)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_SCIF_ENABLE_SET                                                   ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1076)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_SCIF_ENABLE_GET                                                   ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1077)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_SCIF_ENABLE_SET_UNSAFE                                            ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1078)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_SCIF_ENABLE_VERIFY                                                ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1079)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_SCIF_ENABLE_GET_UNSAFE                                            ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1080)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_QRTT_RESET                                                        ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1081)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_QRTT_RESET_UNSAFE                                                 ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1082)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_VALIDATE_AND_RELOCK                                               ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1083)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_VSQ_IDX_VERIFY                                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1084)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_VSQ_COUNTER_SET                                                   ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1085)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_VSQ_COUNTER_GET                                                   ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1086)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_VSQ_COUNTER_READ                                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1087)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_VSQ_COUNTER_VERIFY                                                ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1088)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_VSQ_COUNTER_SET_UNSAFE                                            ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1089)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_VSQ_COUNTER_GET_UNSAFE                                            ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1090)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_VSQ_COUNTER_READ_UNSAFE                                           ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1091)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PKT_PACKET_CALLBACK_SET                                               ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1092)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PKT_PACKET_CALLBACK_SET_UNSAFE                                        ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1093)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PKT_PACKET_CALLBACK_GET                                               ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1094)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PKT_PACKET_CALLBACK_VERIFY                                            ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1095)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PKT_PACKET_CALLBACK_GET_UNSAFE                                        ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1096)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PKT_INIT                                                              ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1097)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PKT_ASYNC_INTERFACE_PACKET_SEND                                       ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1098)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PKT_ASYNC_INTERFACE_PACKET_RECV                                       ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1099)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PKT_PACKET_CALLBACK_SET_UNSAFE2                                       ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1100)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DIAG_DLL_STATUS_GET                                                   ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1101)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DIAG_DLL_STATUS_GET_UNSAFE                                            ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1102)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DIAG_DRAM_STATUS_GET                                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1103)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DIAG_DRAM_STATUS_GET_UNSAFE                                           ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1104)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_STATUS_FLD_POLL_UNSAFE                                            ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1105)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FABRIC_SRD_QRTT_RESET                                                 ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1106)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_ON_OFF_SET                                                        ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1107)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_ON_OFF_GET                                                        ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1108)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FABRIC_SRD_QRTT_RESET_UNSAFE                                          ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1109)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_ON_OFF_SET_UNSAFE                                                 ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1110)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_ON_OFF_VERIFY                                                     ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1111)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_NIF_ON_OFF_GET_UNSAFE                                                 ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1112)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_TX_PHYS_EXPLICITE_TO_INTERN                                       ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1113)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MGMT_CTRL_CELLS_COUNTER_CLEAR                                         ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1114)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PORTS_LAG_MEMBER_ADD                                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1115)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PORTS_LAG_MEMBER_ADD_UNSAFE                                           ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1116)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DBG_QUEUE_FLUSH                                                       ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1117)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DBG_QUEUE_FLUSH_UNSAFE                                                ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1118)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DBG_QUEUE_FLUSH_ALL                                                   ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1119)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DBG_QUEUE_FLUSH_ALL_UNSAFE                                            ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1120)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PKT_PACKET_RECEIVE_MODE_SET                                           ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1121)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PKT_PACKET_RECEIVE_MODE_SET_UNSAFE                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1122)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PKT_PACKET_RECEIVE_MODE_GET                                           ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1123)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PKT_PACKET_RECEIVE_MODE_VERIFY                                        ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1124)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PKT_PACKET_RECEIVE_MODE_GET_UNSAFE                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1125)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_OFP_RATES_UPDATE_DEVICE_SET                                           ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1126)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_OFP_RATES_UPDATE_DEVICE_SET_UNSAFE                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1127)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_OFP_RATES_UPDATE_DEVICE_GET                                           ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1128)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_OFP_RATES_UPDATE_DEVICE_GET_UNSAFE                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1129)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DBG_QUEUE_FLUSH_INTERNAL_UNSAFE                                       ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1130)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DBG_INGR_RESET                                                        ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1131)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DBG_INGR_RESET_UNSAFE                                                 ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1132)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_RELOCK                                                            ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1133)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_RELOCK_UNSAFE                                                     ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1134)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_FABRIC_ENHANCED_SET                                              ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1135)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_FABRIC_ENHANCED_SET_UNSAFE                                       ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1136)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_FABRIC_ENHANCED_GET                                              ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1137)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_FABRIC_ENHANCED_SET_VERIFY                                       ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1138)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_FABRIC_ENHANCED_GET_UNSAFE                                       ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1139)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DIAG_DRAM_DIAGNOSTIC_GET                                              ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1140)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DIAG_DRAM_DIAGNOSTIC_GET_UNSAFE                                       ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1141)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DIAG_DRAM_DIAGNOSTIC_GET_VERIFY                                       ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1142)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_RPLCT_TBL_ENTRY_UNOCCUPIED_SET                                   ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1143)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_RPLCT_TBL_ENTRY_UNOCCUPIED_SET_ALL                               ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1144)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_RPLCT_TBL_ENTRY_IS_OCCUPIED                                      ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1145)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_RPLCT_TBL_ENTRY_IS_EMPTY                                         ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1146)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_MAL_RATE_SET_UNSAFE                                               ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1147)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_MAL_RATE_GET_UNSAFE                                               ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1148)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SSR_IS_DEVICE_INIT_DONE                                               ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1149)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SSR_IS_DEVICE_INIT_DONE_UNSAFE                                        ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1150)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DBG_ECI_ACCESS_TST                                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1151)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DBG_ECI_ACCESS_TST_UNSAFE                                             ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1152)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPQ_QUEUE_QRTT_UNMAP                                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1153)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPQ_QUEUE_QRTT_UNMAP_UNSAFE                                           ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1154)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_CELL_MC_TBL_WRITE                                                     ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1155)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_CELL_MC_TBL_WRITE_PRINT                                               ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1156)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_CELL_MC_TBL_WRITE_UNSAFE                                              ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1157)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_CELL_MC_TBL_WRITE_VERIFY                                              ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1158)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_CELL_MC_TBL_READ                                                      ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1159)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_CELL_MC_TBL_READ_PRINT                                                ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1160)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_CELL_MC_TBL_READ_UNSAFE                                               ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1161)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_CELL_MC_TBL_READ_VERIFY                                               ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1162)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_CELL_INIT                                                             ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1163)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_CELL_MC_TBL_INBAND_BUILD                                              ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1164)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_CELL_ACK_GET                                                          ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1165)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_STAT_IFP_CNT_SELECT_SET                                               ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1166)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_STAT_IFP_CNT_SELECT_SET_UNSAFE                                        ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1167)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_STAT_IFP_CNT_SELECT_GET_VERIFY                                        ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1168)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_STAT_IFP_CNT_SELECT_GET                                               ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1169)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_STAT_IFP_CNT_SELECT_SET_VERIFY                                        ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1170)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_STAT_IFP_CNT_SELECT_GET_UNSAFE                                        ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1171)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_STAT_VOQ_CNT_SELECT_SET                                               ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1172)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_STAT_VOQ_CNT_SELECT_SET_UNSAFE                                        ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1173)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_STAT_VOQ_CNT_SELECT_GET_VERIFY                                        ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1174)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_STAT_VOQ_CNT_SELECT_GET                                               ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1175)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_STAT_VOQ_CNT_SELECT_SET_VERIFY                                        ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1176)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_STAT_VOQ_CNT_SELECT_GET_UNSAFE                                        ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1177)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_STAT_VSQ_CNT_SELECT_SET                                               ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1178)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_STAT_VSQ_CNT_SELECT_SET_UNSAFE                                        ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1179)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_STAT_VSQ_CNT_SELECT_GET_VERIFY                                        ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1180)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_STAT_VSQ_CNT_SELECT_GET                                               ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1181)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_STAT_VSQ_CNT_SELECT_SET_VERIFY                                        ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1182)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_STAT_VSQ_CNT_SELECT_GET_UNSAFE                                        ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1183)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MGMT_CORE_FREQUENCY_GET                                               ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1184)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MGMT_CORE_FREQUENCY_GET_PRINT                                         ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1185)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MGMT_CORE_FREQUENCY_GET_UNSAFE                                        ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1186)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DRAM_WINDOW_VALIDITY_GET                                              ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1187)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DRAM_WINDOW_VALIDITY_GET_PRINT                                        ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1188)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DRAM_WINDOW_VALIDITY_GET_UNSAFE                                       ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1189)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DRAM_WINDOW_VALIDITY_GET_VERIFY                                       ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1190)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_EJER2_ARAD_ACCESS_VALIDATE_DONE                                        ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1191)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PA_EGR_UNSCHED_DROP_SET_UNSAFE                                        ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1192)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PA_EGR_UNSCHED_DROP_GET_UNSAFE                                        ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1193)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PA_SYS_PHYS_TO_LOCAL_PORT_MAP_SET_UNSAFE                              ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1196)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_STAT_IS_RELEVANT_COUNTER                                              ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1197)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_QRTT_STATUS_PRINT                                                 ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1250)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MC_INIT                                                               ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1251)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DBG_EGRESS_SHAPING_ENABLE_SET_UNSAFE                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1252)|JER2_ARAD_PROC_BITS)

#define JER2_ARAD_INTERRUPT_INIT                                                        ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1253)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_INTERRUPT_ALL_INTERRUPTS_AND_INDICATIONS_CLEAR_UNSAFE                 ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1254)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_QRTT_SYNC_FIFO_EN                                                 ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1261)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SRD_INIT                                                              ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1262)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_EG_GROUP_ENTRIES_SET                                             ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1263)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_EG_TBL_ENTRY_FORMAT_GET                                          ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1264)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MULT_EG_PROGRESS_INDEX_GET                                            ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1265)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IF_TYPE_FROM_ID                                                       ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1266)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DIAG_QDR_WINDOW_VALIDITY_GET_UNSAFE                                   ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1273)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DBG_PCM_READINGS_GET                                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1276)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DBG_PCM_READINGS_GET_UNSAFE                                           ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1277)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DBG_SCH_RESET                                                         ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1280)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DBG_SCH_RESET_UNSAFE                                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1281)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DBG_SCH_SUSPECT_SPR_DETECT                                            ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1282)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DBG_SCH_SUSPECT_SPR_DETECT_PRINT                                      ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1283)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DBG_SCH_SUSPECT_SPR_DETECT_UNSAFE                                     ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1284)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DBG_SCH_SUSPECT_SPR_DETECT_VERIFY                                     ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1285)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DBG_SCH_SUSPECT_SPR_CONFIRM                                           ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1286)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DBG_SCH_SUSPECT_SPR_CONFIRM_UNSAFE                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1287)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DBG_SCH_SUSPECT_SPR_CONFIRM_VERIFY                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1288)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_FLOW_IPF_CONFIG_MODE_SET_PRINT                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1289)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_FLOW_IPF_CONFIG_MODE_SET_UNSAFE                                   ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1290)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_FLOW_IPF_CONFIG_MODE_SET_VERIFY                                   ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1291)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_FLOW_IPF_CONFIG_MODE_GET                                          ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1292)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_FLOW_IPF_CONFIG_MODE_GET_PRINT                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1293)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_FLOW_IPF_CONFIG_MODE_GET_UNSAFE                                   ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1295)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SCH_FLOW_IPF_CONFIG_MODE_SET                                          ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1296)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DEBUG_GET_PROCS_PTR                                                   ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1297)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DEBUG_GET_ERRS_PTR                                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1298)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DIAG_SOFT_ERROR_TEST_START                                            ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1299)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DIAG_SOFT_ERROR_TEST_START_PRINT                                      ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1300)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DIAG_SOFT_ERROR_TEST_START_UNSAFE                                     ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1301)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DIAG_SOFT_ERROR_TEST_START_VERIFY                                     ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1302)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DIAG_SOFT_ERROR_TEST_RESULT_GET                                       ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1303)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DIAG_SOFT_ERROR_TEST_RESULT_GET_PRINT                                 ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1304)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DIAG_SOFT_ERROR_TEST_RESULT_GET_UNSAFE                                ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1305)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DIAG_SOFT_ERROR_TEST_RESULT_GET_VERIFY                                ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1306)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DBG_DEV_RESET                                                         ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1307)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DBG_DEV_RESET_PRINT                                                   ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1308)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DBG_DEV_RESET_UNSAFE                                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1309)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DBG_DPRC_RESET_UNSAFE                                                 ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1310)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DBG_DEV_RESET_VERIFY                                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1311)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_CTRL_CELLS_EN_FAST                                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1314)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SHD_INDIRECT_INIT                                                     ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1315)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SHD_INDIRECT_CLOSE                                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1316)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SHD_INDIRECT_FREE                                                     ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1317)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SHD_INDIRECT_MALLOC                                                   ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1318)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SHD_INDIRECT_WRITE                                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1319)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SHD_INDIRECT_OP_WRITE                                                 ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1320)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SHD_ENTRY_READ                                                        ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1321)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SHD_ENTRY_COMPARE                                                     ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1322)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SHD_ENTRY_WRITE                                                       ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1323)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SHD_INDIRECT_RANGE_COMPARE                                            ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1324)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_SHD_SCRUB_RANGE                                                       ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1325)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MGMT_INIT_SEQUENCE_FIXES_APPLY_UNSAFE                                 ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1326)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DRAM_PCKT_FROM_BUFF_READ                                              ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1327)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DRAM_PCKT_FROM_BUFF_READ_UNSAFE                                       ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1328)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DRAM_PCKT_FROM_BUFF_READ_VERIFY                                       ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1329)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_QDR_SET_REPS_FOR_TBL_UNSAFE                                           ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1330)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DBG_FAPS_WITH_NO_MESH_LINKS_GET                                       ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1331)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DBG_FAPS_WITH_NO_MESH_LINKS_GET_UNSAFE                                ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1332)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PORTS_EXPECTED_CHAN_GET                                               ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1340)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FABRIC_FC_SHAPER_SET                                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1341)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FABRIC_FC_SHAPER_GET                                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1342)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FABRIC_FC_SHAPER_SET_UNSAFE                                           ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1342)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FABRIC_FC_SHAPER_GET_UNSAFE                                           ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1343)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FABRIC_FC_SHAPER_VERIFY                                               ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1344)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DIAG_LAST_PACKET_INFO_GET_UNSAFE                                      ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1345)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DIAG_LAST_PACKET_INFO_GET_VERIFY                                      ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1345)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DIAG_LAST_PACKET_INFO_GET                                             ((JER2_ARAD_INTERNAL_FUNCS_BASE + 1346)|JER2_ARAD_PROC_BITS)

#define JER2_ARAD_ITM_VSQ_GROUP_E_SET_RT_CLASS                                          ((JER2_ARAD_INTERNAL_FUNCS_BASE +  1347)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_VSQ_GROUP_F_SET_RT_CLASS                                          ((JER2_ARAD_INTERNAL_FUNCS_BASE +  1348)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_VSQ_GROUP_E_GET_RT_CLASS                                          ((JER2_ARAD_INTERNAL_FUNCS_BASE +  1349)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_ITM_VSQ_GROUP_F_GET_RT_CLASS                                          ((JER2_ARAD_INTERNAL_FUNCS_BASE +  1350)|JER2_ARAD_PROC_BITS)

#define JER2_ARAD_IPQ_TC_PROFILE_GET_UNSAFE                                             ((JER2_ARAD_INTERNAL_FUNCS_BASE +  1351)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPQ_TC_PROFILE_SET_UNSAFE                                             ((JER2_ARAD_INTERNAL_FUNCS_BASE +  1352)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPQ_TC_PROFILE_VERIFY                                                 ((JER2_ARAD_INTERNAL_FUNCS_BASE +  1353)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPQ_TC_PROFILE_SET                                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE +  1354)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPQ_TC_PROFILE_GET                                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE +  1355)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPQ_TRAFFIC_CLASS_PROFILE_MAP_SET                                     ((JER2_ARAD_INTERNAL_FUNCS_BASE +  1356)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPQ_TRAFFIC_CLASS_PROFILE_MAP_GET                                     ((JER2_ARAD_INTERNAL_FUNCS_BASE +  1357)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_IPQ_TRAFFIC_CLASS_PROFILE_MAP_VERIFY                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE +  1358)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DRAM_INIT_WRITE_BYPASS_PLL_MODE                                       ((JER2_ARAD_INTERNAL_FUNCS_BASE +  1359)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DRAM_INIT_ENABLE_DDR_CLK_DIVIDERS                                     ((JER2_ARAD_INTERNAL_FUNCS_BASE +  1360)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DRAM_INIT_DRC_CLAM_SHELL_CFG                                          ((JER2_ARAD_INTERNAL_FUNCS_BASE +  1361)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DRAM_INIT_DPRC_OUT_OF_RESET                                           ((JER2_ARAD_INTERNAL_FUNCS_BASE +  1362)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DRAM_INIT_DRC_PARAM_REGISTER_WRITE                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE +  1379)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MGMT_DRAM_INIT_DRC_SOFT_INIT                                          ((JER2_ARAD_INTERNAL_FUNCS_BASE +  1380)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DRAM_INIT_WAIT_DRAM_INIT_FINISHED                                     ((JER2_ARAD_INTERNAL_FUNCS_BASE +  1382)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DRAM_INIT_DRC_INIT                                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE +  1383)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MGMT_INIT_DDR_CONFIGURE                                               ((JER2_ARAD_INTERNAL_FUNCS_BASE +  1384)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MGMT_INIT_PLL_RESET                                                   ((JER2_ARAD_INTERNAL_FUNCS_BASE +  1385)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_POLLING                                                               ((JER2_ARAD_INTERNAL_FUNCS_BASE +  1386)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_INIT_SERDES_PLL_SET                                                   ((JER2_ARAD_INTERNAL_FUNCS_BASE +  1387)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_STAT_IF_RATE_LIMIT_PRD_GET                                            ((JER2_ARAD_INTERNAL_FUNCS_BASE +  1388)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DRAM_RUBS_WRITE                                                       ((JER2_ARAD_INTERNAL_FUNCS_BASE +  1392)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DRAM_RUBS_WRITE_BR                                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE +  1393)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DRAM_RUBS_READ                                                        ((JER2_ARAD_INTERNAL_FUNCS_BASE +  1394)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DRAM_RUBS_MODIFY                                                      ((JER2_ARAD_INTERNAL_FUNCS_BASE +  1395)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DRAM_INIT_POWER_DOWN_UNUSED_DPRCS                                     ((JER2_ARAD_INTERNAL_FUNCS_BASE +  1396)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DRAM_INIT_CFG_DPRCS_LAST_IN_CHAIN                                     ((JER2_ARAD_INTERNAL_FUNCS_BASE +  1397)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PP_IHB_TCAM_ACCESS_PROFILE_TBL_READ_UNSAFE                            ((JER2_ARAD_INTERNAL_FUNCS_BASE +  1398)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PP_IHB_TCAM_ACCESS_PROFILE_TBL_WRITE_UNSAFE                           ((JER2_ARAD_INTERNAL_FUNCS_BASE +  1399)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PP_IHB_TCAM_PD_PROFILE_TBL_READ_UNSAFE                                ((JER2_ARAD_INTERNAL_FUNCS_BASE +  1400)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PP_IHB_TCAM_PD_PROFILE_TBL_WRITE_UNSAFE                               ((JER2_ARAD_INTERNAL_FUNCS_BASE +  1401)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PP_IHB_TCAM_TBL_MOVE_UNSAFE                                           ((JER2_ARAD_INTERNAL_FUNCS_BASE +  1402)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PP_IHB_TCAM_REP_VALID_POLLING                                         ((JER2_ARAD_INTERNAL_FUNCS_BASE +  1403)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PP_IHP_TCAM_TBL_READ_UNSAFE                                           ((JER2_ARAD_INTERNAL_FUNCS_BASE +  1404)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PP_IHB_TCAM_TBL_WRITE_UNSAFE                                          ((JER2_ARAD_INTERNAL_FUNCS_BASE +  1405)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PP_IHB_TCAM_VALID_BIT_TBL_SET_UNSAFE                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE +  1406)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PP_IHB_TCAM_VALID_BIT_TBL_GET_UNSAFE                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE +  1407)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PP_IHB_TCAM_TBL_FLUSH_UNSAFE                                          ((JER2_ARAD_INTERNAL_FUNCS_BASE +  1408)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PP_IHB_TCAM_TBL_COMPARE_UNSAFE                                        ((JER2_ARAD_INTERNAL_FUNCS_BASE +  1409)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DRAM_CONFIGURE_BIST                                                   ((JER2_ARAD_INTERNAL_FUNCS_BASE +  1410)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DRAM_BIST_ATOMIC_ACTION                                               ((JER2_ARAD_INTERNAL_FUNCS_BASE +  1413)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DRAM_READ_BIST_ERR_CNT                                                ((JER2_ARAD_INTERNAL_FUNCS_BASE +  1414)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DRAM_SET_MODE_REGS                                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE +  1436)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DRAM_INFO_SET_INTERNAL_UNSAFE                                         ((JER2_ARAD_INTERNAL_FUNCS_BASE +  1437)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DRAM_JER2_ARAD_DRAM_INFO_2_JER2_ARAD_DRAM_INTERNAL_INFO                         ((JER2_ARAD_INTERNAL_FUNCS_BASE +  1438)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DRAM_JER2_ARAD_DRAM_MR_INFO_2_JER2_ARAD_DRAM_INTERNAL_MR_INFO                   ((JER2_ARAD_INTERNAL_FUNCS_BASE +  1439)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DRAM_INIT_DRC_PHY_REGISTER_SET                                        ((JER2_ARAD_INTERNAL_FUNCS_BASE +  1440)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DRAM_BIST_TEST_START_VERIFY                                           ((JER2_ARAD_INTERNAL_FUNCS_BASE +  1443)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DRAM_BIST_TEST_START_UNSAFE                                           ((JER2_ARAD_INTERNAL_FUNCS_BASE +  1444)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DRAM_BIST_TEST_START                                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE +  1445)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DRAM_INIT_DDR_CONFIGURE_MMU                                           ((JER2_ARAD_INTERNAL_FUNCS_BASE +  1446)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_B_ITM_GLOB_RCS_DROP_VERIFY                                            ((JER2_ARAD_INTERNAL_FUNCS_BASE +  1447)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_PORT_EGR_EGQ_NIF_CANCEL_EN_REGS_SET                                   ((JER2_ARAD_INTERNAL_FUNCS_BASE +  1448)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FABRIC_LINK_UP_GET                                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE +  1449)|JER2_ARAD_PROC_BITS) 
#define JER2_ARAD_FABRIC_LINK_UP_GET_UNSAFE                                             ((JER2_ARAD_INTERNAL_FUNCS_BASE +  1450)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FABRIC_LINK_UP_GET_VERIFY                                              ((JER2_ARAD_INTERNAL_FUNCS_BASE +  1451)|JER2_ARAD_PROC_BITS) 
#define JER2_ARAD_FABRIC_PORT_SPEED_GET                                                ((JER2_ARAD_INTERNAL_FUNCS_BASE +  1452)|JER2_ARAD_PROC_BITS) 
#define JER2_ARAD_FABRIC_PORT_SPEED_SET                                                ((JER2_ARAD_INTERNAL_FUNCS_BASE +  1453)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FABRIC_PRIORITY_SET                                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE +  1454)|JER2_ARAD_PROC_BITS) 
#define JER2_ARAD_FABRIC_PRIORITY_GET                                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE +  1455)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_FABRIC_PRIORITY_BITS_MAPPING_TO_FDT_INDEX_GET                          ((JER2_ARAD_INTERNAL_FUNCS_BASE +  1456)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MGMT_IPT_INIT                                                          ((JER2_ARAD_INTERNAL_FUNCS_BASE +  1457)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_MGMT_MESH_TOPOLOGY_INIT                                                ((JER2_ARAD_INTERNAL_FUNCS_BASE +  1458)|JER2_ARAD_PROC_BITS)
#define JER2_ARAD_DRAM_INIT_DPRC_INIT                                                   ((JER2_ARAD_INTERNAL_FUNCS_BASE +  1459)|JER2_ARAD_PROC_BITS) 
#define JER2_ARAD_DRAM_INIT_TUNE_DDR                                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE +  1460)|JER2_ARAD_PROC_BITS) 
#define JER2_ARAD_MGMT_OCB_PRM_VALUE_TO_FIELD_VALUE                                     ((JER2_ARAD_INTERNAL_FUNCS_BASE +  1461)|JER2_ARAD_PROC_BITS) 
#define JER2_ARAD_SW_DB_MULTICAST_TERMINATE                                             ((JER2_ARAD_INTERNAL_FUNCS_BASE +  1462)|JER2_ARAD_PROC_BITS) 
#define JER2_ARAD_PP_LEM_ACCESS_NEXT_STAMP_GET                                          ((JER2_ARAD_INTERNAL_FUNCS_BASE +  1463)|JER2_ARAD_PROC_BITS) 
#define JER2_ARAD_PORT_RX_ENABLE_SET                                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE +  1464)|JER2_ARAD_PROC_BITS) 
#define JER2_ARAD_PORT_RX_ENABLE_GET                                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE +  1465)|JER2_ARAD_PROC_BITS) 
#define JER2_ARAD_PP_IHB_TCAM_TBL_BANK_OFFSET_GET                                       ((JER2_ARAD_INTERNAL_FUNCS_BASE +  1466)|JER2_ARAD_PROC_BITS) 
#define JER2_ARAD_SW_DB_LEVEL_INFO_GET                                                  ((JER2_ARAD_INTERNAL_FUNCS_BASE +  1467)|JER2_ARAD_PROC_BITS) 
#define JER2_ARAD_MGMT_SW_VER_UNSAFE                                                    ((JER2_ARAD_INTERNAL_FUNCS_BASE +  1468)|JER2_ARAD_PROC_BITS) 

typedef enum
{
  JER2_ARAD_INTERRUPT_GEN_CLEAR_READ = JER2_ARAD_SW_DB_LEVEL_INFO_GET + 1, 
  JER2_ARAD_INTERRUPT_GEN_CLEAR_WRITE,
  JER2_ARAD_OFP_RATES_SINGLE_PORT_SET_UNSAFE,
  JER2_ARAD_OFP_RATES_IF_ID2CHAN_ARB_GET_UNSAFE,
  JER2_ARAD_OFP_RATES_PORT_PRIORITY_SHAPER_SET,
  JER2_ARAD_OFP_RATES_PORT_PRIORITY_SHAPER_GET,
  JER2_ARAD_OFP_RATES_PORT_PRIORITY_SHAPER_SET_UNSAFE,
  JER2_ARAD_OFP_RATES_PORT_PRIORITY_SHAPER_GET_UNSAFE,
  JER2_ARAD_OFP_RATES_PORT_PRIORITY_SHAPER_VERIFY,
  JER2_ARAD_OFP_RATES_TCG_SHAPER_SET,
  JER2_ARAD_OFP_RATES_TCG_SHAPER_GET,
  JER2_ARAD_OFP_RATES_TCG_SHAPER_SET_UNSAFE,
  JER2_ARAD_OFP_RATES_TCG_SHAPER_GET_UNSAFE,
  JER2_ARAD_OFP_RATES_TCG_SHAPER_VERIFY,
  JER2_ARAD_OFP_RATES_RETRIEVE_EGRESS_SHAPER_REG_FIELD_NAME,
  JER2_ARAD_SCH_PORT_PRIORITY_SHAPER_RATE_SET_UNSAFE,
  JER2_ARAD_SCH_PORT_PRIORITY_SHAPER_RATE_GET_UNSAFE,
  JER2_ARAD_ITM_COMMITTED_Q_SIZE_SET,
  JER2_ARAD_ITM_COMMITTED_Q_SIZE_SET_PRINT,
  JER2_ARAD_ITM_COMMITTED_Q_SIZE_SET_UNSAFE,
  JER2_ARAD_ITM_COMMITTED_Q_SIZE_SET_VERIFY,
  JER2_ARAD_ITM_COMMITTED_Q_SIZE_GET_VERIFY,
  JER2_ARAD_ITM_COMMITTED_Q_SIZE_GET,
  JER2_ARAD_ITM_COMMITTED_Q_SIZE_GET_PRINT,
  JER2_ARAD_ITM_COMMITTED_Q_SIZE_GET_UNSAFE,
  JER2_ARAD_ITM_PFC_TC_MAP_SET,
  JER2_ARAD_ITM_PFC_TC_MAP_GET,
  JER2_ARAD_ITM_DISCARD_DP_SET_VERIFY,
  JER2_ARAD_ITM_DISCARD_DP_SET_UNSAFE,
  JER2_ARAD_ITM_DISCARD_DP_GET_UNSAFE,
  JER2_ARAD_IPQ_TRAFFIC_CLASS_PROFILE_MAP_SET_UNSAFE,
  JER2_ARAD_IPQ_TRAFFIC_CLASS_PROFILE_MAP_GET_UNSAFE,
  JER2_ARAD_ITM_DISCARD_DP_SET,
  JER2_ARAD_ITM_DISCARD_DP_GET,
  JER2_ARAD_PORT_CONTROL_PCS_SET,
  JER2_ARAD_PORT_CONTROL_PCS_SET_UNSAFE,                                        
  JER2_ARAD_PORT_CONTROL_PCS_SET_VERIFY,
  JER2_ARAD_PORT_CONTROL_PCS_GET,
  JER2_ARAD_PORT_CONTROL_PCS_GET_UNSAFE,                                        
  JER2_ARAD_PORT_CONTROL_PCS_GET_VERIFY,

    JER2_ARAD_PP_IHB_PMF_PASS_2_KEY_UPDATE_TBL_SET_UNSAFE,
    JER2_ARAD_PP_IHB_PMF_PASS_2_KEY_UPDATE_TBL_GET_UNSAFE,                                              
  JER2_ARAD_IRE_TDM_CONFIG_TBL_GET_UNSAFE,
  JER2_ARAD_IRE_TDM_CONFIG_TBL_SET_UNSAFE,
    JER2_ARAD_IDR_CONTEXT_MRU_TBL_SET_UNSAFE,
    JER2_ARAD_IDR_CONTEXT_MRU_TBL_GET_UNSAFE,
  JER2_ARAD_IRE_CPU_CTXT_MAP_TBL_GET_UNSAFE,
  JER2_ARAD_IRE_CPU_CTXT_MAP_TBL_SET_UNSAFE,
  JER2_ARAD_IRR_SNOOP_MIRROR_TABLE0_TBL_GET_UNSAFE,
  JER2_ARAD_IRR_SNOOP_MIRROR_TABLE0_TBL_SET_UNSAFE,
  JER2_ARAD_IRR_SNOOP_MIRROR_TABLE1_TBL_GET_UNSAFE,
  JER2_ARAD_IRR_SNOOP_MIRROR_TABLE1_TBL_SET_UNSAFE,
  JER2_ARAD_IRR_LAG_TO_LAG_RANGE_TBL_GET_UNSAFE,
  JER2_ARAD_IRR_LAG_TO_LAG_RANGE_TBL_SET_UNSAFE,
  JER2_ARAD_IRR_LAG_MAPPING_TBL_GET_UNSAFE,
  JER2_ARAD_IRR_LAG_MAPPING_TBL_SET_UNSAFE,
  JER2_ARAD_IRR_LAG_NEXT_MEMBER_TBL_GET_UNSAFE,
  JER2_ARAD_IRR_LAG_NEXT_MEMBER_TBL_SET_UNSAFE,
  JER2_ARAD_IRR_TRAFFIC_CLASS_MAPPING_TBL_GET_UNSAFE,
  JER2_ARAD_IRR_TRAFFIC_CLASS_MAPPING_TBL_SET_UNSAFE,
  JER2_ARAD_IHP_TM_PORT_SYS_PORT_CONFIG_TBL_GET_UNSAFE,
  JER2_ARAD_IHP_TM_PORT_SYS_PORT_CONFIG_TBL_SET_UNSAFE,
  JER2_ARAD_EGQ_TC_DP_MAP_TBL_GET_UNSAFE,
    JER2_ARAD_EGQ_TC_DP_MAP_TBL_SET_UNSAFE,
  JER2_ARAD_NIF_PORT_BLOCK_GET,
    JER2_ARAD_NIF_PORT_BLOCK_FROM_WC_GET,
    JER2_ARAD_NIF_MAC_CONF_SET_UNSAFE,
    JER2_ARAD_NIF_MAC_ECC_MEMORY_PROTECTION_SET_UNSAFE,
    JER2_ARAD_NIF_SRD_LANES_GET,
    JER2_ARAD_NIF_TYPE_MODE_VERIFY,
    JER2_ARAD_PORT_PARSER_PROGRAM_POINTER_GET_UNSAFE,
  JER2_ARAD_EGQ_FQP_NIF_PORT_MUX_TBL_SET_UNSAFE,
  JER2_ARAD_EGQ_PQP_NIF_PORT_MUX_TBL_SET_UNSAFE,
    JER2_ARAD_PP_IHB_PMF_PROGRAM_SELECTION_CAM_TBL_GET_UNSAFE,
    JER2_ARAD_PP_IHB_PMF_PROGRAM_SELECTION_CAM_TBL_SET_UNSAFE,

  JER2_ARAD_IHB_PINFO_LBP_TBL_GET_UNSAFE,
  JER2_ARAD_IHB_PINFO_LBP_TBL_SET_UNSAFE,
  JER2_ARAD_IHB_PINFO_PMF_TBL_GET_UNSAFE,
  JER2_ARAD_IHB_PINFO_PMF_TBL_SET_UNSAFE,
  JER2_ARAD_IHB_PTC_INFO_TBL_GET_UNSAFE,  
  JER2_ARAD_IHB_PTC_INFO_TBL_SET_UNSAFE, 

  JER2_ARAD_EPNI_LFEM0_FIELD_SELECT_MAP_TBL_GET_UNSAFE,
  JER2_ARAD_EPNI_LFEM0_FIELD_SELECT_MAP_TBL_SET_UNSAFE,
  JER2_ARAD_EPNI_LFEM1_FIELD_SELECT_MAP_TBL_GET_UNSAFE,
  JER2_ARAD_EPNI_LFEM1_FIELD_SELECT_MAP_TBL_SET_UNSAFE,
  JER2_ARAD_EPNI_LFEM2_FIELD_SELECT_MAP_TBL_GET_UNSAFE,
  JER2_ARAD_EPNI_LFEM2_FIELD_SELECT_MAP_TBL_SET_UNSAFE,
    JER2_ARAD_PP_EGQ_PMF_PROGRAM_SELECTION_CAM_TBL_SET_UNSAFE,
    JER2_ARAD_PP_EGQ_PMF_PROGRAM_SELECTION_CAM_TBL_GET_UNSAFE,
  JER2_ARAD_TDM_INIT,
  JER2_ARAD_TDM_FTMH_SET,
  JER2_ARAD_TDM_FTMH_SET_PRINT,
  JER2_ARAD_TDM_FTMH_SET_UNSAFE,
  JER2_ARAD_TDM_ING_FTMH_FILL_HEADER,
  JER2_ARAD_TDM_ING_FTMH_SET_UNSAFE,
  JER2_ARAD_TDM_EG_FTMH_SET_UNSAFE,
  JER2_ARAD_TDM_FTMH_SET_VERIFY,
  JER2_ARAD_TDM_FTMH_GET,
  JER2_ARAD_TDM_FTMH_GET_PRINT,
  JER2_ARAD_TDM_FTMH_GET_VERIFY,
  JER2_ARAD_TDM_FTMH_GET_UNSAFE,
  JER2_ARAD_TDM_ING_FTMH_GET_UNSAFE,
  JER2_ARAD_TDM_EG_FTMH_GET_UNSAFE,
  JER2_ARAD_TDM_OPT_SIZE_SET,
  JER2_ARAD_TDM_OPT_SIZE_SET_PRINT,
  JER2_ARAD_TDM_OPT_SIZE_SET_UNSAFE,
  JER2_ARAD_TDM_OPT_SIZE_SET_VERIFY,
  JER2_ARAD_TDM_OPT_SIZE_GET,
  JER2_ARAD_TDM_OPT_SIZE_GET_PRINT,
  JER2_ARAD_TDM_OPT_SIZE_GET_VERIFY,
  JER2_ARAD_TDM_OPT_SIZE_GET_UNSAFE,
  JER2_ARAD_TDM_STAND_SIZE_RANGE_SET,
  JER2_ARAD_TDM_STAND_SIZE_RANGE_SET_PRINT,
  JER2_ARAD_TDM_STAND_SIZE_RANGE_SET_UNSAFE,
  JER2_ARAD_TDM_STAND_SIZE_RANGE_SET_VERIFY,
  JER2_ARAD_TDM_STAND_SIZE_RANGE_GET,
  JER2_ARAD_TDM_STAND_SIZE_RANGE_GET_PRINT,
  JER2_ARAD_TDM_STAND_SIZE_RANGE_GET_VERIFY,
  JER2_ARAD_TDM_STAND_SIZE_RANGE_GET_UNSAFE,
  JER2_ARAD_TDM_OFP_SET_PRINT,
  JER2_ARAD_TDM_OFP_SET_UNSAFE,
  JER2_ARAD_TDM_OFP_SET_VERIFY,
  JER2_ARAD_TDM_OFP_GET_PRINT,
  JER2_ARAD_TDM_OFP_GET_VERIFY,
  JER2_ARAD_TDM_OFP_GET_UNSAFE,
  JER2_ARAD_TDM_IFP_SET_UNSAFE,
  JER2_ARAD_TDM_IFP_SET_VERIFY,
  JER2_ARAD_TDM_IFP_GET,
  JER2_ARAD_TDM_IFP_GET_VERIFY,
  JER2_ARAD_TDM_IFP_GET_UNSAFE,
  JER2_ARAD_CNT_COUNTERS_SET,
  JER2_ARAD_CNT_COUNTERS_SET_PRINT,
  JER2_ARAD_CNT_COUNTERS_SET_UNSAFE,
  JER2_ARAD_CNT_COUNTERS_SET_VERIFY,
  JER2_ARAD_CNT_COUNTERS_GET,
  JER2_ARAD_CNT_COUNTERS_GET_PRINT,
  JER2_ARAD_CNT_COUNTERS_GET_VERIFY,
  JER2_ARAD_CNT_COUNTERS_GET_UNSAFE,
  JER2_ARAD_CNT_STATUS_GET,
  JER2_ARAD_CNT_STATUS_GET_PRINT,
  JER2_ARAD_CNT_STATUS_GET_UNSAFE,
  JER2_ARAD_CNT_STATUS_GET_VERIFY,
  JER2_ARAD_CNT_ALGORITHMIC_READ,
  JER2_ARAD_CNT_ALGORITHMIC_READ_PRINT,
  JER2_ARAD_CNT_ALGORITHMIC_READ_UNSAFE,
  JER2_ARAD_CNT_ALGORITHMIC_READ_VERIFY,
  JER2_ARAD_CNT_DIRECT_READ,
  JER2_ARAD_CNT_DIRECT_READ_PRINT,
  JER2_ARAD_CNT_DIRECT_READ_UNSAFE,
  JER2_ARAD_CNT_DIRECT_READ_VERIFY,
  JER2_ARAD_CNT_Q2CNT_ID,
  JER2_ARAD_CNT_Q2CNT_ID_PRINT,
  JER2_ARAD_CNT_Q2CNT_ID_UNSAFE,
  JER2_ARAD_CNT_Q2CNT_ID_VERIFY,
  JER2_ARAD_CNT_CNT2Q_ID,
  JER2_ARAD_CNT_CNT2Q_ID_PRINT,
  JER2_ARAD_CNT_CNT2Q_ID_UNSAFE,
  JER2_ARAD_CNT_CNT2Q_ID_VERIFY,
  JER2_ARAD_CNT_METER_HDR_COMPENSATION_SET,
  JER2_ARAD_CNT_METER_HDR_COMPENSATION_SET_PRINT,
  JER2_ARAD_CNT_METER_HDR_COMPENSATION_SET_UNSAFE,
  JER2_ARAD_CNT_METER_HDR_COMPENSATION_SET_VERIFY,
  JER2_ARAD_CNT_METER_HDR_COMPENSATION_GET,
  JER2_ARAD_CNT_METER_HDR_COMPENSATION_GET_PRINT,
  JER2_ARAD_CNT_METER_HDR_COMPENSATION_GET_VERIFY,
  JER2_ARAD_CNT_METER_HDR_COMPENSATION_GET_UNSAFE,
  JER2_ARAD_ACTION_CMD_SNOOP_SET,
  JER2_ARAD_ACTION_CMD_SNOOP_SET_PRINT,
  JER2_ARAD_ACTION_CMD_SNOOP_SET_UNSAFE,
  JER2_ARAD_ACTION_CMD_SNOOP_SET_VERIFY,
  JER2_ARAD_ACTION_CMD_SNOOP_GET,
  JER2_ARAD_ACTION_CMD_SNOOP_GET_PRINT,
  JER2_ARAD_ACTION_CMD_SNOOP_GET_VERIFY,
  JER2_ARAD_ACTION_CMD_SNOOP_GET_UNSAFE,
  JER2_ARAD_ACTION_CMD_MIRROR_SET,
  JER2_ARAD_ACTION_CMD_MIRROR_SET_PRINT,
  JER2_ARAD_ACTION_CMD_MIRROR_SET_UNSAFE,
  JER2_ARAD_ACTION_CMD_MIRROR_SET_VERIFY,
  JER2_ARAD_ACTION_CMD_MIRROR_GET,
  JER2_ARAD_ACTION_CMD_MIRROR_GET_PRINT,
  JER2_ARAD_ACTION_CMD_MIRROR_GET_VERIFY,
  JER2_ARAD_ACTION_CMD_MIRROR_GET_UNSAFE,
  JER2_ARAD_CNT_DMA_SET,
  JER2_ARAD_CNT_DMA_SET_UNSAFE,
  JER2_ARAD_CNT_DMA_SET_VERIFY,
  JER2_ARAD_CNT_DMA_UNSET,
  JER2_ARAD_CNT_DMA_UNSET_UNSAFE,
  JER2_ARAD_CNT_DMA_UNSET_VERIFY,
  JER2_ARAD_MGMT_MAX_PCKT_SIZE_SET,
  JER2_ARAD_MGMT_MAX_PCKT_SIZE_SET_PRINT,
  JER2_ARAD_MGMT_MAX_PCKT_SIZE_SET_UNSAFE,
  JER2_ARAD_MGMT_MAX_PCKT_SIZE_SET_VERIFY,
  JER2_ARAD_MGMT_MAX_PCKT_SIZE_GET,
  JER2_ARAD_MGMT_MAX_PCKT_SIZE_GET_PRINT,
  JER2_ARAD_MGMT_MAX_PCKT_SIZE_GET_VERIFY,
  JER2_ARAD_MGMT_MAX_PCKT_SIZE_GET_UNSAFE,
  JER2_ARAD_MGMT_OCB_MC_RANGE_SET,
  JER2_ARAD_MGMT_OCB_MC_RANGE_SET_UNSAFE,
  JER2_ARAD_MGMT_OCB_MC_RANGE_GET,
  JER2_ARAD_MGMT_OCB_MC_RANGE_GET_UNSAFE,
  JER2_ARAD_MGMT_OCB_VOQ_ELIGIBLE_SET,
  JER2_ARAD_MGMT_OCB_VOQ_ELIGIBLE_SET_UNSAFE,
  JER2_ARAD_MGMT_OCB_VOQ_ELIGIBLE_GET,
  JER2_ARAD_MGMT_OCB_VOQ_ELIGIBLE_GET_UNSAFE,
  JER2_ARAD_NIF_WC_BASIC_CONF_SET,
  JER2_ARAD_NIF_WC_BASIC_CONF_SET_PRINT,
    JER2_ARAD_NIF_MAC_BASIC_CONF_SET_UNSAFE,
    JER2_ARAD_NIF_INIT_CONF_VERIFY,
  JER2_ARAD_NIF_WC_BASIC_CONF_SET_UNSAFE,
    JER2_ARAD_NIF_MAC_BASIC_CONF_VERIFY,
  JER2_ARAD_NIF_WC_BASIC_CONF_VERIFY,
    JER2_ARAD_NIF_MODE_GET,
    JER2_ARAD_NIF_MODE_GET_UNSAFE,
    JER2_ARAD_NIF_DEFAULT_RATE_GET,
  JER2_ARAD_NIF_WC_BASIC_CONF_GET,
  JER2_ARAD_NIF_WC_BASIC_CONF_GET_PRINT,
  JER2_ARAD_NIF_WC_ID_VERIFY,
  JER2_ARAD_NIF_IF2EGRESS_OFFSET,
  JER2_ARAD_NIF_WC_BASIC_CONF_GET_UNSAFE,
  JER2_ARAD_NIF_ILKN_SET,
  JER2_ARAD_NIF_ILKN_SET_PRINT,
  JER2_ARAD_NIF_ILKN_SET_UNSAFE,
  JER2_ARAD_NIF_ILKN_SET_VERIFY,
  JER2_ARAD_NIF_ILKN_GET,
  JER2_ARAD_NIF_ILKN_GET_PRINT,
  JER2_ARAD_NIF_ILKN_GET_VERIFY,
  JER2_ARAD_NIF_ILKN_GET_UNSAFE,
    JER2_ARAD_NIF_ID_TO_PORT_GET,
    JER2_ARAD_NIF_RATE_GET,
  JER2_ARAD_NIF_IPG_SET,
  JER2_ARAD_NIF_IPG_GET,
  JER2_ARAD_NIF_IPG_SET_UNSAFE,
  JER2_ARAD_NIF_IPG_GET_UNSAFE,
  JER2_ARAD_NIF_IPG_SET_VERIFY,
  JER2_ARAD_NIF_IPG_GET_VERIFY,
  JER2_ARAD_NIF_LINK_STATUS_GET_PRINT,
  JER2_ARAD_NIF_LINK_STATUS_GET_VERIFY,
  JER2_ARAD_FC_GEN_INBND_SET,
  JER2_ARAD_FC_GEN_INBND_SET_PRINT,
  JER2_ARAD_FC_GEN_INBND_SET_UNSAFE,
  JER2_ARAD_FC_GEN_INBND_SET_VERIFY,
  JER2_ARAD_FC_GEN_INBND_GET,
  JER2_ARAD_FC_GEN_INBND_GET_PRINT,
  JER2_ARAD_FC_GEN_INBND_GET_VERIFY,
  JER2_ARAD_FC_GEN_INBND_GET_UNSAFE,
  JER2_ARAD_FC_GEN_INBND_GLB_HP_SET,
  JER2_ARAD_FC_GEN_INBND_GLB_HP_SET_PRINT,
  JER2_ARAD_FC_GEN_INBND_GLB_HP_SET_UNSAFE,
  JER2_ARAD_FC_GEN_INBND_GLB_HP_SET_VERIFY,
  JER2_ARAD_FC_GEN_INBND_GLB_HP_GET,
  JER2_ARAD_FC_GEN_INBND_GLB_HP_GET_PRINT,
  JER2_ARAD_FC_GEN_INBND_GLB_HP_GET_VERIFY,
  JER2_ARAD_FC_GEN_INBND_GLB_HP_GET_UNSAFE,
  JER2_ARAD_FC_GEN_INBND_CNM_MAP_SET,
  JER2_ARAD_FC_GEN_INBND_CNM_MAP_SET_PRINT,
  JER2_ARAD_FC_GEN_INBND_CNM_MAP_SET_UNSAFE,
  JER2_ARAD_FC_GEN_INBND_CNM_MAP_SET_VERIFY,
  JER2_ARAD_FC_GEN_INBND_CNM_MAP_GET,
  JER2_ARAD_FC_GEN_INBND_CNM_MAP_GET_PRINT,
  JER2_ARAD_FC_GEN_INBND_CNM_MAP_GET_VERIFY,
  JER2_ARAD_FC_GEN_INBND_CNM_MAP_GET_UNSAFE,
  JER2_ARAD_FC_GEN_INBND_PFC_SET_UNSAFE,
  JER2_ARAD_FC_GEN_INBND_PFC_GET_UNSAFE,
  JER2_ARAD_FC_GEN_INBND_LL_SET_UNSAFE,
  JER2_ARAD_FC_GEN_INBND_LL_GET_UNSAFE,
  JER2_ARAD_FLOW_CONTROL_INIT_OOB_TX,
  JER2_ARAD_FLOW_CONTROL_INIT_OOB_RX,
  JER2_ARAD_FC_REC_INBND_SET,
  JER2_ARAD_FC_REC_INBND_SET_PRINT,
  JER2_ARAD_FC_REC_INBND_SET_UNSAFE,
  JER2_ARAD_FC_REC_INBND_SET_VERIFY,
  JER2_ARAD_FC_REC_INBND_GET,
  JER2_ARAD_FC_REC_INBND_GET_PRINT,
  JER2_ARAD_FC_REC_INBND_GET_VERIFY,
  JER2_ARAD_FC_REC_INBND_GET_UNSAFE,
  JER2_ARAD_FC_REC_INBND_OFP_MAP_SET,
  JER2_ARAD_FC_REC_INBND_OFP_MAP_SET_PRINT,
  JER2_ARAD_FC_REC_INBND_OFP_MAP_SET_UNSAFE,
  JER2_ARAD_FC_REC_INBND_OFP_MAP_SET_VERIFY,
  JER2_ARAD_FC_REC_INBND_OFP_MAP_GET,
  JER2_ARAD_FC_REC_INBND_OFP_MAP_GET_PRINT,
  JER2_ARAD_FC_REC_INBND_OFP_MAP_GET_VERIFY,
  JER2_ARAD_FC_REC_INBND_OFP_MAP_GET_UNSAFE,
  JER2_ARAD_FC_RCY_OFP_SET,
  JER2_ARAD_FC_RCY_OFP_SET_PRINT,
  JER2_ARAD_FC_RCY_OFP_SET_UNSAFE,
  JER2_ARAD_FC_RCY_OFP_SET_VERIFY,
  JER2_ARAD_FC_RCY_OFP_GET,
  JER2_ARAD_FC_RCY_OFP_GET_PRINT,
  JER2_ARAD_FC_RCY_OFP_GET_VERIFY,
  JER2_ARAD_FC_RCY_OFP_GET_UNSAFE,
  JER2_ARAD_FC_RCY_HR_ENABLE_SET_PRINT,
  JER2_ARAD_FC_RCY_HR_ENABLE_SET_VERIFY,
  JER2_ARAD_FC_RCY_HR_ENABLE_GET_PRINT,
  JER2_ARAD_FC_RCY_HR_ENABLE_GET_VERIFY,
  JER2_ARAD_FC_GEN_CAL_SET,
  JER2_ARAD_FC_GEN_CAL_SET_PRINT,
  JER2_ARAD_FC_GEN_CAL_SET_UNSAFE,
  JER2_ARAD_FC_GEN_CAL_SET_VERIFY,
  JER2_ARAD_FC_GEN_CAL_GET,
  JER2_ARAD_FC_GEN_CAL_GET_PRINT,
  JER2_ARAD_FC_GEN_CAL_GET_VERIFY,
  JER2_ARAD_FC_GEN_CAL_GET_UNSAFE,
  JER2_ARAD_FC_REC_CAL_SET,
  JER2_ARAD_FC_REC_CAL_SET_PRINT,
  JER2_ARAD_FC_REC_CAL_SET_UNSAFE,
  JER2_ARAD_FC_REC_CAL_SET_VERIFY,
  JER2_ARAD_FC_REC_CAL_GET,
  JER2_ARAD_FC_REC_CAL_GET_PRINT,
  JER2_ARAD_FC_REC_CAL_GET_VERIFY,
  JER2_ARAD_FC_REC_CAL_GET_UNSAFE,
  JER2_ARAD_FC_EGR_REC_OOB_STAT_GET_PRINT,
  JER2_ARAD_FC_EGR_REC_OOB_STAT_GET_VERIFY,
  JER2_ARAD_FC_ILKN_LLFC_SET,
  JER2_ARAD_FC_ILKN_LLFC_SET_PRINT,
  JER2_ARAD_FC_ILKN_LLFC_SET_UNSAFE,
  JER2_ARAD_FC_ILKN_LLFC_SET_VERIFY,
  JER2_ARAD_FC_ILKN_LLFC_GET,
  JER2_ARAD_FC_ILKN_LLFC_GET_PRINT,
  JER2_ARAD_FC_ILKN_LLFC_GET_VERIFY,
  JER2_ARAD_FC_ILKN_LLFC_GET_UNSAFE,
  JER2_ARAD_FC_ILKN_MUB_REC_SET_UNSAFE,
  JER2_ARAD_FC_ILKN_MUB_REC_GET_UNSAFE,
  JER2_ARAD_FC_ILKN_MUB_GEN_CAL_SET_UNSAFE,
  JER2_ARAD_FC_ILKN_MUB_GEN_CAL_GET_UNSAFE,
  JER2_ARAD_FC_ILKN_RETRANSMIT_SET_UNSAFE,
  JER2_ARAD_FC_ILKN_RETRANSMIT_GET_UNSAFE,
  JER2_ARAD_FC_PFC_GENERIC_BITMAP_SET,
  JER2_ARAD_FC_PFC_GENERIC_BITMAP_GET,
  JER2_ARAD_FC_PFC_GENERIC_BITMAP_VERIFY,
  JER2_ARAD_FC_PFC_GENERIC_BITMAP_GET_UNSAFE,
  JER2_ARAD_FC_PFC_GENERIC_BITMAP_SET_UNSAFE,
  JER2_ARAD_FC_REC_SCH_OOB_SET_UNSAFE,
  JER2_ARAD_FC_REC_SCH_OOB_SET_VERIFY,
  JER2_ARAD_FC_NIF_OVERSUBSCR_SCHEME_SET,
  JER2_ARAD_FC_NIF_OVERSUBSCR_SCHEME_SET_PRINT,
  JER2_ARAD_FC_NIF_OVERSUBSCR_SCHEME_SET_UNSAFE,
  JER2_ARAD_FC_NIF_OVERSUBSCR_SCHEME_SET_VERIFY,
  JER2_ARAD_FC_NIF_OVERSUBSCR_SCHEME_GET,
  JER2_ARAD_FC_NIF_OVERSUBSCR_SCHEME_GET_PRINT,
  JER2_ARAD_FC_NIF_OVERSUBSCR_SCHEME_GET_VERIFY,
  JER2_ARAD_FC_NIF_OVERSUBSCR_SCHEME_GET_UNSAFE,
  JER2_ARAD_FC_NIF_PAUSE_QUANTA_SET,
  JER2_ARAD_FC_NIF_PAUSE_QUANTA_SET_PRINT,
  JER2_ARAD_FC_NIF_PAUSE_QUANTA_SET_UNSAFE,
  JER2_ARAD_FC_NIF_PAUSE_QUANTA_SET_VERIFY,
  JER2_ARAD_FC_NIF_PAUSE_QUANTA_GET,
  JER2_ARAD_FC_NIF_PAUSE_QUANTA_GET_PRINT,
  JER2_ARAD_FC_NIF_PAUSE_QUANTA_GET_VERIFY,
  JER2_ARAD_FC_NIF_PAUSE_QUANTA_GET_UNSAFE,
  JER2_ARAD_FC_NIF_PAUSE_FRAME_SRC_ADDR_SET,
  JER2_ARAD_FC_NIF_PAUSE_FRAME_SRC_ADDR_SET_PRINT,
  JER2_ARAD_FC_NIF_PAUSE_FRAME_SRC_ADDR_SET_UNSAFE,
  JER2_ARAD_FC_NIF_PAUSE_FRAME_SRC_ADDR_SET_VERIFY,
  JER2_ARAD_FC_NIF_PAUSE_FRAME_SRC_ADDR_GET,
  JER2_ARAD_FC_NIF_PAUSE_FRAME_SRC_ADDR_GET_PRINT,
  JER2_ARAD_FC_NIF_PAUSE_FRAME_SRC_ADDR_GET_VERIFY,
  JER2_ARAD_FC_NIF_PAUSE_FRAME_SRC_ADDR_GET_UNSAFE,
  JER2_ARAD_FC_EXTEND_Q_GRP_SET,
  JER2_ARAD_FC_EXTEND_Q_GRP_SET_PRINT,
  JER2_ARAD_FC_EXTEND_Q_GRP_SET_UNSAFE,
  JER2_ARAD_FC_EXTEND_Q_GRP_SET_VERIFY,
  JER2_ARAD_FC_EXTEND_Q_GRP_GET,
  JER2_ARAD_FC_EXTEND_Q_GRP_GET_PRINT,
  JER2_ARAD_FC_EXTEND_Q_GRP_GET_VERIFY,
  JER2_ARAD_FC_EXTEND_Q_GRP_GET_UNSAFE,
  JER2_ARAD_FC_CLEAR_CALENDAR_UNSAFE,
  JER2_ARAD_FC_HCFC_BITMAP_SET_UNSAFE,
  JER2_ARAD_FC_HCFC_BITMAP_GET_UNSAFE,
  JER2_ARAD_FC_INIT_PFC_MAPPING_UNSAFE,
  JER2_ARAD_FC_INIT_PFC_MAPPING,
  JER2_ARAD_FC_ILKN_RETRANSMIT_CAL_SET_VERIFY,
  JER2_ARAD_FC_ILKN_RETRANSMIT_CAL_SET_UNSAFE,
  JER2_ARAD_FC_ILKN_RETRANSMIT_CAL_GET_UNSAFE,
  JER2_ARAD_FC_ILKN_RETRANSMIT_CAL_SET,
  JER2_ARAD_FC_ILKN_RETRANSMIT_CAL_GET,
  JER2_ARAD_CNM_CP_PACKET_SET,
  JER2_ARAD_CNM_CP_PACKET_GET,
  JER2_ARAD_CNM_CP_PACKET_SET_UNSAFE,
  JER2_ARAD_CNM_CP_PACKET_GET_UNSAFE,
  JER2_ARAD_CNM_CP_PROFILE_GET,
  JER2_ARAD_CNM_CP_PROFILE_SET,
  JER2_ARAD_CNM_CP_PROFILE_GET_UNSAFE,
  JER2_ARAD_CNM_CP_PROFILE_SET_UNSAFE,
  JER2_ARAD_CNM_CP_PROFILE_VERIFY,
  JER2_ARAD_CNM_CPQ_GET,
  JER2_ARAD_CNM_CPQ_SET,
  JER2_ARAD_CNM_CPQ_GET_UNSAFE,
  JER2_ARAD_CNM_CPQ_SET_UNSAFE,
  JER2_ARAD_CNM_CPQ_VERIFY,
  JER2_ARAD_CNM_QUEUE_MAPPING_GET,
  JER2_ARAD_CNM_QUEUE_MAPPING_SET,
  JER2_ARAD_CNM_QUEUE_MAPPING_GET_UNSAFE,
  JER2_ARAD_CNM_QUEUE_MAPPING_SET_UNSAFE,
  JER2_ARAD_CNM_QUEUE_MAPPING_SET_VERIFY,
  JER2_ARAD_CNM_CP_SET,
  JER2_ARAD_CNM_CP_GET,
  JER2_ARAD_CNM_CP_SET_VERIFY,
  JER2_ARAD_CNM_CP_SET_UNSAFE,
  JER2_ARAD_CNM_CP_GET_UNSAFE,  
  JER2_ARAD_IQM_CNM_PROFILE_TBL_SET_UNSAFE,
  JER2_ARAD_IQM_CNM_PROFILE_TBL_GET_UNSAFE,
  JER2_ARAD_IQM_CNM_DS_TBL_SET_UNSAFE,
  JER2_ARAD_IQM_CNM_DS_TBL_GET_UNSAFE,
  JER2_ARAD_NIF_COUNTER_GET_VERIFY,
  JER2_ARAD_NIF_ALL_COUNTERS_GET_VERIFY,
  JER2_ARAD_VERIFY_MULT_IN_INGRESS_AND_NOT_SINGLE_COPY,
  JER2_ARAD_MULT_PROPERTIES_SET_UNSAFE,
  JER2_ARAD_MULT_PROPERTIES_GET_UNSAFE,
    JER2_ARAD_EGR_PROG_EDITOR_UNSAFE,
  JER2_ARAD_EGR_Q_PRIO_SET,
  JER2_ARAD_EGR_Q_PRIO_VERIFY,
  JER2_ARAD_EGR_OFP_SCHEDULING_WFQ_SET_UNSAFE,
  JER2_ARAD_EGR_OFP_SCHEDULING_WFQ_GET_UNSAFE,
  JER2_ARAD_EGR_Q_PRIO_SET_UNSAFE,
  JER2_ARAD_EGR_Q_PRIO_SET_VERIFY,
  JER2_ARAD_EGR_Q_PRIO_GET,
  JER2_ARAD_EGR_Q_PRIO_GET_VERIFY,
  JER2_ARAD_EGR_Q_PRIO_GET_UNSAFE,
  JER2_ARAD_EGR_Q_PRIO_MAP_ENTRY_GET,
  JER2_ARAD_EGR_Q_PROFILE_MAP_SET,
  JER2_ARAD_EGR_Q_PROFILE_MAP_SET_PRINT,
  JER2_ARAD_EGR_Q_PROFILE_MAP_SET_UNSAFE,
  JER2_ARAD_EGR_Q_PROFILE_MAP_SET_VERIFY,
  JER2_ARAD_EGR_Q_PROFILE_MAP_GET,
  JER2_ARAD_EGR_Q_PROFILE_MAP_GET_PRINT,
  JER2_ARAD_EGR_Q_PROFILE_MAP_GET_VERIFY,
  JER2_ARAD_EGR_Q_PROFILE_MAP_GET_UNSAFE,
  JER2_ARAD_EGR_OTM_PORT_CUD_EXTENSION_ID_VERIFY,
  JER2_ARAD_EGR_DSP_PP_TO_BASE_Q_PAIR_GET_UNSAFE,
  JER2_ARAD_EGR_DSP_PP_TO_BASE_Q_PAIR_SET_VERIFY,
  JER2_ARAD_EGR_DSP_PP_TO_BASE_Q_PAIR_GET_VERIFY,
  JER2_ARAD_EGR_DSP_PP_TO_BASE_Q_PAIR_SET_UNSAFE,
  JER2_ARAD_EGR_DSP_PP_TO_BASE_Q_PAIR_SET,
  JER2_ARAD_EGR_DSP_PP_TO_BASE_Q_PAIR_GET,
  JER2_ARAD_EGR_DSP_PP_PRIORITIES_MODE_SET,
  JER2_ARAD_EGR_DSP_PP_PRIORITIES_MODE_GET,
  JER2_ARAD_EGR_DSP_PP_PRIORITIES_MODE_SET_UNSAFE,
  JER2_ARAD_EGR_DSP_PP_PRIORITIES_MODE_SET_VERIFY,
  JER2_ARAD_EGR_DSP_PP_PRIORITIES_MODE_GET_UNSAFE,
  JER2_ARAD_EGR_DSP_PP_PRIORITIES_MODE_GET_VERIFY,  
  JER2_ARAD_EGR_DSP_PP_SHAPER_MODE_GET_UNSAFE,
  JER2_ARAD_EGR_DSP_PP_SHAPER_MODE_GET_VERIFY,
  JER2_ARAD_EGR_DSP_PP_SHAPER_MODE_SET_UNSAFE,
  JER2_ARAD_EGR_DSP_PP_SHAPER_MODE_SET_VERIFY,
  JER2_ARAD_EGR_QUEUING_PARTITION_SCHEME_SET,
  JER2_ARAD_EGR_QUEUING_PARTITION_SCHEME_SET_UNSAFE,
  JER2_ARAD_EGR_QUEUING_DEV_SET,
  JER2_ARAD_EGR_QUEUING_DEV_GET,
  JER2_ARAD_EGR_QUEUING_GLOBAL_DROP_SET,
  JER2_ARAD_EGR_QUEUING_GLOBAL_DROP_SET_UNSAFE,
  JER2_ARAD_EGR_QUEUING_GLOBAL_DROP_GET,
  JER2_ARAD_EGR_QUEUING_GLOBAL_DROP_GET_UNSAFE,
  JER2_ARAD_EGR_QUEUING_SP_TC_DROP_SET,
  JER2_ARAD_EGR_QUEUING_SP_TC_DROP_SET_UNSAFE,
  JER2_ARAD_EGR_QUEUING_SP_TC_DROP_GET,
  JER2_ARAD_EGR_QUEUING_SP_TC_DROP_GET_UNSAFE,
  JER2_ARAD_EGR_QUEUING_SP_RESERVED_SET,
  JER2_ARAD_EGR_QUEUING_SP_RESERVED_SET_UNSAFE,
  JER2_ARAD_EGR_QUEUING_SCH_UNSCH_DROP_SET,
  JER2_ARAD_EGR_QUEUING_SCH_UNSCH_DROP_SET_UNSAFE,
  JER2_ARAD_EGR_QUEUING_SCHED_PORT_FC_THRESH_SET,
  JER2_ARAD_EGR_QUEUING_SCHED_PORT_FC_THRESH_SET_UNSAFE,
  JER2_ARAD_EGR_QUEUING_SCHED_Q_FC_THRESH_SET,
  JER2_ARAD_EGR_QUEUING_GLOBAL_FC_SET,
  JER2_ARAD_EGR_QUEUING_GLOBAL_FC_SET_UNSAFE,
  JER2_ARAD_EGR_QUEUING_GLOBAL_FC_GET,
  JER2_ARAD_EGR_QUEUING_GLOBAL_FC_GET_UNSAFE,
  JER2_ARAD_EGR_QUEUING_MC_TC_FC_SET,
  JER2_ARAD_EGR_QUEUING_MC_TC_FC_SET_UNSAFE,
  JER2_ARAD_EGR_QUEUING_MC_TC_FC_GET,
  JER2_ARAD_EGR_QUEUING_MC_TC_FC_GET_UNSAFE,
  JER2_ARAD_EGR_QUEUING_MC_DROP_SET,
  JER2_ARAD_EGR_QUEUING_DEV_SET_UNSAFE,
  JER2_ARAD_EGR_QUEUING_DEV_GET_UNSAFE,
  JER2_ARAD_EGR_QUEUING_MC_COS_MAP_SET,
  JER2_ARAD_EGR_QUEUING_MC_COS_MAP_GET,
  JER2_ARAD_EGR_QUEUING_MC_COS_MAP_SET_UNSAFE,
  JER2_ARAD_EGR_QUEUING_MC_COS_MAP_GET_UNSAFE,
  JER2_ARAD_EGR_QUEUING_IF_FC_SET,
  JER2_ARAD_EGR_QUEUING_IF_FC_GET,
  JER2_ARAD_EGR_QUEUING_IF_FC_SET_UNSAFE,
  JER2_ARAD_EGR_QUEUING_IF_FC_GET_UNSAFE,
  JER2_ARAD_EGR_QUEUING_IF_FC_UC_SET,
  JER2_ARAD_EGR_QUEUING_IF_FC_UC_GET,
  JER2_ARAD_EGR_QUEUING_IF_FC_UC_SET_UNSAFE,
  JER2_ARAD_EGR_QUEUING_IF_FC_UC_GET_UNSAFE,
  JER2_ARAD_EGR_QUEUING_IF_FC_MC_SET,
  JER2_ARAD_EGR_QUEUING_IF_FC_MC_GET,
  JER2_ARAD_EGR_QUEUING_IF_FC_MC_SET_UNSAFE,
  JER2_ARAD_EGR_QUEUING_IF_FC_MC_GET_UNSAFE,
  JER2_ARAD_EGR_QUEUING_IF_UC_MAP_SET,
  JER2_ARAD_EGR_QUEUING_IF_UC_MAP_SET_UNSAFE,
  JER2_ARAD_EGR_QUEUING_IF_MC_MAP_SET,
  JER2_ARAD_EGR_QUEUING_IF_MC_MAP_SET_UNSAFE,
  JER2_ARAD_EGR_QUEUING_OFP_TCG_SET,
  JER2_ARAD_EGR_QUEUING_OFP_TCG_GET,
  JER2_ARAD_EGR_QUEUING_OFP_TCG_SET_UNSAFE,
  JER2_ARAD_EGR_QUEUING_OFP_TCG_GET_UNSAFE,
  JER2_ARAD_EGR_QUEUING_OFP_TCG_SET_VERIFY,
  JER2_ARAD_EGR_QUEUING_OFP_TCG_GET_VERIFY,
  JER2_ARAD_EGR_QUEUING_TCG_WEIGHT_SET,
  JER2_ARAD_EGR_QUEUING_TCG_WEIGHT_GET,
  JER2_ARAD_EGR_QUEUING_TCG_WEIGHT_SET_UNSAFE,
  JER2_ARAD_EGR_QUEUING_TCG_WEIGHT_GET_UNSAFE,
  JER2_ARAD_EGR_QUEUING_TCG_WEIGHT_SET_VERIFY_UNSAFE,
  JER2_ARAD_EGR_QUEUING_TCG_WEIGHT_GET_VERIFY_UNSAFE,
    JER2_ARAD_EGR_QUEUING_Q_PAIR_PORT_TC_FIND,
  JER2_ARAD_MGMT_HW_ADJUST_NIF,
  JER2_ARAD_NIF_WC_XAUI_SET,
  JER2_ARAD_NIF_ID_VERIFY,
  JER2_ARAD_NIF_TYPE_VERIFY,
  JER2_ARAD_NIF_WC_RXAUI_SET,
  JER2_ARAD_NIF_WC_TYPE_GET,
  JER2_ARAD_MGMT_IHB_TBLS_INIT,
  JER2_ARAD_MGMT_EPNI_TBLS_INIT,
  JER2_ARAD_DIAG_SAMPLE_ENABLE_SET,
  JER2_ARAD_DIAG_SAMPLE_ENABLE_GET,
  JER2_ARAD_DIAG_SAMPLE_ENABLE_SET_UNSAFE,
  JER2_ARAD_DIAG_SAMPLE_ENABLE_GET_UNSAFE,
  JER2_ARAD_DIAG_SIGNALS_DUMP,
  JER2_ARAD_DIAG_DBG_VAL_GET_UNSAFE,
  JER2_ARAD_DIAG_SIGNALS_DUMP_UNSAFE,
  JER2_ARAD_PMF_LOW_LEVEL_INIT_UNSAFE,
  JER2_ARAD_PMF_LOW_LEVEL_TM_INIT_SET_UNSAFE,
  JER2_ARAD_PMF_LOW_LEVEL_STACK_INIT_SET_UNSAFE,
  JER2_ARAD_PMF_LOW_LEVEL_STACK_PGM_SET_UNSAFE,
  JER2_ARAD_PMF_LOW_LEVEL_RAW_PGM_SET_UNSAFE,
  JER2_ARAD_PMF_LOW_LEVEL_ETH_PGM_SET_UNSAFE,
  JER2_ARAD_PMF_LOW_LEVEL_TM_PGM_SET_UNSAFE,
  JER2_ARAD_PMF_CE_ENTRY_VERIFY,
  JER2_ARAD_PMF_CE_INSTRUCTION_FLD_GET,
  JER2_ARAD_PMF_CE_PACKET_HEADER_ENTRY_SET,
  JER2_ARAD_PMF_CE_PACKET_HEADER_ENTRY_SET_PRINT,
  JER2_ARAD_PMF_CE_PACKET_HEADER_ENTRY_SET_UNSAFE,
    JER2_ARAD_PMF_CE_INTERNAL_FIELD_INFO_FIND,
    JER2_ARAD_PMF_CE_INTERNAL_FIELD_TABLE_SIZE_FIND,
    JER2_ARAD_PMF_CE_INTERNAL_FIELD_TABLE_LINE_FIND,
    JER2_ARAD_PMF_CE_HEADER_INFO_FIND,
    JER2_ARAD_PMF_CE_INTERNAL_FIELD_OFFSET_COMPUTE,
    JER2_ARAD_PMF_CE_INTERNAL_FIELD_OFFSET_QUAL_FIND,
  JER2_ARAD_PMF_CE_PACKET_HEADER_ENTRY_GET,
  JER2_ARAD_PMF_CE_PACKET_HEADER_ENTRY_GET_PRINT,
  JER2_ARAD_PMF_CE_PACKET_HEADER_ENTRY_GET_UNSAFE,
  JER2_ARAD_PMF_CE_IRPP_INFO_ENTRY_SET,
  JER2_ARAD_PMF_CE_IRPP_INFO_ENTRY_SET_PRINT,
  JER2_ARAD_PMF_CE_IRPP_INFO_ENTRY_SET_UNSAFE,
  JER2_ARAD_PMF_CE_IRPP_INFO_ENTRY_GET,
  JER2_ARAD_PMF_CE_IRPP_INFO_ENTRY_GET_PRINT,
  JER2_ARAD_PMF_CE_IRPP_INFO_ENTRY_GET_UNSAFE,
  JER2_ARAD_PMF_CE_NOP_ENTRY_SET,
  JER2_ARAD_PMF_CE_NOP_ENTRY_SET_PRINT,
  JER2_ARAD_PMF_CE_NOP_ENTRY_SET_UNSAFE,
  JER2_ARAD_PMF_CE_NOP_ENTRY_GET,
  JER2_ARAD_PMF_CE_NOP_ENTRY_GET_PRINT,
  JER2_ARAD_PMF_CE_NOP_ENTRY_GET_UNSAFE,
  JER2_ARAD_PMF_TCAM_LOOKUP_SET,
  JER2_ARAD_PMF_TCAM_LOOKUP_SET_PRINT,
  JER2_ARAD_PMF_TCAM_LOOKUP_SET_UNSAFE,
  JER2_ARAD_PMF_TCAM_LOOKUP_SET_VERIFY,
  JER2_ARAD_PMF_TCAM_LOOKUP_GET,
  JER2_ARAD_PMF_TCAM_LOOKUP_GET_PRINT,
  JER2_ARAD_PMF_TCAM_LOOKUP_GET_VERIFY,
  JER2_ARAD_PMF_TCAM_LOOKUP_GET_UNSAFE,
  JER2_ARAD_PMF_TCAM_ENTRY_ADD,
  JER2_ARAD_PMF_TCAM_ENTRY_ADD_PRINT,
  JER2_ARAD_PMF_TCAM_ENTRY_ADD_UNSAFE,
  JER2_ARAD_PMF_TCAM_ENTRY_ADD_VERIFY,
  JER2_ARAD_PMF_TCAM_ENTRY_LOCATION_GET,
  JER2_ARAD_PMF_TCAM_ENTRY_GET,
  JER2_ARAD_PMF_TCAM_ENTRY_GET_PRINT,
  JER2_ARAD_PMF_TCAM_ENTRY_GET_UNSAFE,
  JER2_ARAD_PMF_TCAM_ENTRY_GET_VERIFY,
  JER2_ARAD_PMF_TCAM_ENTRY_REMOVE,
  JER2_ARAD_PMF_TCAM_ENTRY_REMOVE_PRINT,
  JER2_ARAD_PMF_TCAM_ENTRY_REMOVE_UNSAFE,
  JER2_ARAD_PMF_TCAM_ENTRY_REMOVE_VERIFY,
  JER2_ARAD_PMF_TCAM_KEY_CLEAR,
  JER2_ARAD_PMF_TCAM_KEY_CLEAR_PRINT,
  JER2_ARAD_TCAM_KEY_CLEAR_UNSAFE,
  JER2_ARAD_TCAM_KEY_CLEAR_VERIFY,
  JER2_ARAD_PMF_TCAM_KEY_VAL_SET,
  JER2_ARAD_PMF_TCAM_KEY_VAL_SET_PRINT,
  JER2_ARAD_TCAM_KEY_VAL_SET_UNSAFE,
  JER2_ARAD_TCAM_KEY_VAL_SET_VERIFY,
  JER2_ARAD_PMF_TCAM_KEY_VAL_GET,
  JER2_ARAD_PMF_TCAM_KEY_VAL_GET_PRINT,
  JER2_ARAD_TCAM_KEY_VAL_GET_VERIFY,
  JER2_ARAD_TCAM_KEY_VAL_GET_UNSAFE,
  JER2_ARAD_TCAM_KEY_MASKED_VAL_SET,
  JER2_ARAD_TCAM_KEY_MASKED_VAL_SET_PRINT,
  JER2_ARAD_TCAM_KEY_MASKED_VAL_SET_UNSAFE,
  JER2_ARAD_TCAM_KEY_MASKED_VAL_SET_VERIFY,
  JER2_ARAD_TCAM_KEY_MASKED_VAL_GET,
  JER2_ARAD_TCAM_KEY_MASKED_VAL_GET_PRINT,
  JER2_ARAD_TCAM_KEY_MASKED_VAL_GET_VERIFY,
  JER2_ARAD_TCAM_KEY_MASKED_VAL_GET_UNSAFE,
  JER2_ARAD_PMF_DB_DIRECT_TBL_KEY_SRC_SET,
  JER2_ARAD_PMF_DB_DIRECT_TBL_KEY_SRC_SET_PRINT,
  JER2_ARAD_PMF_DB_DIRECT_TBL_KEY_SRC_SET_UNSAFE,
  JER2_ARAD_PMF_DB_DIRECT_TBL_KEY_SRC_SET_VERIFY,
  JER2_ARAD_PMF_DB_DIRECT_TBL_KEY_SRC_GET,
  JER2_ARAD_PMF_DB_DIRECT_TBL_KEY_SRC_GET_PRINT,
  JER2_ARAD_PMF_DB_DIRECT_TBL_KEY_SRC_GET_VERIFY,
  JER2_ARAD_PMF_DB_DIRECT_TBL_KEY_SRC_GET_UNSAFE,
  JER2_ARAD_PMF_DB_DIRECT_TBL_ENTRY_SET,
  JER2_ARAD_PMF_DB_DIRECT_TBL_ENTRY_SET_PRINT,
  JER2_ARAD_PMF_DB_DIRECT_TBL_ENTRY_SET_UNSAFE,
  JER2_ARAD_PMF_DB_DIRECT_TBL_ENTRY_SET_VERIFY,
  JER2_ARAD_PMF_DB_DIRECT_TBL_ENTRY_GET,
  JER2_ARAD_PMF_DB_DIRECT_TBL_ENTRY_GET_PRINT,
  JER2_ARAD_PMF_DB_DIRECT_TBL_ENTRY_GET_VERIFY,
  JER2_ARAD_PMF_DB_DIRECT_TBL_ENTRY_GET_UNSAFE,
  JER2_ARAD_PMF_DB_FEM_INPUT_SET,
	JER2_ARAD_PMF_DB_FES_SET_UNSAFE,
	JER2_ARAD_PMF_DB_FES_SET_VERIFY,
	JER2_ARAD_PMF_DB_FES_GET_UNSAFE,
	JER2_ARAD_PMF_DB_FES_GET_VERIFY,
    JER2_ARAD_PMF_DB_FES_MOVE_UNSAFE,
  JER2_ARAD_PMF_DB_FEM_INPUT_SET_PRINT,
  JER2_ARAD_PMF_DB_FEM_INPUT_SET_UNSAFE,
  JER2_ARAD_PMF_DB_FEM_INPUT_SET_VERIFY,
  JER2_ARAD_PMF_DB_FEM_INPUT_GET,
  JER2_ARAD_PMF_DB_FEM_INPUT_GET_PRINT,
  JER2_ARAD_PMF_DB_FEM_INPUT_GET_VERIFY,
  JER2_ARAD_PMF_DB_FEM_INPUT_GET_UNSAFE,
  JER2_ARAD_PMF_DB_TAG_SELECT_SET,
  JER2_ARAD_PMF_DB_TAG_SELECT_SET_PRINT,
  JER2_ARAD_PMF_DB_TAG_SELECT_SET_UNSAFE,
  JER2_ARAD_PMF_DB_TAG_SELECT_SET_VERIFY,
  JER2_ARAD_PMF_DB_TAG_SELECT_GET,
  JER2_ARAD_PMF_DB_TAG_SELECT_GET_PRINT,
  JER2_ARAD_PMF_DB_TAG_SELECT_GET_VERIFY,
  JER2_ARAD_PMF_DB_TAG_SELECT_GET_UNSAFE,
  JER2_ARAD_PMF_FEM_SELECT_BITS_SET,
  JER2_ARAD_PMF_FEM_SELECT_BITS_SET_PRINT,
  JER2_ARAD_PMF_FEM_SELECT_BITS_SET_UNSAFE,
  JER2_ARAD_PMF_FEM_SELECT_BITS_SET_VERIFY,
  JER2_ARAD_PMF_FEM_SELECT_BITS_GET,
  JER2_ARAD_PMF_FEM_SELECT_BITS_GET_PRINT,
  JER2_ARAD_PMF_FEM_SELECT_BITS_GET_VERIFY,
  JER2_ARAD_PMF_FEM_SELECT_BITS_GET_UNSAFE,
  JER2_ARAD_PMF_FEM_ACTION_FORMAT_MAP_SET,
  JER2_ARAD_PMF_FEM_ACTION_FORMAT_MAP_SET_PRINT,
  JER2_ARAD_PMF_FEM_ACTION_FORMAT_MAP_SET_UNSAFE,
  JER2_ARAD_PMF_FEM_ACTION_FORMAT_MAP_SET_VERIFY,
  JER2_ARAD_PMF_FEM_ACTION_FORMAT_MAP_GET,
  JER2_ARAD_PMF_FEM_ACTION_FORMAT_MAP_GET_PRINT,
  JER2_ARAD_PMF_FEM_ACTION_FORMAT_MAP_GET_VERIFY,
  JER2_ARAD_PMF_FEM_ACTION_FORMAT_MAP_GET_UNSAFE,
  JER2_ARAD_PMF_FEM_ACTION_FORMAT_SET,
  JER2_ARAD_PMF_FEM_ACTION_FORMAT_SET_PRINT,
  JER2_ARAD_PMF_FEM_ACTION_FORMAT_SET_UNSAFE,
  JER2_ARAD_PMF_FEM_ACTION_FORMAT_SET_VERIFY,
  JER2_ARAD_PMF_FEM_ACTION_FORMAT_GET,
  JER2_ARAD_PMF_FEM_ACTION_FORMAT_GET_PRINT,
  JER2_ARAD_PMF_FEM_ACTION_FORMAT_GET_VERIFY,
  JER2_ARAD_PMF_FEM_ACTION_FORMAT_GET_UNSAFE,
  JER2_ARAD_PMF_PGM_SELECTION_ENTRY_SET,
  JER2_ARAD_PMF_PGM_SELECTION_ENTRY_SET_PRINT,
  JER2_ARAD_PMF_PGM_SELECTION_ENTRY_SET_UNSAFE,
  JER2_ARAD_PMF_PGM_SELECTION_ENTRY_SET_VERIFY,
  JER2_ARAD_PMF_PGM_SELECTION_ENTRY_GET,
  JER2_ARAD_PMF_PGM_SELECTION_ENTRY_GET_PRINT,
  JER2_ARAD_PMF_PGM_SELECTION_ENTRY_GET_VERIFY,
  JER2_ARAD_PMF_PGM_SELECTION_ENTRY_GET_UNSAFE,
  JER2_ARAD_PMF_PGM_SET,
  JER2_ARAD_PMF_PGM_SET_PRINT,
  JER2_ARAD_PMF_PGM_SET_UNSAFE,
  JER2_ARAD_PMF_PGM_SET_VERIFY,
  JER2_ARAD_PMF_PGM_GET,
  JER2_ARAD_PMF_PGM_GET_PRINT,
  JER2_ARAD_PMF_PGM_GET_VERIFY,
  JER2_ARAD_PMF_PGM_GET_UNSAFE,
  JER2_ARAD_PMF_LOW_LEVEL_GET_PROCS_PTR,
  JER2_ARAD_PMF_LOW_LEVEL_GET_ERRS_PTR,
  JER2_ARAD_PMF_FEM_OUTPUT_SIZE_GET,
  JER2_ARAD_PMF_PGM_SELECTION_ENTRY_BUILD,
  JER2_ARAD_PMF_LOW_LEVEL_PGM_SEL_TBL_NOF_ENTRIES_GET,
  JER2_ARAD_PMF_LOW_LEVEL_PGM_HEADER_PROFILE_TO_HW_ADD,
  JER2_ARAD_PMF_LOW_LEVEL_PGM_HEADER_PROFILE_SET,
  JER2_ARAD_PMF_LOW_LEVEL_PGM_HEADER_PROFILE_GET,
  JER2_ARAD_PMF_LOW_LEVEL_PGM_PORT_PROFILE_TO_HW_ADD,
  JER2_ARAD_PMF_PGM_MGMT_PROFILE_GET,
  JER2_ARAD_PMF_LOW_LEVEL_PGM_PROFILE_TO_HW_ADD,
  JER2_ARAD_PMF_LOW_LEVEL_PARSER_PMF_PROFILE_GET,
  JER2_ARAD_PMF_LOW_LEVEL_PGM_PORT_PROFILE_ENCODE,
  JER2_ARAD_PMF_LOW_LEVEL_PGM_PORT_PROFILE_DECODE,
  JER2_ARAD_PMF_LOW_LEVEL_PGM_PORT_PROFILE_GET,
  JER2_ARAD_PMF_LOW_LEVEL_PGM_HEADER_TYPE_GET,
  JER2_ARAD_PMF_LOW_LEVEL_PGM_COUNTER_INIT_UNSAFE,
  JER2_ARAD_PMF_LOW_LEVEL_PGM_INIT_UNSAFE,
  JER2_ARAD_PMF_DB_TAG_FLD_SELECT,
  JER2_ARAD_TCAM_KEY_FIELD_LSB_SIZE_GET,
  JER2_ARAD_PMF_TCAM_KEY_A_B_FIELD_LSB_SIZE_GET,
  JER2_ARAD_TCAM_KEY_SIZE_GET,
  JER2_ARAD_PMF_CE_IRPP_FIELD_SIZE_AND_VAL_GET,
  JER2_ARAD_PMF_DIAG_FORCE_PROG_SET,
  JER2_ARAD_PMF_DIAG_FORCE_PROG_SET_PRINT,
  JER2_ARAD_PMF_DIAG_FORCE_PROG_SET_UNSAFE,
  JER2_ARAD_PMF_DIAG_FORCE_PROG_SET_VERIFY,
  JER2_ARAD_PMF_DIAG_FORCE_PROG_GET,
  JER2_ARAD_PMF_DIAG_FORCE_PROG_GET_PRINT,
  JER2_ARAD_PMF_DIAG_FORCE_PROG_GET_VERIFY,
  JER2_ARAD_PMF_DIAG_FORCE_PROG_GET_UNSAFE,
  JER2_ARAD_PMF_DIAG_SELECTED_PROGS_GET,
  JER2_ARAD_PMF_DIAG_SELECTED_PROGS_GET_PRINT,
  JER2_ARAD_PMF_DIAG_SELECTED_PROGS_GET_UNSAFE,
  JER2_ARAD_PMF_DIAG_SELECTED_PROGS_GET_VERIFY,
  JER2_ARAD_PMF_DIAG_FORCE_ACTION_SET,
  JER2_ARAD_PMF_DIAG_FORCE_ACTION_SET_PRINT,
  JER2_ARAD_PMF_DIAG_FORCE_ACTION_SET_UNSAFE,
  JER2_ARAD_PMF_DIAG_FORCE_ACTION_SET_VERIFY,
  JER2_ARAD_PMF_DIAG_FORCE_ACTION_GET,
  JER2_ARAD_PMF_DIAG_FORCE_ACTION_GET_PRINT,
  JER2_ARAD_PMF_DIAG_FORCE_ACTION_GET_VERIFY,
  JER2_ARAD_PMF_DIAG_FORCE_ACTION_GET_UNSAFE,
  JER2_ARAD_PMF_DIAG_FORCE_ACTION_UPDATE,
  JER2_ARAD_PMF_DIAG_FORCE_ACTION_UPDATE_PRINT,
  JER2_ARAD_PMF_DIAG_FORCE_ACTION_UPDATE_UNSAFE,
  JER2_ARAD_PMF_DIAG_FORCE_ACTION_UPDATE_VERIFY,
  JER2_ARAD_PMF_DIAG_BUILT_KEYS_GET,
  JER2_ARAD_PMF_DIAG_BUILT_KEYS_GET_PRINT,
  JER2_ARAD_PMF_DIAG_BUILT_KEYS_GET_UNSAFE,
  JER2_ARAD_PMF_DIAG_BUILT_KEYS_GET_VERIFY,
  JER2_ARAD_PMF_DIAG_FEM_FREEZE_SET,
  JER2_ARAD_PMF_DIAG_FEM_FREEZE_SET_PRINT,
  JER2_ARAD_PMF_DIAG_FEM_FREEZE_SET_UNSAFE,
  JER2_ARAD_PMF_DIAG_FEM_FREEZE_SET_VERIFY,
  JER2_ARAD_PMF_DIAG_FEM_FREEZE_GET,
  JER2_ARAD_PMF_DIAG_FEM_FREEZE_GET_PRINT,
  JER2_ARAD_PMF_DIAG_FEM_FREEZE_GET_VERIFY,
  JER2_ARAD_PMF_DIAG_FEM_FREEZE_GET_UNSAFE,
  JER2_ARAD_PMF_DIAG_FEMS_INFO_GET,
  JER2_ARAD_PMF_DIAG_FEMS_INFO_GET_PRINT,
  JER2_ARAD_PMF_DIAG_FEMS_INFO_GET_UNSAFE,
  JER2_ARAD_PMF_DIAG_FEMS_INFO_GET_VERIFY,
  JER2_ARAD_PP_IHP_PINFO_LLR_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHP_PINFO_LLR_TBL_SET_UNSAFE,
  JER2_ARAD_PP_IHP_LIF_TABLE_TBL_SET_UNSAFE,
  JER2_ARAD_PP_IHP_LIF_TABLE_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHP_VTT_PP_PORT_VSI_PROFILES_TBL_SET_UNSAFE,
  JER2_ARAD_PP_IHP_VTT_PP_PORT_VSI_PROFILES_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHP_LLR_LLVP_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHP_LLR_LLVP_TBL_SET_UNSAFE,
  JER2_ARAD_PP_IHP_LL_MIRROR_PROFILE_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHP_LL_MIRROR_PROFILE_TBL_SET_UNSAFE,
  JER2_ARAD_PP_IHP_SUBNET_CLASSIFY_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHP_SUBNET_CLASSIFY_TBL_SET_UNSAFE,
  JER2_ARAD_PP_IHP_PORT_PROTOCOL_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHP_PORT_PROTOCOL_TBL_SET_UNSAFE,
  JER2_ARAD_PP_IHP_TOS_2_COS_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHP_TOS_2_COS_TBL_SET_UNSAFE,
  JER2_ARAD_PP_IHP_RESERVED_MC_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHP_RESERVED_MC_TBL_SET_UNSAFE,
  JER2_ARAD_PP_IHP_MACT_FLUSH_DB_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHP_MACT_FLUSH_DB_TBL_SET_UNSAFE,
  JER2_ARAD_PP_IHP_MACT_AGING_CONFIGURATION_TABLE_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHP_MACT_AGING_CONFIGURATION_TABLE_TBL_SET_UNSAFE,
  JER2_ARAD_PP_IHP_TM_PORT_PARSER_PROGRAM_POINTER_CONFIG_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHP_TM_PORT_PARSER_PROGRAM_POINTER_CONFIG_TBL_SET_UNSAFE,
  JER2_ARAD_PP_IHP_PP_PORT_INFO_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHP_PP_PORT_INFO_TBL_SET_UNSAFE,
  JER2_ARAD_PP_IHP_VIRTUAL_PORT_FEM_BIT_SELECT_TABLE_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHP_VIRTUAL_PORT_FEM_BIT_SELECT_TABLE_TBL_SET_UNSAFE,
  JER2_ARAD_PP_IHP_VIRTUAL_PORT_FEM_MAP_INDEX_TABLE_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHP_VIRTUAL_PORT_FEM_MAP_INDEX_TABLE_TBL_SET_UNSAFE,
  JER2_ARAD_PP_IHP_VIRTUAL_PORT_FEM_FIELD_SELECT_MAP_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHP_VIRTUAL_PORT_FEM_FIELD_SELECT_MAP_TBL_SET_UNSAFE,
  JER2_ARAD_PP_IHP_SRC_SYSTEM_PORT_FEM_BIT_SELECT_TABLE_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHP_SRC_SYSTEM_PORT_FEM_BIT_SELECT_TABLE_TBL_SET_UNSAFE,
  JER2_ARAD_PP_IHP_SRC_SYSTEM_PORT_FEM_MAP_INDEX_TABLE_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHP_SRC_SYSTEM_PORT_FEM_MAP_INDEX_TABLE_TBL_SET_UNSAFE,
  JER2_ARAD_PP_IHP_SRC_SYSTEM_PORT_FEM_FIELD_SELECT_MAP_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHP_SRC_SYSTEM_PORT_FEM_FIELD_SELECT_MAP_TBL_SET_UNSAFE,
  JER2_ARAD_PP_IHP_PARSER_PROGRAM_POINTER_FEM_BIT_SELECT_TABLE_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHP_PARSER_PROGRAM_POINTER_FEM_BIT_SELECT_TABLE_TBL_SET_UNSAFE,
  JER2_ARAD_PP_IHP_PARSER_PROGRAM_POINTER_FEM_MAP_INDEX_TABLE_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHP_PARSER_PROGRAM_POINTER_FEM_MAP_INDEX_TABLE_TBL_SET_UNSAFE,
  JER2_ARAD_PP_IHP_PARSER_PROGRAM_POINTER_FEM_FIELD_SELECT_MAP_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHP_PARSER_PROGRAM_POINTER_FEM_FIELD_SELECT_MAP_TBL_SET_UNSAFE,
  JER2_ARAD_PP_IHP_PARSER_PROGRAM_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHP_PARSER_PROGRAM_TBL_SET_UNSAFE,
  JER2_ARAD_PP_IHP_PACKET_FORMAT_TABLE_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHP_PACKET_FORMAT_TABLE_TBL_SET_UNSAFE,
  JER2_ARAD_PP_IHP_PARSER_CUSTOM_MACRO_PARAMETERS_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHP_PARSER_CUSTOM_MACRO_PARAMETERS_TBL_SET_UNSAFE,
  JER2_ARAD_PP_IHP_PARSER_ETH_PROTOCOLS_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHP_PARSER_ETH_PROTOCOLS_TBL_SET_UNSAFE,
  JER2_ARAD_PP_IHP_PARSER_IP_PROTOCOLS_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHP_PARSER_IP_PROTOCOLS_TBL_SET_UNSAFE,
  JER2_ARAD_PP_IHP_PARSER_CUSTOM_MACRO_PROTOCOLS_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHP_PARSER_CUSTOM_MACRO_PROTOCOLS_TBL_SET_UNSAFE,
  JER2_ARAD_PP_IHP_ISEM_MANAGEMENT_REQUEST_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHP_ISEM_MANAGEMENT_REQUEST_TBL_SET_UNSAFE,
  JER2_ARAD_PP_IHP_VSI_LOW_CFG_1_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHP_VSI_LOW_CFG_1_TBL_SET_UNSAFE,
  JER2_ARAD_PP_IHP_VSI_LOW_CFG_2_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHP_VSI_LOW_CFG_2_TBL_SET_UNSAFE,
  JER2_ARAD_PP_IHP_VSI_HIGH_MY_MAC_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHP_VSI_HIGH_MY_MAC_TBL_SET_UNSAFE,
  JER2_ARAD_PP_IHP_VSI_HIGH_PROFILE_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHP_VSI_HIGH_PROFILE_TBL_SET_UNSAFE,
  JER2_ARAD_PP_IHP_VSI_HIGH_DA_NOT_FOUND_DESTINATION_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHP_VSI_HIGH_DA_NOT_FOUND_DESTINATION_TBL_SET_UNSAFE,
  JER2_ARAD_PP_IHP_MACT_FID_COUNTER_DB_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHP_MACT_FID_COUNTER_DB_TBL_SET_UNSAFE,
  JER2_ARAD_PP_IHP_MACT_FID_PROFILE_DB_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHP_MACT_FID_PROFILE_DB_TBL_SET_UNSAFE,
  JER2_ARAD_PP_IHP_MACT_FID_COUNTER_PROFILE_DB_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHP_MACT_FID_COUNTER_PROFILE_DB_TBL_SET_UNSAFE,
  JER2_ARAD_PP_IHP_BVD_TOPOLOGY_ID_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHP_BVD_TOPOLOGY_ID_TBL_SET_UNSAFE,
  JER2_ARAD_PP_IHP_BVD_FID_CLASS_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHP_BVD_FID_CLASS_TBL_SET_UNSAFE,
  JER2_ARAD_PP_IHP_FID_CLASS_2_FID_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHP_FID_CLASS_2_FID_TBL_SET_UNSAFE,
  JER2_ARAD_PP_IHP_VLAN_RANGE_COMPRESSION_TABLE_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHP_VLAN_RANGE_COMPRESSION_TABLE_TBL_SET_UNSAFE,
  JER2_ARAD_PP_IHP_VTT_IN_PP_PORT_VLAN_CONFIG_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHP_VTT_IN_PP_PORT_VLAN_CONFIG_TBL_SET_UNSAFE,
  JER2_ARAD_PP_IHP_DESIGNATED_VLAN_TABLE_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHP_DESIGNATED_VLAN_TABLE_TBL_SET_UNSAFE,
  JER2_ARAD_PP_IHP_VLAN_PORT_MEMBERSHIP_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHP_VLAN_PORT_MEMBERSHIP_TBL_SET_UNSAFE,
  JER2_ARAD_PP_IHP_VTT_IN_PP_PORT_CONFIG_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHP_VTT_IN_PP_PORT_CONFIG_TBL_SET_UNSAFE,
  JER2_ARAD_PP_IHP_LIF_TABLE_AC_P2P_TO_AC_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHP_LIF_TABLE_AC_P2P_TO_AC_TBL_SET_UNSAFE,
  JER2_ARAD_PP_IHP_LIF_TABLE_AC_P2P_TO_PWE_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHP_LIF_TABLE_AC_P2P_TO_PWE_TBL_SET_UNSAFE,
  JER2_ARAD_PP_IHP_LIF_TABLE_AC_P2P_TO_PBB_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHP_LIF_TABLE_AC_P2P_TO_PBB_TBL_SET_UNSAFE,
  JER2_ARAD_PP_IHP_LIF_TABLE_AC_MP_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHP_LIF_TABLE_AC_MP_TBL_SET_UNSAFE,
  JER2_ARAD_PP_IHP_LIF_TABLE_ISID_P2P_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHP_LIF_TABLE_ISID_P2P_TBL_SET_UNSAFE,
  JER2_ARAD_PP_IHP_LIF_TABLE_ISID_MP_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHP_LIF_TABLE_ISID_MP_TBL_SET_UNSAFE,
  JER2_ARAD_PP_IHP_LIF_TABLE_TRILL_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHP_LIF_TABLE_TRILL_TBL_SET_UNSAFE,
  JER2_ARAD_PP_IHP_LIF_TABLE_IP_TT_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHP_LIF_TABLE_IP_TT_TBL_SET_UNSAFE,
  JER2_ARAD_PP_IHP_LIF_TABLE_LABEL_PWE_P2P_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHP_LIF_TABLE_LABEL_PWE_P2P_TBL_SET_UNSAFE,
  JER2_ARAD_PP_IHP_LIF_TABLE_LABEL_PWE_MP_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHP_LIF_TABLE_LABEL_PWE_MP_TBL_SET_UNSAFE,
  JER2_ARAD_PP_IHP_LIF_TABLE_LABEL_PROTOCOL_OR_LSP_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHP_LIF_TABLE_LABEL_PROTOCOL_OR_LSP_TBL_SET_UNSAFE,
  JER2_ARAD_PP_IHP_STP_TABLE_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHP_STP_TABLE_TBL_SET_UNSAFE,
  JER2_ARAD_PP_IHP_VRID_MY_MAC_MAP_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHP_VRID_MY_MAC_MAP_TBL_SET_UNSAFE,
  JER2_ARAD_PP_IHP_VTT_LLVP_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHP_VTT_LLVP_TBL_SET_UNSAFE,
  JER2_ARAD_PP_IHP_ISEM_1ST_PROGRAM_SELECTION_CAM_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHP_ISEM_1ST_PROGRAM_SELECTION_CAM_TBL_SET_UNSAFE,
  JER2_ARAD_PP_IHP_ISEM_2ND_PROGRAM_SELECTION_CAM_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHP_ISEM_2ND_PROGRAM_SELECTION_CAM_TBL_SET_UNSAFE,
  JER2_ARAD_PP_IHP_VTT1ST_KEY_CONSTRUCTION0_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHP_VTT1ST_KEY_CONSTRUCTION0_TBL_SET_UNSAFE,
  JER2_ARAD_PP_IHP_VTT1ST_KEY_CONSTRUCTION1_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHP_VTT1ST_KEY_CONSTRUCTION1_TBL_SET_UNSAFE,
  JER2_ARAD_PP_IHP_VTT2ND_KEY_CONSTRUCTION_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHP_VTT2ND_KEY_CONSTRUCTION_TBL_SET_UNSAFE,
  JER2_ARAD_PP_IHP_INGRESS_VLAN_EDIT_COMMAND_TABLE_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHP_INGRESS_VLAN_EDIT_COMMAND_TABLE_TBL_SET_UNSAFE,
  JER2_ARAD_PP_IHP_VLAN_EDIT_PCP_DEI_MAP_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHP_VLAN_EDIT_PCP_DEI_MAP_TBL_SET_UNSAFE,
  JER2_ARAD_PP_IHP_TERMINATION_PROFILE_TABLE_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHP_TERMINATION_PROFILE_TABLE_TBL_SET_UNSAFE,
  JER2_ARAD_PP_IHP_ACTION_PROFILE_MPLS_VALUE_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHP_ACTION_PROFILE_MPLS_VALUE_TBL_SET_UNSAFE,
  JER2_ARAD_PP_IHP_IN_RIF_CONFIG_TABLE_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHP_IN_RIF_CONFIG_TABLE_TBL_SET_UNSAFE,
  JER2_ARAD_PP_IHP_TC_DP_MAP_TABLE_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHP_TC_DP_MAP_TABLE_TBL_SET_UNSAFE,
  JER2_ARAD_PP_IHB_PINFO_FER_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHB_PINFO_FER_TBL_SET_UNSAFE,
  JER2_ARAD_PP_IHB_LB_PFC_PROFILE_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHB_LB_PFC_PROFILE_TBL_SET_UNSAFE,
  JER2_ARAD_PP_IHB_LB_VECTOR_PROGRAM_MAP_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHB_LB_VECTOR_PROGRAM_MAP_TBL_SET_UNSAFE,
  JER2_ARAD_PP_IHB_FEC_SUPER_ENTRY_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHB_FEC_SUPER_ENTRY_TBL_SET_UNSAFE,
  JER2_ARAD_PP_IHB_FEC_ECMP_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHB_FEC_ECMP_TBL_SET_UNSAFE,
  JER2_ARAD_PP_IHB_FEC_ENTRY_GENERAL_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHB_FEC_ENTRY_GENERAL_TBL_SET_UNSAFE,

  JER2_ARAD_PP_IHB_FEC_ENTRY_ACCESSED_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHB_FEC_ENTRY_ACCESSED_TBL_SET_UNSAFE,
  JER2_ARAD_PP_IHB_PATH_SELECT_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHB_PATH_SELECT_TBL_SET_UNSAFE,
  JER2_ARAD_PP_IHB_DESTINATION_STATUS_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHB_DESTINATION_STATUS_TBL_SET_UNSAFE,
  JER2_ARAD_PP_IHB_FWD_ACT_PROFILE_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHB_FWD_ACT_PROFILE_TBL_SET_UNSAFE,
  JER2_ARAD_PP_IHB_SNOOP_ACTION_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHB_SNOOP_ACTION_TBL_SET_UNSAFE,
  JER2_ARAD_PP_IHB_PINFO_FLP_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHB_PINFO_FLP_TBL_SET_UNSAFE,
  JER2_ARAD_PP_IHB_VRF_CONFIG_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHB_VRF_CONFIG_TBL_SET_UNSAFE,
  JER2_ARAD_PP_IHB_HEADER_PROFILE_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHB_HEADER_PROFILE_TBL_SET_UNSAFE,
  JER2_ARAD_PP_IHB_SNP_ACT_PROFILE_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHB_SNP_ACT_PROFILE_TBL_SET_UNSAFE,
  JER2_ARAD_PP_IHB_MRR_ACT_PROFILE_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHB_MRR_ACT_PROFILE_TBL_SET_UNSAFE,
  JER2_ARAD_PP_IHB_LPM_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHB_LPM_TBL_SET_UNSAFE,
  JER2_ARAD_PP_IHB_TCAM_ACTION_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHB_TCAM_ACTION_TBL_SET_UNSAFE,
  JER2_ARAD_PP_EGQ_PP_PPCT_TBL_GET_UNSAFE,
  JER2_ARAD_PP_EGQ_PP_PPCT_TBL_SET_UNSAFE,
  JER2_ARAD_PP_EGQ_INGRESS_VLAN_EDIT_COMMAND_MAP_TBL_GET_UNSAFE,
  JER2_ARAD_PP_EGQ_INGRESS_VLAN_EDIT_COMMAND_MAP_TBL_SET_UNSAFE,
  JER2_ARAD_PP_EGQ_VSI_MEMBERSHIP_TBL_GET_UNSAFE,
  JER2_ARAD_PP_EGQ_VSI_MEMBERSHIP_TBL_SET_UNSAFE,
  JER2_ARAD_PP_EGQ_TTL_SCOPE_TBL_GET_UNSAFE,
  JER2_ARAD_PP_EGQ_TTL_SCOPE_TBL_SET_UNSAFE,
  JER2_ARAD_PP_EGQ_AUX_TABLE_TBL_GET_UNSAFE,
  JER2_ARAD_PP_EGQ_AUX_TABLE_TBL_SET_UNSAFE,
  JER2_ARAD_PP_EGQ_ACTION_PROFILE_TABLE_TBL_GET_UNSAFE,
  JER2_ARAD_PP_EGQ_ACTION_PROFILE_TABLE_TBL_SET_UNSAFE,
  JER2_ARAD_PP_EPNI_PCP_DEI_MAP_TBL_GET_UNSAFE,
  JER2_ARAD_PP_EPNI_PCP_DEI_MAP_TBL_SET_UNSAFE,
  JER2_ARAD_PP_EPNI_TX_TAG_TABLE_TBL_GET_UNSAFE,
  JER2_ARAD_PP_EPNI_TX_TAG_TABLE_TBL_SET_UNSAFE,
  JER2_ARAD_PP_EPNI_STP_TBL_GET_UNSAFE,
  JER2_ARAD_PP_EPNI_STP_TBL_SET_UNSAFE,
  JER2_ARAD_PP_EPNI_SMALL_EM_RESULT_MEMORY_TBL_GET_UNSAFE,
  JER2_ARAD_PP_EPNI_SMALL_EM_RESULT_MEMORY_TBL_SET_UNSAFE,
  JER2_ARAD_PP_EPNI_PCP_DEI_TABLE_TBL_GET_UNSAFE,
  JER2_ARAD_PP_EPNI_PCP_DEI_TABLE_TBL_SET_UNSAFE,
  JER2_ARAD_PP_EPNI_PP_PCT_TBL_GET_UNSAFE,
  JER2_ARAD_PP_EPNI_PP_PCT_TBL_SET_UNSAFE,
  JER2_ARAD_PP_EPNI_LLVP_TABLE_TBL_GET_UNSAFE,
  JER2_ARAD_PP_EPNI_LLVP_TABLE_TBL_SET_UNSAFE,
  JER2_ARAD_PP_EPNI_EGRESS_EDIT_CMD_TBL_GET_UNSAFE,
  JER2_ARAD_PP_EPNI_EGRESS_EDIT_CMD_TBL_SET_UNSAFE,
  JER2_ARAD_PP_EPNI_INGRESS_VLAN_EDIT_COMMAND_MAP_TBL_GET_UNSAFE,
  JER2_ARAD_PP_EPNI_INGRESS_VLAN_EDIT_COMMAND_MAP_TBL_SET_UNSAFE,
  JER2_ARAD_PP_EPNI_EXP_REMARK_TBL_GET_UNSAFE,
  JER2_ARAD_PP_EPNI_EXP_REMARK_TBL_SET_UNSAFE,
  JER2_ARAD_PP_EPNI_DSCP_REMARK_TBL_GET_UNSAFE,
  JER2_ARAD_PP_EPNI_DSCP_REMARK_TBL_SET_UNSAFE,
  JER2_ARAD_PP_EPNI_REMARK_MPLS_TO_EXP_TBL_GET_UNSAFE,
  JER2_ARAD_PP_EPNI_REMARK_MPLS_TO_DSCP_TBL_GET_UNSAFE,
  JER2_ARAD_PP_EPNI_REMARK_IPV4_TO_EXP_TBL_GET_UNSAFE,
  JER2_ARAD_PP_EPNI_REMARK_IPV4_TO_DSCP_TBL_GET_UNSAFE,
  JER2_ARAD_PP_EPNI_REMARK_IPV6_TO_EXP_TBL_GET_UNSAFE,
  JER2_ARAD_PP_EPNI_REMARK_IPV6_TO_DSCP_TBL_GET_UNSAFE,
  JER2_ARAD_PP_EPNI_REMARK_MPLS_TO_EXP_TBL_SET_UNSAFE,
  JER2_ARAD_PP_EPNI_REMARK_MPLS_TO_DSCP_TBL_SET_UNSAFE,
  JER2_ARAD_PP_EPNI_REMARK_IPV4_TO_EXP_TBL_SET_UNSAFE,
  JER2_ARAD_PP_EPNI_REMARK_IPV4_TO_DSCP_TBL_SET_UNSAFE,
  JER2_ARAD_PP_EPNI_REMARK_IPV6_TO_EXP_TBL_SET_UNSAFE,
  JER2_ARAD_PP_EPNI_REMARK_IPV6_TO_DSCP_TBL_SET_UNSAFE,
  JER2_ARAD_PP_EPNI_PRGE_PROGRAM_SELECTION_CAM_TBL_GET_UNSAFE,
  JER2_ARAD_PP_EPNI_PRGE_PROGRAM_SELECTION_CAM_TBL_SET_UNSAFE,
  JER2_ARAD_PP_EPNI_PRGE_PROGRAM_TBL_SET_UNSAFE,
  JER2_ARAD_PP_EPNI_PRGE_PROGRAM_TBL_GET_UNSAFE,
  JER2_ARAD_PP_EPNI_PRGE_INSTRUCTION_TBL_SET_UNSAFE,
  JER2_ARAD_PP_EPNI_PRGE_INSTRUCTION_TBL_GET_UNSAFE,
  JER2_ARAD_PP_EPNI_PRGE_DATA_TBL_GET_UNSAFE,
  JER2_ARAD_PP_EPNI_PRGE_DATA_TBL_SET_UNSAFE,
  JER2_ARAD_PP_EGQ_DSP_PTR_MAP_TBL_GET_UNSAFE,
  JER2_ARAD_PP_EGQ_DSP_PTR_MAP_TBL_SET_UNSAFE,
  JER2_ARAD_PP_FRWRD_IP_TCAM_INIT_UNSAFE,
  JER2_ARAD_PORTS_SET_IN_SRC_SYS_PORT_SET_UNSAFE,
  JER2_ARAD_PORTS_SET_IN_SRC_SYS_PORT_GET_UNSAFE,
  JER2_ARAD_PORTS_FAP_PORT_ID_CUD_EXTENSION_VERIFY,
  JER2_ARAD_PORTS_INIT_INTERFACES_CONTEXT_MAP,
  JER2_ARAD_PORT_TO_INTERFACE_MAP_SYMMETRIC_GET_UNSAFE,
  JER2_ARAD_PARSER_INIT,
  JER2_ARAD_PARSER_INGRESS_SHAPE_STATE_SET,
  JER2_ARAD_PORT_PP_PORT_SET_UNSAFE,
  JER2_ARAD_PORT_PP_PORT_SET_VERIFY,
  JER2_ARAD_PORT_PP_PORT_GET,
  JER2_ARAD_PORT_PP_PORT_GET_UNSAFE,
  JER2_ARAD_PORT_PP_PORT_GET_VERIFY,
  JER2_ARAD_PORT_TM_PROFILE_REMOVE,
  JER2_ARAD_PORT_TM_PROFILE_REMOVE_UNSAFE,
  JER2_ARAD_PORT_TM_PROFILE_REMOVE_VERIFY,
  JER2_ARAD_PORT_TO_PP_PORT_MAP_SET,
  JER2_ARAD_PORT_TO_PP_PORT_MAP_SET_UNSAFE,
  JER2_ARAD_PORT_TO_PP_PORT_MAP_SET_VERIFY,
  JER2_ARAD_PORT_TO_PP_PORT_MAP_GET,
  JER2_ARAD_PORT_TO_PP_PORT_MAP_GET_VERIFY,
  JER2_ARAD_PORT_TO_PP_PORT_MAP_GET_UNSAFE,
  JER2_ARAD_A_LOCAL_TO_SYS_PHYS_PORT_MAP_GET_UNSAFE,
  JER2_ARAD_PP_IHP_VTT1ST_KEY_CONSTRUCTION_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHP_VTT1ST_KEY_CONSTRUCTION_TBL_SET_UNSAFE,
  JER2_ARAD_PORT_INGR_MAP_WRITE_VAL,
  JER2_ARAD_PORT2IF_INGR_MAP_NEW_UNMAP_OLD,
  JER2_ARAD_PORT_EGR_MAP_WRITE_VAL,
  JER2_ARAD_PORT2IF_EGR_MAP_NEW_UNMAP_OLD,
  JER2_ARAD_PORT_ADD_EGR_MAP_WRITE_VAL,
  JER2_ARAD_PORTS_SWAP_SET_VERIFY,
  JER2_ARAD_PORTS_SWAP_SET_UNSAFE,
  JER2_ARAD_PORTS_SWAP_GET_VERIFY,
  JER2_ARAD_PORTS_SWAP_GET_UNSAFE,
  JER2_ARAD_PORTS_SWAP_SET,
  JER2_ARAD_PORTS_SWAP_GET,
  JER2_ARAD_SWAP_INFO_SET_VERIFY,
  JER2_ARAD_SWAP_INFO_SET_UNSAFE,
  JER2_ARAD_SWAP_INFO_GET_VERIFY,
  JER2_ARAD_SWAP_INFO_GET_UNSAFE,
  JER2_ARAD_MGMT_SWAP_INIT,
  JER2_ARAD_NIF_ON_OFF_SET_VERIFY,
  JER2_ARAD_NIF_ON_OFF_GET_VERIFY,
  JER2_ARAD_NIF_LOOPBACK_SET_PRINT,
  JER2_ARAD_NIF_LOOPBACK_SET_VERIFY,
  JER2_ARAD_NIF_LOOPBACK_GET_PRINT,
  JER2_ARAD_NIF_LOOPBACK_GET_VERIFY,
  JER2_ARAD_SCH_PORT_TC2SE_ID,
  JER2_ARAD_SCH_SE2PORT_TC_ID,
  JER2_ARAD_SCH_PORT_TCG_WEIGHT_SET,
  JER2_ARAD_SCH_PORT_TCG_WEIGHT_GET,
  JER2_ARAD_SCH_PORT_TCG_WEIGHT_SET_UNSAFE,
  JER2_ARAD_SCH_PORT_TCG_WEIGHT_GET_UNSAFE,
  JER2_ARAD_SCH_PORT_TCG_WEIGHT_SET_VERIFY_UNSAFE,
  JER2_ARAD_SCH_PORT_TCG_WEIGHT_GET_VERIFY_UNSAFE,
  JER2_ARAD_NIF_ILKN_CLK_CONFIG,
  JER2_ARAD_CNT_GET_INGRESS_MODE,
  JER2_ARAD_CNT_CHECK_VOQ_PARAMS,
  JER2_ARAD_CNT_CHECK_VOQ_SET_SIZE,
  JER2_ARAD_CNT_GET_EGRESS_MODE,
  JER2_ARAD_CNT_GET_PROCESSOR_ID,
  JER2_ARAD_CNT_GET_SOURCE_ID,
  JER2_ARAD_CNT_INIT,
  JER2_ARAD_INIT_DRAM_PLL_SET,
  JER2_ARAD_SW_DB_MULTISET_ADD,
  JER2_ARAD_SW_DB_MULTISET_REMOVE,
  JER2_ARAD_SW_DB_MULTISET_LOOKUP,
  JER2_ARAD_SW_DB_MULTISET_ADD_BY_INDEX,
  JER2_ARAD_SW_DB_MULTISET_REMOVE_BY_INDEX,
  JER2_ARAD_SW_DB_MULTISET_CLEAR,
  JER2_ARAD_TCAM_BANK_INIT_UNSAFE,
  JER2_ARAD_TCAM_BANK_INIT_VERIFY,
  JER2_ARAD_TCAM_DB_CREATE_UNSAFE,
  JER2_ARAD_TCAM_DB_CREATE_VERIFY,
  JER2_ARAD_TCAM_DB_DESTROY_UNSAFE,
  JER2_ARAD_TCAM_DB_DESTROY_VERIFY,
  JER2_ARAD_TCAM_DB_BANK_ADD_UNSAFE,
  JER2_ARAD_TCAM_DB_BANK_ADD_VERIFY,
  JER2_ARAD_TCAM_DB_BANK_REMOVE_UNSAFE,
  JER2_ARAD_TCAM_DB_BANK_REMOVE_VERIFY,
  JER2_ARAD_TCAM_DB_NOF_BANKS_GET_UNSAFE,
  JER2_ARAD_TCAM_DB_NOF_BANKS_GET_VERIFY,
  JER2_ARAD_TCAM_DB_BANK_PREFIX_GET_UNSAFE,
  JER2_ARAD_TCAM_DB_BANK_PREFIX_GET_VERIFY,
  JER2_ARAD_TCAM_DB_ENTRY_SIZE_GET_UNSAFE,
  JER2_ARAD_TCAM_DB_ENTRY_SIZE_GET_VERIFY,
  JER2_ARAD_TCAM_DB_DIRECT_TBL_ENTRY_SET_UNSAFE,
  JER2_ARAD_TCAM_DB_DIRECT_TBL_ENTRY_SET_VERIFY,
  JER2_ARAD_TCAM_DB_ENTRY_ADD_UNSAFE,
  JER2_ARAD_TCAM_DB_ENTRY_ADD_VERIFY,
  JER2_ARAD_TCAM_DB_ENTRY_GET_UNSAFE,
  JER2_ARAD_TCAM_DB_ENTRY_GET_VERIFY,
  JER2_ARAD_TCAM_DB_ENTRY_SEARCH_UNSAFE,
  JER2_ARAD_TCAM_DB_ENTRY_SEARCH_VERIFY,
  JER2_ARAD_TCAM_DB_ENTRY_REMOVE_UNSAFE,
  JER2_ARAD_TCAM_DB_ENTRY_REMOVE_VERIFY,
  JER2_ARAD_TCAM_DB_IS_BANK_USED_UNSAFE,
  JER2_ARAD_TCAM_DB_IS_BANK_USED_VERIFY,
  JER2_ARAD_TCAM_ACCESS_PROFILE_CREATE,
  JER2_ARAD_TCAM_ACCESS_PROFILE_CREATE_PRINT,
  JER2_ARAD_TCAM_ACCESS_PROFILE_CREATE_UNSAFE,
  JER2_ARAD_TCAM_ACCESS_PROFILE_CREATE_VERIFY,
  JER2_ARAD_TCAM_MANAGED_DB_ENTRY_ADD,
  JER2_ARAD_TCAM_MANAGED_DB_ENTRY_ADD_PRINT,
  JER2_ARAD_TCAM_MANAGED_DB_ENTRY_ADD_UNSAFE,
  JER2_ARAD_TCAM_MANAGED_DB_ENTRY_ADD_VERIFY,
  JER2_ARAD_TCAM_MANAGED_DB_ENTRY_REMOVE,
  JER2_ARAD_TCAM_MANAGED_DB_ENTRY_REMOVE_PRINT,
  JER2_ARAD_TCAM_MANAGED_DB_ENTRY_REMOVE_UNSAFE,
  JER2_ARAD_TCAM_MANAGED_DB_ENTRY_REMOVE_VERIFY,
  JER2_ARAD_TCAM_ACCESS_PROFILE_ACCESS_DEVICE_GET,
  JER2_ARAD_TCAM_ACCESS_PROFILE_ACCESS_DEVICE_GET_PRINT,
  JER2_ARAD_TCAM_ACCESS_PROFILE_ACCESS_DEVICE_GET_UNSAFE,
  JER2_ARAD_TCAM_ACCESS_PROFILE_ACCESS_DEVICE_GET_VERIFY,
    JER2_ARAD_PMF_PGM_PACKET_FORMAT_CODE_CONVERT,
  JER2_ARAD_PMF_PGM_MGMT_SET,
  JER2_ARAD_PORT_PARSE_HEADER_TYPE_UNSAFE,
  JER2_ARAD_TDM_DIRECT_ROUTING_SET,
  JER2_ARAD_TDM_DIRECT_ROUTING_SET_PRINT,
  JER2_ARAD_TDM_DIRECT_ROUTING_SET_UNSAFE,
  JER2_ARAD_TDM_DIRECT_ROUTING_SET_VERIFY,
  JER2_ARAD_TDM_DIRECT_ROUTING_GET,
  JER2_ARAD_TDM_DIRECT_ROUTING_GET_PRINT,
  JER2_ARAD_TDM_DIRECT_ROUTING_GET_VERIFY,
  JER2_ARAD_TDM_DIRECT_ROUTING_GET_UNSAFE,
  JER2_ARAD_TDM_PORT_PACKET_CRC_SET,
  JER2_ARAD_TDM_PORT_PACKET_CRC_SET_UNSAFE,
  JER2_ARAD_TDM_PORT_PACKET_CRC_SET_VERIFY,
  JER2_ARAD_TDM_PORT_PACKET_CRC_GET,
  JER2_ARAD_TDM_PORT_PACKET_CRC_GET_VERIFY,
  JER2_ARAD_TDM_PORT_PACKET_CRC_GET_UNSAFE,
  JER2_ARAD_TDM_DIRECT_ROUTING_PROFILE_MAP_SET,
  JER2_ARAD_TDM_DIRECT_ROUTING_PROFILE_MAP_SET_UNSAFE,
  JER2_ARAD_TDM_DIRECT_ROUTING_PROFILE_MAP_SET_VERIFY,
  JER2_ARAD_TDM_DIRECT_ROUTING_PROFILE_MAP_GET,
  JER2_ARAD_TDM_DIRECT_ROUTING_PROFILE_MAP_GET_VERIFY,
  JER2_ARAD_TDM_DIRECT_ROUTING_PROFILE_MAP_GET_UNSAFE,
  JER2_ARAD_EGR_SCHED_DROP_Q_PAIR_THRESH_SET_UNSAFE,
  JER2_ARAD_EGR_SCHED_DROP_Q_PAIR_THRESH_GET_UNSAFE,
  JER2_ARAD_EGR_UNSCHED_DROP_Q_PAIR_THRESH_SET_UNSAFE,
  JER2_ARAD_EGR_UNSCHED_DROP_Q_PAIR_THRESH_GET_UNSAFE,
  JER2_ARAD_EGR_OFP_FC_Q_PAIR_THRESH_GET_UNSAFE,
  JER2_ARAD_EGR_OFP_FC_Q_PAIR_THRESH_SET_UNSAFE,
  JER2_ARAD_SOC_MEM_FIELD32_VALIDATE,

  JER2_ARAD_PP_IHB_FLP_PROGRAM_SELECTION_CAM_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHB_FLP_PROGRAM_SELECTION_CAM_TBL_SET_UNSAFE,
  JER2_ARAD_PP_IHB_FLP_PROCESS_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHB_FLP_PROCESS_TBL_SET_UNSAFE,
  JER2_ARAD_PP_IHB_FLP_LOOKUPS_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHB_FLP_LOOKUPS_TBL_SET_UNSAFE,
  JER2_ARAD_PP_IHB_FLP_KEY_CONSTRUCTION_TBL_GET_UNSAFE,
  JER2_ARAD_PP_IHB_FLP_KEY_CONSTRUCTION_TBL_SET_UNSAFE,
  JER2_ARAD_PP_EPNI_ISID_TABLE_TBL_GET_UNSAFE,
  JER2_ARAD_PP_EPNI_ISID_TABLE_TBL_SET_UNSAFE,



  JER2_ARAD_WB_DB_INIT,
  JER2_ARAD_WB_DB_LAYOUT_INIT,
  JER2_ARAD_WB_DB_RESTORE_STATE,
  JER2_ARAD_WB_DB_INFO_ALLOC,
  JER2_ARAD_WB_DB_SYNC,

  JER2_ARAD_WB_DB_LAG_RESTORE_IN_USE_STATE,
  JER2_ARAD_WB_DB_LAG_RESTORE_LOCAL_TO_SYS_STATE,
  JER2_ARAD_WB_DB_LAG_RESTORE_LOCAL_TO_REASSEMBLY_CONTEXT_STATE,
  JER2_ARAD_WB_DB_LAG_RESTORE_LOCAL_TO_XGS_MAC_EXTENDER_INFO_STATE,
  JER2_ARAD_WB_DB_LAG_SAVE_IN_USE_STATE,
  JER2_ARAD_WB_DB_LAG_SAVE_LOCAL_TO_SYS_STATE,
  JER2_ARAD_WB_DB_LAG_SAVE_LOCAL_TO_REASSEMBLY_CONTEXT_STATE,
  JER2_ARAD_WB_DB_LAG_SAVE_LOCAL_TO_XGS_MAC_EXTENDER_INFO_STATE,
  JER2_ARAD_WB_DB_LAG_UPDATE_IN_USE_STATE,
  JER2_ARAD_WB_DB_LAG_UPDATE_LOCAL_TO_SYS_STATE,
  JER2_ARAD_WB_DB_LAG_UPDATE_LOCAL_TO_REASSEMBLY_CONTEXT_STATE,
  JER2_ARAD_WB_DB_LAG_UPDATE_LOCAL_TO_XGS_MAC_EXTENDER_INFO_STATE,
  
  JER2_ARAD_WB_DB_EGR_PORT_RESTORE_QUEUE_RATE_STATE,
  JER2_ARAD_WB_DB_EGR_PORT_RESTORE_TCG_RATE_STATE,
  JER2_ARAD_WB_DB_EGR_PORT_RESTORE_RATES_STATE,
  JER2_ARAD_WB_DB_EGR_PORT_RESTORE_PRIORITY_RATES_STATE,
  JER2_ARAD_WB_DB_EGR_PORT_RESTORE_TCG_SHAPER_RATES_STATE,
  JER2_ARAD_WB_DB_EGR_PORT_RESTORE_SCH_CHAN_ARB_RATE_STATE,
  JER2_ARAD_WB_DB_EGR_PORT_RESTORE_NOF_CALCAL_INSTANCES_STATE,
  JER2_ARAD_WB_DB_EGR_PORT_RESTORE_CALCAL_LENGTH_STATE,
  JER2_ARAD_WB_DB_EGR_PORT_RESTORE_UPDATE_DEVICE_STATE,
  JER2_ARAD_WB_DB_EGR_PORT_RESTORE_UPDATE_DEV_CHANGED_STATE,
  JER2_ARAD_WB_DB_EGR_PORT_RESTORE_EGQ_TCG_QPAIR_SHAPER_ENABLE_STATE,
  JER2_ARAD_WB_DB_EGR_PORT_RESTORE_ERP_INTERFACE_STATE,
  JER2_ARAD_WB_DB_EGR_PORT_RESTORE_DSP_PP_TO_BASE_QUEUE_PAIR_MAPPING_STATE,
  JER2_ARAD_WB_DB_EGR_PORT_RESTORE_DSP_PP_NOF_QUEUE_PAIRS_STATE,
  JER2_ARAD_WB_DB_EGR_PORT_RESTORE_PORTS_PROG_EDITOR_PROFILE_STATE,
  JER2_ARAD_WB_DB_EGR_PORT_SAVE_QUEUE_RATE_STATE,
  JER2_ARAD_WB_DB_EGR_PORT_SAVE_TCG_RATE_STATE,
  JER2_ARAD_WB_DB_EGR_PORT_SAVE_RATES_STATE,
  JER2_ARAD_WB_DB_EGR_PORT_SAVE_PRIORITY_RATES_STATE,
  JER2_ARAD_WB_DB_EGR_PORT_SAVE_TCG_SHAPER_RATES_STATE,
  JER2_ARAD_WB_DB_EGR_PORT_SAVE_SCH_CHAN_ARB_RATE_STATE,
  JER2_ARAD_WB_DB_EGR_PORT_SAVE_NOF_CALCAL_INSTANCES_STATE,
  JER2_ARAD_WB_DB_EGR_PORT_SAVE_CALCAL_LENGTH_STATE,
  JER2_ARAD_WB_DB_EGR_PORT_SAVE_UPDATE_DEVICE_STATE,
  JER2_ARAD_WB_DB_EGR_PORT_SAVE_UPDATE_DEV_CHANGED_STATE,
  JER2_ARAD_WB_DB_EGR_PORT_SAVE_EGQ_TCG_QPAIR_SHAPER_ENABLE_STATE,
  JER2_ARAD_WB_DB_EGR_PORT_SAVE_ERP_INTERFACE_STATE,
  JER2_ARAD_WB_DB_EGR_PORT_SAVE_DSP_PP_TO_BASE_QUEUE_PAIR_MAPPING_STATE,
  JER2_ARAD_WB_DB_EGR_PORT_SAVE_DSP_PP_NOF_QUEUE_PAIRS_STATE,
  JER2_ARAD_WB_DB_EGR_PORT_SAVE_PORTS_PROG_EDITOR_PROFILE_STATE,
  JER2_ARAD_WB_DB_EGR_PORT_UPDATE_QUEUE_RATE_STATE,
  JER2_ARAD_WB_DB_EGR_PORT_UPDATE_TCG_RATE_STATE,
  JER2_ARAD_WB_DB_EGR_PORT_UPDATE_RATES_STATE,
  JER2_ARAD_WB_DB_EGR_PORT_UPDATE_PRIORITY_RATES_STATE,
  JER2_ARAD_WB_DB_EGR_PORT_UPDATE_TCG_SHAPER_RATES_STATE,
  JER2_ARAD_WB_DB_EGR_PORT_UPDATE_SCH_CHAN_ARB_RATE_STATE,
  JER2_ARAD_WB_DB_EGR_PORT_UPDATE_NOF_CALCAL_INSTANCES_STATE,
  JER2_ARAD_WB_DB_EGR_PORT_UPDATE_CALCAL_LENGTH_STATE,
  JER2_ARAD_WB_DB_EGR_PORT_UPDATE_UPDATE_DEVICE_STATE,
  JER2_ARAD_WB_DB_EGR_PORT_UPDATE_UPDATE_DEV_CHANGED_STATE,
  JER2_ARAD_WB_DB_EGR_PORT_UPDATE_EGQ_TCG_QPAIR_SHAPER_ENABLE_STATE,
  JER2_ARAD_WB_DB_EGR_PORT_UPDATE_ERP_INTERFACE_STATE,
  JER2_ARAD_WB_DB_EGR_PORT_UPDATE_DSP_PP_TO_BASE_QUEUE_PAIR_MAPPING_STATE,
  JER2_ARAD_WB_DB_EGR_PORT_UPDATE_DSP_PP_NOF_QUEUE_PAIRS_STATE,
  JER2_ARAD_WB_DB_EGR_PORT_UPDATE_PORTS_PROG_EDITOR_PROFILE_STATE,
  JER2_ARAD_WB_DB_MULTICAST_RESTORE_NOF_UNOCCUPIED_STATE,
  JER2_ARAD_WB_DB_MULTICAST_RESTORE_UNOCCUPIED_LIST_HEAD_STATE,
  JER2_ARAD_WB_DB_MULTICAST_RESTORE_NEXT_UNOCCUPIED_PTR_STATE,
  JER2_ARAD_WB_DB_MULTICAST_RESTORE_BACKWARDS_PTR_STATE,
  JER2_ARAD_WB_DB_MULTICAST_RESTORE_EG_MULT_NOF_VLAN_BITMAPS_STATE,
  JER2_ARAD_WB_DB_MULTICAST_SAVE_NOF_UNOCCUPIED_STATE,
  JER2_ARAD_WB_DB_MULTICAST_SAVE_UNOCCUPIED_LIST_HEAD_STATE,
  JER2_ARAD_WB_DB_MULTICAST_SAVE_NEXT_UNOCCUPIED_PTR_STATE,
  JER2_ARAD_WB_DB_MULTICAST_SAVE_BACKWARDS_PTR_STATE,
  JER2_ARAD_WB_DB_MULTICAST_SAVE_EG_MULT_NOF_VLAN_BITMAPS_STATE,
  JER2_ARAD_WB_DB_MULTICAST_UPDATE_NOF_UNOCCUPIED_STATE,
  JER2_ARAD_WB_DB_MULTICAST_UPDATE_UNOCCUPIED_LIST_HEAD_STATE,
  JER2_ARAD_WB_DB_MULTICAST_UPDATE_NEXT_UNOCCUPIED_PTR_STATE,
  JER2_ARAD_WB_DB_MULTICAST_UPDATE_BACKWARDS_PTR_STATE,
  JER2_ARAD_WB_DB_MULTICAST_UPDATE_EG_MULT_NOF_VLAN_BITMAPS_STATE,
  
  JER2_ARAD_WB_DB_CELL_RESTORE_CURRENT_CELL_IDENT_STATE,
  JER2_ARAD_WB_DB_CELL_SAVE_CURRENT_CELL_IDENT_STATE,
  JER2_ARAD_WB_DB_CELL_UPDATE_CURRENT_CELL_IDENT_STATE,


  JER2_ARAD_WB_DB_TDM_RESTORE_CONTEXT_MAP_STATE, 
  JER2_ARAD_WB_DB_TDM_SAVE_CONTEXT_MAP_STATE,
  JER2_ARAD_WB_DB_TDM_UPDATE_CONTEXT_MAP_STATE,

  JER2_ARAD_WB_DB_MULTICAST_RESTORE_EGRESS_GROUPS_OPEN_DATA_STATE,
  JER2_ARAD_WB_DB_MULTICAST_SAVE_EGRESS_GROUPS_OPEN_DATA_STATE,
  JER2_ARAD_WB_DB_MULTICAST_UPDATE_EGRESS_GROUPS_OPEN_DATA_STATE,

  JER2_ARAD_WB_DB_VSI_RESTORE_ISID_STATE,
  JER2_ARAD_WB_DB_VSI_SAVE_ISID_STATE,
  JER2_ARAD_WB_DB_VSI_UPDATE_ISID_STATE,

  JER2_ARAD_SW_DB_TM_RESTORE_IS_SIMPLE_Q_TO_RATE_CLS_MODE_STATE,
  JER2_ARAD_SW_DB_TM_SAVE_IS_SIMPLE_Q_TO_RATE_CLS_MODE_STATE,
  JER2_ARAD_SW_DB_TM_UPDATE_IS_SIMPLE_Q_TO_RATE_CLS_MODE_STATE,

  JER2_ARAD_SW_DB_TM_RESTORE_QUEUE_TO_RATE_CLASS_MAPPING_REF_COUNT_STATE,
  JER2_ARAD_SW_DB_TM_SAVE_QUEUE_TO_RATE_CLASS_MAPPING_REF_COUNT_STATE,
  JER2_ARAD_SW_DB_TM_UPDATE_QUEUE_TO_RATE_CLASS_MAPPING_REF_COUNT_STATE,

  JER2_ARAD_SW_DB_TM_RESTORE_SYSPORT_TO_BASEQUEUE_STATE,
  JER2_ARAD_SW_DB_TM_SAVE_SYSPORT_TO_BASEQUEUE_STATE,
  JER2_ARAD_SW_DB_TM_UPDATE_SYSPORT_TO_BASEQUEUE_STATE,

  JER2_ARAD_WB_DB_TM_RESTORE_MODPORT_TO_SYSPORT_STATE,
  JER2_ARAD_WB_DB_TM_SAVE_MODPORT_TO_SYSPORT_STATE,
  JER2_ARAD_WB_DB_TM_UPDATE_MODPORT_TO_SYSPORT_STATE,

  JER2_ARAD_EGR_PROG_EDITOR_ALLOCATE_PROGRAM_AND_ITS_LFEMS,
  JER2_ARAD_EGR_PROG_EDITOR_ALLOCATE_PROGRAMS_AND_LFEMS_ALLOC,
  JER2_ARAD_EGR_PROG_EDITOR_PROGRAMS_USAGE_INIT,
  JER2_ARAD_SW_DB_SRC_BIND_INITIALIZE,
  JER2_ARAD_SW_DB_REASSEMBLY_CONTEXT_INITIALIZE,

  JER2_ARAD_NOF_PROCS
} JER2_ARAD_PROCS;


/*
 * Procedure identifiers.
 * }

 */

/*
 * Error codes
 * {
 */
typedef enum JER2_ARAD_ERR_LIST
{
/*
 * Start Error number for specific errors in the JER2_ARAD driver.
 */
  JER2_ARAD_START_ERR_LIST_NUMBER      = DNX_SAND_JER2_ARAD_START_ERR_NUMBER,
  JER2_ARAD_DO_NO_USE_0001,
  JER2_ARAD_DO_NO_USE_0002,
  JER2_ARAD_REGS_NOT_INITIALIZED,
  JER2_ARAD_TBLS_NOT_INITIALIZED,
  JER2_ARAD_FLD_OUT_OF_RANGE,
  JER2_ARAD_REG_ACCESS_ERR,
/*
 * Procedure jer2_arad_get_err_text() reports:
 * Input variable 'err_id' out of range (unknown value).
 */
  JER2_ARAD_GET_ERR_TEXT_001,
/*
 * Procedure jer2_arad_proc_id_to_string() reports:
 * Input variable 'proc_id' out of range (unknown value).
 */
  JER2_ARAD_PROC_ID_TO_STRING_001,

  /*
   divide by zero error
  */
  JER2_ARAD_DIVISION_BY_ZERO_ERR,

  /*
    The weight value is out of range
   */
  JER2_ARAD_ING_SCH_WEIGHT_OUT_OF_RANGE_ERR,
  /*
    Max Credit (max_burst) Value is out of range
   */
  JER2_ARAD_ING_SCH_MAX_CREDIT_OUT_OF_RANGE_ERR,
  /*
    Id of context in the context array of the mesh info structure is out of range
   */
  JER2_ARAD_ING_SCH_MESH_ID_OUT_OF_RANGE_ERR,
  /*
    nof_entries of the mesh info structure is out of range
   */
  JER2_ARAD_ING_SCH_MESH_NOF_ENTRIES_OUT_OF_RANGE_ERR,
  /*
    exact cal value calculation exceeds maximum cal value
  */
  JER2_ARAD_ING_SCH_EXACT_CAL_LARGER_THAN_MAXIMUM_VALUE_ERR,
  /*
    Arad ITM credit request hungry threshold is out of range
   */
  JER2_ARAD_ITM_CR_REQUEST_HUNGRY_TH_OUT_OF_RANGE_ERR,
  /*
    Arad ITM credit request hungry multiplier is out of range
   */
  JER2_ARAD_ITM_CR_REQUEST_HUNGRY_TH_MULTIPLIER_OUT_OF_RANGE_ERR,
  /*
    Arad ITM credit request satisfied (man & exp) threshold is out of range
  */
  JER2_ARAD_ITM_CR_REQUEST_SATISFIED_TH_OUT_OF_RANGE_ERR,
  /*
    Arad ITM credit request satisfied empty queues threshold is out of range
  */
  JER2_ARAD_ITM_CR_REQUEST_SATISFIED_EMPTY_Q_TH_OUT_OF_RANGE_ERR,
  /*
    Arad ITM credit request watchdog threshold is out of range
  */
  JER2_ARAD_ITM_CR_REQUEST_WD_TH_OUT_OF_RANGE_ERR,
  /*
    Arad ITM credit request OCB only configuration
  */
  JER2_ARAD_ITM_CR_REQUEST_OCB_ONLY_ERR,
  /*
    Arad ITM credit discount value is out of range
  */
  JER2_ARAD_ITM_CR_DISCOUNT_OUT_OF_RANGE_ERR,
  /*
    Arad ITM credit class index is out of range
  */
  JER2_ARAD_ITM_CR_CLS_OUT_OF_RANGE_ERR,
  /*
    Arad ITM wred exponential weight parameter is out of range
  */
  JER2_ARAD_ITM_WRED_EXP_WT_PARAMETER_OUT_OF_RANGE_ERR,
  JER2_ARAD_ITM_VSQ_CATEGORY_END_OUT_OF_ORDER_ERR,
  JER2_ARAD_ITM_VSQ_CATEGORY_END_OUT_OF_RANGE_ERR,
  /*
    Arad ITM admit test index is out of range
  */
  JER2_ARAD_ITM_ADMT_TEST_NDX_OUT_OF_RANGE_ERR,
  JER2_ARAD_ITM_IPS_QT_RNG_OUT_OF_RANGE_ERR,
  JER2_ARAD_ITM_QT_RT_CLS_RNG_OUT_OF_RANGE_ERR,
  JER2_ARAD_DRPP_OUT_OF_RANGE_ERR,
  JER2_ARAD_ITM_ADMT_TEST_ID_OUT_OF_RANGE_ERR,


  JER2_ARAD_ITM_WRED_MIN_TH_HIGHER_THEN_MAX_TH_ERR,
  JER2_ARAD_ITM_WRED_MIN_AVRG_TH_OUT_OF_RANGE_ERR,
  JER2_ARAD_ITM_WRED_MAX_AVRG_TH_OUT_OF_RANGE_ERR,
  JER2_ARAD_ITM_WRED_PROB_OUT_OF_RANGE_ERR,
  JER2_ARAD_ITM_WRED_MAX_PACKET_SIZE_OUT_OF_RANGE_ERR,
  JER2_ARAD_QUEUE_ID_OUT_OF_RANGE_ERR,
  JER2_ARAD_ITM_VSQ_GROUP_ID_OUT_OF_RANGE_ERR,
  JER2_ARAD_ITM_VSQ_NDX_OUT_OF_RANGE_ERR,
  JER2_ARAD_ITM_VSQ_QT_RT_OUT_OF_RANGE_ERR,
  JER2_ARAD_ITM_VSQ_WRED_EXP_WT_PARAMETER_OUT_OF_RANGE_ERR,
  JER2_ARAD_VSQ_FC_PARAMETER_OUT_OF_RANGE_ERR,
  JER2_ARAD_ITM_VSQ_MAX_INST_Q_SIZ_PARAMETER_OUT_OF_RANGE_ERR,
  JER2_ARAD_ITM_VSQ_WRED_MIN_TH_HIGHER_THEN_MAX_TH_ERR,
  JER2_ARAD_ITM_VSQ_MIN_AVRG_TH_OUT_OF_RANGE_ERR,
  JER2_ARAD_ITM_VSQ_MAX_AVRG_TH_OUT_OF_RANGE_ERR,
  JER2_ARAD_ITM_VSQ_LBL_IN_STAT_TAG_LSB_LARGER_THAN_MSB_ERR,
  JER2_ARAD_ITM_VSQ_LBL_OUT_OF_RANGE_ERR,
  JER2_ARAD_ITM_VSQ_LBL_DP_LSB_OUT_OF_RANGE_ERR,
  JER2_ARAD_ITM_VSQ_VSQ_INDEX_OUT_OF_RANGE,
  JER2_ARAD_SIGNATURE_OUT_OF_RANGE_ERR,
  JER2_ARAD_ITM_ING_SHAPE_SCH_PORT_OUT_OF_RANGE_ERR,
  JER2_ARAD_ITM_ING_SHAPE_SCH_RATE_OUT_OF_RANGE_ERR,
  JER2_ARAD_QUEUE_NUM_OUT_OF_RANGE_ERR,
  JER2_ARAD_MULTI_FABRIC_QUEUE_ORDER_ERR,
  JER2_ARAD_ITM_ING_SHAPE_Q_LOW_ABOVE_HIGH_NUM_OUT_OF_RANGE_ERR,
  JER2_ARAD_ITM_PRIORITY_MAP_NDX_OUT_OF_RANGE_ERR,
  JER2_ARAD_ITM_PRIORITY_MAP_SEGMENT_NDX_OUT_OF_RANGE_ERR,
  JER2_ARAD_ITM_SYS_RED_QUEUE_TH_OUT_OF_RANGE_ERR,
  JER2_ARAD_ITM_SYS_RED_DRP_PROB_INDEX_OUT_OF_RANGE_ERR,
  JER2_ARAD_ITM_SYS_RED_EG_INFO_AGING_TIMER_OUT_OF_RANGE_ERR,
  JER2_ARAD_ITM_SYS_RED_GLOB_RCS_RNG_THS_OUT_OF_RANGE_ERR,
  JER2_ARAD_ITM_SYS_RED_GLOB_RCS_THS_OUT_OF_RANGE_ERR,
  JER2_ARAD_ITM_SYS_RED_GLOB_RCS_RNG_VALS_OUT_OF_RANGE_ERR,
  JER2_ARAD_ITM_GLOB_RCS_FC_SIZE_OUT_OF_RANGE_ERR,
  JER2_ARAD_ITM_GLOB_RCS_DROP_SIZE_OUT_OF_RANGE_ERR,
  JER2_ARAD_SNOOP_COMMAND_INDEX_OUT_OF_RANGE_ERR,
  JER2_ARAD_IPQ_EXPLICIT_MAPPING_MODE_BASE_QUEUE_ID_OUT_OF_RANGE_ERR,
  JER2_ARAD_IPQ_TRAFFIC_CLASS_MAP_TR_CLS_OUT_OF_RANGE_ERR,
  JER2_ARAD_IPQ_DESTINATION_ID_PACKETS_BASE_DEST_NDX_OUT_OF_RANGE_ERR,
  JER2_ARAD_IPQ_DESTINATION_ID_PACKETS_BASE_BASE_QUEUE_OUT_OF_RANGE_ERR,
  JER2_ARAD_IPQ_DESTINATION_ID_PACKETS_CONFIG_ERR,
  JER2_ARAD_IPQ_INVALID_TC_PROFILE_ERR,
  JER2_ARAD_IPQ_INVALID_FLOW_ID_ERR,
  JER2_ARAD_IPQ_INVALID_SYS_PORT_ERR,
  JER2_ARAD_IPQ_INVALID_TC_ERR,
  JER2_ARAD_PORTS_MIRROR_PORT_INDEX_OUT_OF_RANGE_ERR,
  JER2_ARAD_PORTS_MIRROR_DROP_PRCDNC_OUT_OF_RANGE_ERR,
  JER2_ARAD_PORTS_MIRROR_TR_CLS_OUT_OF_RANGE_ERR,
  JER2_ARAD_SYS_PHYSICAL_PORT_NDX_OUT_OF_RANGE_ERR,
  JER2_ARAD_IPQ_K_QUEUE_INDEX_OUT_OF_RANGE_ERR,
  JER2_ARAD_CORE_INDEX_OUT_OF_RANGE_ERR,
  JER2_ARAD_FLOW_ID_OUT_OF_RANGE_ERR,
  JER2_ARAD_IPQ_BASE_FLOW_FOR_INTERDIGIT_QUEUE_QUARTET_TOO_LOW_ERR,
  JER2_ARAD_IPQ_BASE_FLOW_ALREADY_MAPPED_BY_PREVIOUS_QUEUE_QUARTET_ERR,
  JER2_ARAD_IPQ_BASE_FLOW_QUARTET_NOT_EVEN_ERR,
  JER2_ARAD_IPQ_BASE_FLOW_QUARTET_NOT_MULTIPLY_OF_FOUR_ERR,
  JER2_ARAD_MULT_FABRIC_ILLEGAL_MULTICAST_CLASS_ERR,
  JER2_ARAD_MULT_FABRIC_ILLEGAL_NUMBER_OF_QUEUE_ERR,
  JER2_ARAD_MULT_FABRIC_ILLEGAL_CONF_ERR,
  JER2_ARAD_MULT_FABRIC_ILLEGAL_NOF_LINKS,
  JER2_ARAD_MULT_EG_ILLEGAL_GROUP_RANG_CONFIG_ERR,
  JER2_ARAD_MULT_ILLEGAL_MULT_ID_ERR,
  JER2_ARAD_MULT_EG_ILLEGAL_NOF_REPLICATIONS_CONFIG_ERR,
  JER2_ARAD_MULT_EG_MULTICAST_ID_NOT_IN_VLAN_MEMBERSHIP_RNG_ERR,
  JER2_ARAD_MULT_EG_MULTICAST_ID_IS_IN_VLAN_MEMBERSHIP_RNG_ERR,
  JER2_ARAD_MULT_EG_ILLEGAL_VLAN_PTR_ENTRY_IN_LINK_LIST_ERR,
  JER2_ARAD_MULT_ENTRY_DOES_NOT_EXIST,
  JER2_ARAD_MULT_MC_GROUP_REAL_SIZE_LARGER_THAN_GIVEN_SIZE_ERR,
  JER2_ARAD_MULT_LL_ILLEGAL_ENTRY_FOR_RELOCATION_ERR,
  JER2_ARAD_MULT_EG_EXPECTED_A_TDM_REPLICATION_ERR,
  JER2_ARAD_MULT_EG_ILLEGAL_PORT,
  JER2_ARAD_MULT_MC_ID_OUT_OF_RANGE_ERR,
  JER2_ARAD_MULT_REMOVE_ELEMENT_DID_NOT_SUCCEED_ERROR,
  JER2_ARAD_PACKET_SIZE_OUT_OF_RANGE_ERR,
  JER2_ARAD_SRD_REG_ADDR_OUT_OF_RANGE_ERR,
  JER2_ARAD_SRD_EJER2_ARAD_TIMEOUT_ERR,
  JER2_ARAD_DIAG_MBIST_POLL_TIMEOUT_ERR,
  JER2_ARAD_SRD_LLA_EJER2_ARAD_CMD_READ_ERR,
  JER2_ARAD_SRD_LANE_ID_OUT_OF_RANGE_ERR,
  JER2_ARAD_SRD_STAR_ID_OUT_OF_RANGE_ERR,
  JER2_ARAD_SRD_QRTT_ID_OUT_OF_RANGE_ERR,
  JER2_ARAD_SRD_LANE_LOOPBACK_MODE_OUT_OF_RANGE_ERR,
  JER2_ARAD_SRD_LANE_EQ_MODE_OUT_OF_RANGE_ERR,
  JER2_ARAD_SRD_LANE_STATE_OUT_OF_RANGE_ERR,
  JER2_ARAD_SRD_RATE_DIVISOR_OUT_OF_RANGE_ERR,
  JER2_ARAD_SRD_AMP_VAL_OUT_OF_RANGE_ERR,
  JER2_ARAD_SRD_MEDIA_TYPE_OUT_OF_RANGE_ERR,
  JER2_ARAD_SRD_EXPLCT_PRE_OUT_OF_RANGE_ERR,
  JER2_ARAD_SRD_EXPLCT_POST_OUT_OF_RANGE_ERR,
  JER2_ARAD_SRD_EXPLCT_SWING_OUT_OF_RANGE_ERR,
  JER2_ARAD_SRD_ATTEN_OUT_OF_RANGE_ERR,
  JER2_ARAD_SRD_MAIN_VAL_OUT_OF_RANGE_ERR,
  JER2_ARAD_SRD_PRE_EMPHASIS_VAL_OUT_OF_RANGE_ERR,
  JER2_ARAD_SRD_POST_EMPHASIS_VAL_OUT_OF_RANGE_ERR,
  JER2_ARAD_SRD_UNEXPECTED_LOOPBACK_MODE_ERR,
  JER2_ARAD_SRD_MORE_THAN_ONE_LOOPBACK_MODE_DETECTED_ERR,
  JER2_ARAD_SRD_LOOPBACK_ENABLED_BUT_NO_MODE_ERR,
  JER2_ARAD_SRD_EQ_MORE_THAN_ONE_MODE_DETECTED_ERR,
  JER2_ARAD_SRD_UNEXPECTED_LANE_EQ_MODE_ERR,
  JER2_ARAD_SRD_CMU_ELEMENT_OUT_OF_RANGE_ERR,
  JER2_ARAD_SRD_LANE_ELEMENT_OUT_OF_RANGE_ERR,
  JER2_ARAD_SRD_EYE_SCAN_RESOLUTION_OUT_OF_RANGE_ERR,
  JER2_ARAD_SRD_EYE_SCAN_PRBS_DURATION_MIN_SEC_OUT_OF_RANGE_ERR,
  JER2_ARAD_SRD_EYE_SCAN_RANGE_OUT_OF_RANGE_ERR,
  JER2_ARAD_SRD_ELEMENT_OUT_OF_RANGE_ERR,
  JER2_ARAD_SCH_INVALID_SIMPLE_FLOW_ID_ERR,
  JER2_ARAD_SCH_INVALID_FLOW_ID_ERR,
  JER2_ARAD_SCH_INVALID_SE_ID_ERR,
  JER2_ARAD_SCH_INVALID_PORT_ID_ERR,
  JER2_ARAD_SCH_INVALID_SE_HR_ID_ERR,
  JER2_ARAD_SCH_INVALID_K_FLOW_ID_ERR,
  JER2_ARAD_SCH_INVALID_QUARTET_ID_ERR,
  JER2_ARAD_SCH_AGGR_SE_AND_FLOW_ID_MISMATCH_ERR,
  JER2_ARAD_SCH_CL_ID_OUT_OF_RANGE_ERR,
  JER2_ARAD_SCH_CL_CLASS_MODE_OUT_OF_RANGE_ERR,
  JER2_ARAD_SCH_DESCRETE_WEIGHT_OUT_OF_RANGE_ERR,
  JER2_ARAD_SCH_CL_CLASS_WEIGHTS_MODE_OUT_OF_RANGE_ERR,
  JER2_ARAD_SCH_CL_ENHANCED_MODE_OUT_OF_RANGE_ERR,
  JER2_ARAD_SCH_DEVICE_IF_WEIGHT_OUT_OF_RANGE_ERR,
  JER2_ARAD_SCH_FABRIC_LINK_ID_OUT_OF_RANGE_ERR,
  JER2_ARAD_SCH_FLOW_HR_AND_SCHEDULER_MODE_MISMATCH_ERR,
  JER2_ARAD_SCH_FLOW_HR_CLASS_OUT_OF_RANGE_ERR,
  JER2_ARAD_SCH_FLOW_ID_IS_SECOND_SUB_FLOW_ERR,
  JER2_ARAD_SCH_FLOW_STATUS_NOT_ON_OFF_ERR,
  JER2_ARAD_SCH_FLOW_TO_FIP_SECOND_QUARTET_MISMATCH_ERR,
  JER2_ARAD_SCH_FLOW_TO_Q_INVALID_GLOBAL_CONF_ERR,
  JER2_ARAD_SCH_FLOW_TO_Q_NOF_QUARTETS_MISMATCH_ERR,
  JER2_ARAD_SCH_FLOW_TO_Q_ODD_EVEN_IS_FALSE_ERR,
  JER2_ARAD_SCH_FLOW_TYPE_UNDEFINED_ERR,
  JER2_ARAD_SCH_GAP_IN_SUB_FLOW_ERR,
  JER2_ARAD_SCH_GROUP_OUT_OF_RANGE_ERR,
  JER2_ARAD_SCH_GRP_AND_PORT_RATE_MISMATCH_ERR,
  JER2_ARAD_SCH_NOF_CLASS_TYPES_OUT_OF_RANGE_ERR,
  JER2_ARAD_SCH_HP_CLASS_IDX_OUT_OF_RANGE_ERR,
  JER2_ARAD_SCH_HP_CLASS_NOT_AVAILABLE_ERR,
  JER2_ARAD_SCH_HP_CLASS_OUT_OF_RANGE_ERR,
  JER2_ARAD_SCH_HP_CLASS_VAL_INVALID_ERR,
  JER2_ARAD_SCH_HR_MODE_OUT_OF_RANGE_ERR,
  JER2_ARAD_SCH_CLCONFIG_OUT_OF_RANGE_ERR,
  JER2_ARAD_SCH_ENH_MODE_OUT_OF_RANGE_ERR,
  JER2_ARAD_SCH_INTERFACE_IS_SINGLE_PORT_ERR,
  JER2_ARAD_SCH_INVALID_IF_TYPE_ERR,
  JER2_ARAD_SCH_INTERFACE_RATE_OUT_OF_RANGE_ERR,
  JER2_ARAD_SCH_INTERFACE_WEIGHT_INDEX_OUT_OF_RANGE_ERR,
  JER2_ARAD_SCH_HR_MODE_INVALID_ERR,
  JER2_ARAD_INTERFACE_TYPE_OUT_OF_RANGE_ERR,
  JER2_ARAD_INTERFACE_INDEX_OUT_OF_RANGE_ERR,
  JER2_ARAD_INVALID_CH_NIF_INDEX_ERR,
  JER2_ARAD_INVALID_PORT_NIF_INDEX_ERR,
  JER2_ARAD_TCG_OUT_OF_RANGE_ERR,
  JER2_ARAD_SCH_TCG_WEIGHT_OUT_OF_RANGE_ERR,
  JER2_ARAD_EGQ_TCG_WEIGHT_OUT_OF_RANGE_ERR,
  JER2_ARAD_SCH_INVALID_PORT_GROUP_ERR,
  JER2_ARAD_SCH_SE_ID_AND_TYPE_MISMATCH_ERR,
  JER2_ARAD_SCH_SE_PORT_RATE_OUT_OF_RANGE_ERR,
  JER2_ARAD_SCH_SE_PORT_SE_TYPE_NOT_HR_ERR,
  JER2_ARAD_SCH_SE_STATE_OUT_OF_RANGE_ERR,
  JER2_ARAD_SCH_SE_TYPE_SE_CONFIG_MISMATCH_ERR,
  JER2_ARAD_SCH_SE_TYPE_UNDEFINED_ERR,
  JER2_ARAD_SCH_SLOW_RATE_OUT_OF_RANGE_ERR,
  JER2_ARAD_SCH_SUB_FLOW_AND_SCHEDULER_MODE_MISMATCH_ERR,
  JER2_ARAD_SCH_SUB_FLOW_AND_SCHEDULER_TYPE_MISMATCH_ERR,
  JER2_ARAD_SCH_SUB_FLOW_ATTACHED_TO_DISABLED_SCHEDULER_ERR,
  JER2_ARAD_SCH_SUB_FLOW_CLASS_OUT_OF_RANGE_ERR,
  JER2_ARAD_SCH_SUB_FLOW_ENHANCED_SP_MODE_MISMATCH_ERR,
  JER2_ARAD_SCH_SUB_FLOW_ID_MISMATCH_WITH_FLOW_ID_ERR,
  JER2_ARAD_SCH_SUB_FLOW_SE_TYPE_OUT_OF_RANGE_ERR,
  JER2_ARAD_SCH_SUB_FLOW_WEIGHT_OUT_OF_RANGE_ERR,
  JER2_ARAD_FAP_PORT_ID_INVALID_ERR,
  JER2_ARAD_EGR_THRESH_TYPE_OUT_OF_RANGE_ERR,
  JER2_ARAD_EGR_Q_PRIO_OUT_OF_RANGE_ERR,
  JER2_ARAD_TCG_NOT_SUPPORTED_ERR,
  JER2_ARAD_TCG_SINGLE_MEMBER_ERR,
  JER2_ARAD_REGS_FIELD_VAL_OUT_OF_RANGE_ERR,
  JER2_ARAD_DROP_PRECEDENCE_OUT_OF_RANGE_ERR,
  JER2_ARAD_TRAFFIC_CLASS_OUT_OF_RANGE_ERR,
  JER2_ARAD_REGS_DEFAULT_INSTANCE_FOR_MULTI_INST_BLOCK_ERR,
  JER2_ARAD_REGS_NON_DEFAULT_INSTANCE_FOR_SINGLE_INST_BLOCK_ERR,
  JER2_ARAD_EGR_MANTISSA_OUT_OF_RANGE_ERR,
  JER2_ARAD_EGR_EXPONENT_OUT_OF_RANGE_ERR,
  JER2_ARAD_EGR_MNT_EXP_FLD_OUT_OF_RANGE_ERR,
  JER2_ARAD_NOT_A_CHANNELIZED_IF_ERR,
  JER2_ARAD_EGR_MCI_PRIO_OUT_OF_RANGE_ERR,
  JER2_ARAD_EGR_MCI_PRIO_AND_ID_MISMATCH_ERR,
  JER2_ARAD_EGR_MCI_ERP_AND_MCI_ENABLE_ERR,
  JER2_ARAD_EGR_OFP_CHNIF_PRIO_OUT_OF_RANGE_ERR,
  JER2_ARAD_EGR_OFP_SCH_WFQ_WEIGHT_OUT_OF_RANGE_ERR,
  JER2_ARAD_EGR_Q_PRIO_MAPPING_TYPE_OUT_OF_RANGE_ERR,
  JER2_ARAD_EGR_QDCT_PD_VALUE_OUT_OF_RANGE,
  JER2_ARAD_PORTS_INVALID_ECI_PORT_IDX_ERR,
  JER2_ARAD_NIF_INDEX_OUT_OF_RANGE_ERR,
  JER2_ARAD_NIF_SGMII_INTERFACE_NOF_SINGLE_RATE_ERR,
  JER2_ARAD_NIF_MAL_INDEX_OUT_OF_RANGE_ERR,
  JER2_ARAD_NIF_DIRECTION_OUT_OF_RANGE_ERR,
  JER2_ARAD_NIF_INVALID_TYPE_ERR,
  JER2_ARAD_NIF_IPG_SIZE_OUT_OF_RANGE_ERR,
  JER2_ARAD_NIF_PREAMBLE_AND_BCT_CONF_CONFLICT_ERR,
  JER2_ARAD_NIF_BCT_SIZE_OUT_OF_RANGE_ERR,
  JER2_ARAD_NIF_BCT_CHANNEL_NDX_OUT_OF_RANGE_ERR,
  JER2_ARAD_NIF_MAL_INVALID_CONFIG_STATUS_ON_ERR,
  JER2_ARAD_NIF_TWO_LAST_MALS_SERDES_OVERLAP_ERR,
  JER2_ARAD_NIF_INCOMPATIBLE_TYPE_ERR,
  JER2_ARAD_NIF_MDIO_LESS_THEN_ONE_WORD_DATA_ERR,
  JER2_ARAD_NIF_COUNTER_OUT_OF_RANGE_ERR,
  JER2_ARAD_PORT_CONTROL_PCS_OUT_OF_RANGE_ERR,             
  JER2_ARAD_PORT_CONTROL_PCS_INNER_LINK_OUT_OF_RANGE_ERR, 
  JER2_ARAD_PORT_CONTROL_PCS_LINK_OUT_OF_RANGE_ERR,         
  JER2_ARAD_PORT_CONTROL_PCS_GET_ENCODING_TYPE_OUT_OF_RANGE_ERR,
  JER2_ARAD_PORT_UNOCCUPIED_INTERFACE_FOR_ERP_ERR,
  JER2_ARAD_NIF_ILKN_ID_OUT_OF_RANGE_ERR,
  JER2_ARAD_IPQ_NIF_ID_NOT_FIRST_IN_MAL_ERR,
  JER2_ARAD_CONNECTION_DIRECTION_OUT_OF_RANGE_ERR,
  JER2_ARAD_DBG_FORCE_MODE_OUT_OF_RANGE_ERR,
  JER2_ARAD_DBG_Q_FLUSH_ALL_TIMEOUT_ERR,
  JER2_ARAD_DBG_Q_FLUSH_MODE_OUT_OF_RANGE_ERR,
  JER2_ARAD_DBG_FORCE_MODE_FLD_OUT_OF_RANGE_ERR,
  JER2_ARAD_FABRIC_ILLEGAL_CONNECT_MODE_FE_ERR,
  JER2_ARAD_FABRIC_FAP20_ID_OUT_OF_RANGE_ERR,
  JER2_ARAD_FC_INGR_NOF_GEN_NIF_LL_OUT_OF_RANGE_ERR,
  JER2_ARAD_FC_INGR_NOF_GEN_NIF_CB_OUT_OF_RANGE_ERR,
  JER2_ARAD_FC_NIF_CB_CLASSES_OUT_OF_RANGE_ERR,
  JER2_ARAD_FC_NIF_CB_SGMII_INVALID_CONF_ERR,
  JER2_ARAD_FC_RCY_CONNECTION_CLS_OUT_OF_RANGE_ERR,
  JER2_ARAD_FC_NOF_RCY_OFP_HANDLERS_OUT_OF_RANGE_ERR,
  JER2_ARAD_FC_RCY_ON_GLB_RCS_MODE_OUT_OF_RANGE_ERR,
  JER2_ARAD_FC_OOB_ID_OUT_OF_RANGE_ERR,
  JER2_ARAD_FC_OOB_CAL_REP_OF_RANGE_ERR,
  JER2_ARAD_FC_OOB_CAL_LEN_OF_RANGE_ERR,
  JER2_ARAD_FC_OOB_CAL_SRC_TYPE_INVALID_ERR,
  JER2_ARAD_FC_OOB_CAL_SRC_ID_OF_RANGE_ERR,
  JER2_ARAD_FC_OOB_CAL_DEST_TYPE_INVALID_ERR,
  JER2_ARAD_FC_OOB_CAL_EXCESSIVE_NOF_SCH_OFP_HRS_ERR,
  JER2_ARAD_FC_OOB_CAL_DEST_ID_OF_RANGE_ERR,
  JER2_ARAD_FC_ILKN_RETRANSMIT_CAL_LEN_RANGE_ERR,
  JER2_ARAD_FC_ILKN_RETRANSMIT_CAL_RX_CFG_ERR,
  JER2_ARAD_CNM_ILLEGAL_CP_QUEUE_INDEX,
  JER2_ARAD_CNM_ILLEGAL_CP_PROFILE_INDEX,
  JER2_ARAD_CNM_ILLEGAL_CNM_GEN_MODE,
  JER2_ARAD_CNM_ILLEGAL_CP_QUEUE_SET,
  JER2_ARAD_CNM_ILLEGAL_CP_QUEUE_RANGE,
  JER2_ARAD_CNM_CP_OPTIONS_GET_UNSAFE,
  JER2_ARAD_CNM_CP_OPTIONS_SET_UNSAFE,
  JER2_ARAD_CNM_CP_OPTIONS_GET,
  JER2_ARAD_CNM_CP_OPTIONS_SET,
  JER2_ARAD_IHP_STAG_OFFSET_OUT_OF_RANGE_ERR,
  JER2_ARAD_IHP_TMLAG_OFFSET_BASE_OUT_OF_RANGE_ERR,
  JER2_ARAD_IHP_TMLAG_OFFSET_OUT_OF_RANGE_ERR,
  JER2_ARAD_PORT_TYPE_OUT_OF_RANGE_ERR,
  JER2_ARAD_IHP_SOP2HEADER_OFFSET_OUT_OF_RANGE_ERR,
  JER2_ARAD_IHP_STRIP_FROM_SOP_OUT_OF_RANGE_ERR,
  JER2_ARAD_TBL_RANGE_OUT_OF_LIMIT_ERR,
  JER2_ARAD_REVISION_SUB_TYPE_OUT_OF_LIMIT_ERR,
  JER2_ARAD_CHIP_TYPE_UNKNOWN_ERR,
  JER2_ARAD_CREDIT_SIZE_OUT_OF_RANGE_ERR,
  JER2_ARAD_CREDIT_TYPE_INVALID_ERR,
  JER2_ARAD_FAP_FABRIC_ID_OUT_OF_RANGE_ERR,
  JER2_ARAD_PP_ENABLE_NOT_SUPPORTED_ERR,
  JER2_ARAD_SYSTEM_PHYSICAL_PORT_OUT_OF_RANGE_ERR,
  JER2_ARAD_SYSTEM_PORT_OUT_OF_RANGE_ERR,
  JER2_ARAD_CUD_ID_OUT_OF_RANGE_ERR,
  JER2_ARAD_DEVICE_ID_OUT_OF_RANGE_ERR,
  JER2_ARAD_PORT_DIRECTION_OUT_OF_RANGE_ERR,
  JER2_ARAD_IF_CHANNEL_ID_OUT_OF_RANGE_ERR,
  JER2_ARAD_PORT_LAG_ID_OUT_OF_RANGE_ERR,
  JER2_ARAD_PORT_LAG_NOF_MEMBERS_OUT_OF_RANGE_ERR,
  JER2_ARAD_PORT_LAG_SYS_PORT_ALREADY_MEMBER_ERR,
  JER2_ARAD_PORT_LAG_LB_KEY_OUT_OF_RANGE,
  JER2_ARAD_DEVICE_ID_ABOVE_OUT_OF_RANGE_ERR,
  JER2_ARAD_PORTS_OTMH_OUTLIF_EXT_PERMISSION_ERR,
  JER2_ARAD_CELL_VARIABLE_IN_FAP20_SYSTEM_ERR,
  JER2_ARAD_CELL_VAR_SIZE_IN_FE200_SYSTEM_ERR,
  JER2_ARAD_HW_DRAM_CONF_LEN_OUT_OF_RANGE_ERR,
  JER2_ARAD_HW_DRAM_NOF_INTERFACES_OUT_OF_RANGE_ERR,
  JER2_ARAD_HW_QDR_PROTECT_TYPE_INVALID_ERR,
  JER2_ARAD_HW_QDR_SIZE_OUT_OF_RANGE_ERR,
  JER2_ARAD_MGMT_EGQ_INIT_FAILS_ERR,
  JER2_ARAD_NIF_NOF_PORTS_IN_FAT_PIPE_OUT_OF_RANGE_ERR,
  JER2_ARAD_LAG_GROUP_ID_OUT_OF_RANGE_ERR,
  JER2_ARAD_LAG_ENTRY_ID_OUT_OF_RANGE_ERR,
  JER2_ARAD_DEST_TYPE_OUT_OF_RANGE_ERR,
  JER2_ARAD_DEST_SYS_PORT_TYPE_OUT_OF_RANGE_ERR,
  JER2_ARAD_PORT_HEADER_TYPE_OUT_OF_RANGE_ERR,
  JER2_ARAD_FAP_INTERFACE_AND_PORT_TYPE_MISMATCH_ERR,
  JER2_ARAD_OUTBND_MIRR_IFP_NOT_MAPPED_TO_RCY_IF_ERR,
  JER2_ARAD_ITM_DRAM_BUF_SIZE_OUT_OF_RANGE_ERR,
  JER2_ARAD_AQFM_CREDIT_SOURCE_ID_ERR,
  JER2_ARAD_AQFM_SCH_SUB_FLOW_TYPE_OUT_OF_RANGE_ERR,
  JER2_ARAD_AQFM_CANT_ALLOC_AGG_ID_ERR,
  JER2_ARAD_AQFM_INVALID_PORT_ID_ERR,
  JER2_ARAD_SRD_TX_LANE_DATA_RATE_DIVISOR_INVALID_ERR,
  JER2_ARAD_SRD_CMU_REF_CLK_OUT_OF_RANGE_ERR,
  JER2_ARAD_MGMT_DRAM_INIT_FAILS_ERR,
  JER2_ARAD_PORTS_IF_EXCEEDED_MAX_ITERATIONS_ERR,
    JER2_ARAD_PMF_CE_INTERNAL_FIELD_NOT_FOUND_ERR,
  JER2_ARAD_OFP_SHPR_MODE_OUT_OF_RANGE_ERR,
  JER2_ARAD_OFP_MAL_INDEX_MISMATCH_ERR,
  JER2_ARAD_NIF_FC_LL_MODE_OUT_OF_RANGE_ERR,
  JER2_ARAD_SRD_ENTITY_TYPE_OUT_OF_RANGE_ERR,
  JER2_ARAD_SRD_FLD_SIZE_OUT_OF_RANGE_ERR,
  JER2_ARAD_MGMT_BASIC_CONF_NULL_AT_INIT_ERR,
  JER2_ARAD_MGMT_BASIC_CONF_NOT_SUPPLIED_ERR,
  JER2_ARAD_FABRIC_CONNECT_MESH_MODE_CHANGE_ERR,
  JER2_ARAD_FABRIC_SHAPER_MODE_OUT_OF_RANGE_ERR,
  JER2_ARAD_FTMH_EXTENSION_OUT_OF_RANGE_ERR,
  JER2_ARAD_MGMT_QDR_NOT_READY_ERR,
  JER2_ARAD_MGMT_QDR_INIT_BIST_DID_NOT_FINISH_ERR,
  JER2_ARAD_MGMT_QDRC_NOT_LOCKED_ERR,
  JER2_ARAD_MGMT_QDR_TRAINING_REPLY_FAIL_ERR,
  JER2_ARAD_MGMT_INIT_INVALID_FREQUENCY,
  JER2_ARAD_MGMT_INIT_INVALID_DBUFF_SIZE_ERR,
  JER2_ARAD_MGMT_INIT_FINALIZE_ERR,
  JER2_ARAD_INIT_QDR_PLL_NOT_LOCKED_ERR,
  JER2_ARAD_INIT_DDR_PLL_NOT_LOCKED_ERR,
  JER2_ARAD_INIT_CORE_PLL_NOT_LOCKED_ERR,
  JER2_ARAD_SRD_8051_CHECKSUM_STILL_RUNNING_ERR,
  JER2_ARAD_SRD_8051_CHECKSUM_FAIL_ERR,
  JER2_ARAD_SRD_8051_CHECKSUM_DID_NOT_COMPLETE_ERR,
  JER2_ARAD_SRD_TRIM_FAILED_ERR,
  JER2_ARAD_HW_INVALID_NUMBER_OF_DRAM_INTERFACES_ERR,
  JER2_ARAD_HW_INVALID_NOF_BANKS_FOR_DRAM_TYPE_ERR,
  JER2_ARAD_HW_INVALID_NOF_BANKS_ERR,
  JER2_ARAD_HW_NOF_COLUMNS_OUT_OF_RANGE_ERR,
  JER2_ARAD_SRD_TX_TRIMMING_DID_NOT_END_ERR,
  JER2_ARAD_SRD_PLL_RESET_DID_NOT_END_ERR,
  JER2_ARAD_SRD_LN_RX_RESET_DID_NOT_END_ERR,
  JER2_ARAD_NIF_SRD_RATE_OUT_OF_RANGE_ERR,
  JER2_ARAD_NIF_MAL_IS_FABRIC_NOT_NIF_ERR,
  JER2_ARAD_STAT_NO_SUCH_COUNTER_ERR,
  JER2_ARAD_IHP_IS_RANGE_AND_BASE_Q_MISMATCH_ERR,
  JER2_ARAD_SRD_PLL_CONF_NOT_FOUND_ERR,
  JER2_ARAD_DIAG_DRAM_NDX_OUT_OF_RANGE_ERR,
  JER2_ARAD_DIAG_PATTERN_MODE_OUT_OF_RANGE_ERR,
  JER2_ARAD_DIAG_BIST_ADDRESS_ILLEGAL_RANGE_ERR,
  JER2_ARAD_DIAG_DRAM_OFFSET_ILLEGAL_RANGE_ERR,
  JER2_ARAD_DIAG_INCONSISTENT_DRAM_CONFIG_ERR,
  JER2_ARAD_DIAG_BIST_DATA_MODE_OUT_OF_RANGE_ERR,
  JER2_ARAD_DIAG_DUMP_MODE_OUT_OF_RANGE_ERR,
  JER2_ARAD_DIAG_BIST_DATA_PATERN_OUT_OF_RANGE_ERR,
  JER2_ARAD_DIAG_BIST_WRITES_PER_CYCLE_OUT_OF_RANGE_ERR,
  JER2_ARAD_DIAG_BIST_READS_PER_CYCLE_OUT_OF_RANGE_ERR,
  JER2_ARAD_DIAG_DRAM_ACCESS_TIMEOUT_ERR,
  JER2_ARAD_DIAG_DRAM_ACC_NOF_COLUMNS_INVALID_ERR,
  JER2_ARAD_SRD_ACCESS_INTERNAL_REG_WITH_EXTERNAL_CONF_ERR,
  JER2_ARAD_SRD_CMU_VER_MISMATCH_ERR,
  JER2_ARAD_DBUFF_SIZE_INVALID_ERR,
  JER2_ARAD_PKT_TX_CPU_PACKET_BYTE_SIZE_OUT_OF_RANGE_ERR,
  JER2_ARAD_PKT_ASYNC_MODE_CONFIG_ERR,
  JER2_ARAD_FAT_PIPE_NO_SEQ_HDR_ERR,
  JER2_ARAD_PORT_ID_CONSUMED_BY_FAT_PIPE_ERR,
  JER2_ARAD_FAT_PIPE_NOT_SET_ERR,
  JER2_ARAD_FAT_PIPE_MULTIPLE_PORTS_ERR,
  JER2_ARAD_FAT_PIPE_MUST_BE_FIRST_ERR,
  JER2_ARAD_FAT_PIPE_INVALID_PORT_ID_ERR,
  JER2_ARAD_MULT_VLAN_MEMB_ID_OUT_OF_RANGE_ERR,
  JER2_ARAD_TBL_PROG_IDX_OUT_OF_RANGE_ERR,
  JER2_ARAD_TBL_INSTR_IDX_OUT_OF_RANGE_ERR,
  JER2_ARAD_TBL_PRGR_COS_SET_IDX_OUT_OF_RANGE_ERR,
  JER2_ARAD_SRD_INVALID_REF_CLOCK_ERR,
  JER2_ARAD_SRD_TX_CONF_MODE_OUT_OF_RANGE_ERR,
  JER2_ARAD_SRD_INVALID_RATE_ERR,
  JER2_ARAD_SRD_MISCONFIGURED_RATE_ON_POWERUP_ERR,
  JER2_ARAD_SRD_MISCONFIGURED_RATE_ERR,
  JER2_ARAD_UNSUPPORTED_ERR,
  JER2_ARAD_UNSUPPORTED_FOR_DEVICE_ERR,
  JER2_ARAD_SRD_PLL_NOT_LOCKED_ERR,
  JER2_ARAD_SRD_INVALID_RATE_FOR_FABRIC_MAC_ERR,
  JER2_ARAD_FBR_LINK_INVALID_ERR,
  JER2_ARAD_SRD_AEQ_MODE_INVALID_ERR,
  JER2_ARAD_FAP_PORT_ID_INVALID_WHEN_FAT_PIPE_ENABLED_ERR,
  JER2_ARAD_SRD_PRBS_MODE_OUT_OF_RANGE_ERR,
  JER2_ARAD_DRAM_BUFFERS_UC_FBC_OVERFLOW_ERR,
  JER2_ARAD_DRAM_BUFFERS_FBC_OVERFLOW_ERR,
  JER2_ARAD_SCH_SLOW_RATE_INDEX_INVALID_ERR,
  JER2_ARAD_SCH_INTERNAL_SLOW_RATE_INDEX_INVALID_ERR,
  JER2_ARAD_IPQ_INVALID_QUEUE_ID_ERR,
  JER2_ARAD_NIF_SGMII_RATE_OUT_OF_RANGE_ERR,
  JER2_ARAD_FBR_LINK_ID_OUT_OF_RANGE_ERR,
  JER2_ARAD_FBR_LINK_ON_OFF_STATE_OUT_OF_RANGE_ERR,
  JER2_ARAD_NIF_1000BASE_X_INVALID_RATE_ERR,
  JER2_ARAD_CALLBACK_ALREADY_REGISTERED_ERR,
  JER2_ARAD_ITM_INGRESS_SHAPING_LOW_BELOW_HIGH_ERR,
  JER2_ARAD_BASE_Q_NOT_SET_ERR,
  JER2_ARAD_SSR_FORBIDDEN_FUNCTION_CALL_ERR,
  JER2_ARAD_SSR_INCOMPATIBLE_SRC_VERSION_ERR,
  JER2_ARAD_SW_DB_BUFF_SIZE_MISMATCH_ERR,
  JER2_ARAD_FC_INGR_GEN_GLB_HP_INVALID_ERR,
  JER2_ARAD_NIF_FC_RX_DISABLE_ERR,
  JER2_ARAD_SRD_INIT_TRIM_ERR,
  JER2_ARAD_NIF_SRD_LPBCK_MODE_INCONSISTENT_ERR,
  JER2_ARAD_INCOMPATABLE_NIF_ID_ERR,
  JER2_ARAD_PCKT_SIZE_VSC_BELOW_MIN_ERR,
  JER2_ARAD_PCKT_SIZE_FSC_BELOW_MIN_ERR,
  JER2_ARAD_PCKT_SIZE_VSC_ABOVE_MAX_ERR,
  JER2_ARAD_PCKT_SIZE_FSC_ABOVE_MAX_ERR,
  JER2_ARAD_PCKT_SIZE_MIN_EXCEEDS_MAX_ERR,
  JER2_ARAD_MIN_PCKT_SIZE_INCONSISTENT_ERR,
  JER2_ARAD_TEXT_NO_ERR_TXT_FOUND_ERR,
  JER2_ARAD_SRD_EJER2_ARAD_ACCESS_ERR,
  JER2_ARAD_ECI_ACCESS_ERR,
  JER2_ARAD_SRD_LN_TRIM_DONE_IS_DOWN_ERR,
  JER2_ARAD_SRD_LN_CLCK_RLS_IS_DOWN_ERR,
  JER2_ARAD_SRD_LN_CLCK_RXTDACDONE_IS_DOWN_ERR,
  JER2_ARAD_OP_MODE_A1_OR_BELOW_LOCAL_MISMATCH_ERR,
  JER2_ARAD_PORT_EGR_HDR_CR_DISCOUNT_TYPE_OUT_OF_RANGE_ERR,
  JER2_ARAD_Q_ID_WITH_BASE_Q_MISMATCH_ERR,
  JER2_ARAD_MEM_CORRECTION_RM_BIT_INCONSISTENT_ERR,
  JER2_ARAD_MEM_CORRECTION_ERR,
  JER2_ARAD_MEM_BIST_ERR,
  JER2_ARAD_CELL_DIFFERENT_CELL_IDENT_ERR,
  JER2_ARAD_CELL_NO_RECEIVED_CELL_ERR,
  JER2_ARAD_CELL_WRITE_OUT_OF_BOUNDARY,
  JER2_ARAD_MGMT_PCKT_SIZE_CONF_MODE_OUT_OF_RANGE_ERR,
  JER2_ARAD_MGMT_PCKT_MAX_SIZE_INTERNAL_OUT_OF_RANGE_ERROR,
  JER2_ARAD_MGMT_PCKT_MAX_SIZE_EXTERNAL_OUT_OF_RANGE_ERROR,
  JER2_ARAD_MGMT_SMS_ACTION_TIMOUT_ERR,
  JER2_ARAD_SCH_SUB_FLOW_INVALID_ERR,
  JER2_ARAD_SRD_VALIDATE_AND_RELOCK_FAILED_ERR,
  JER2_ARAD_STAG_ENABLE_MODE_OUT_OF_RANGE_ERR,
  JER2_ARAD_QUEUE_SIZE_OUT_OF_RANGE_ERR,
  JER2_ARAD_SRD_IPU_IRQ_ACK_DOWN_ERR,
  JER2_ARAD_NIF_ON_OFF_STATE_OUT_OF_RANGE_ERR,
  JER2_ARAD_NIF_SGMII_0_OFF_INVALID_ERR,
  JER2_ARAD_SRD_NOT_FABRIC_QUARTET_ERR,
  JER2_ARAD_SRD_TX_EXPLCT_FAIL_ERR,
  JER2_ARAD_SRD_TX_ATTEN_FAIL_ERR,
  JER2_ARAD_SRD_TX_ATTEN_ABOVE_3_125_FAIL_ERR,
  JER2_ARAD_MDIO_OP_TIMEOUT_ERR,
  JER2_ARAD_LBG_PATTERN_DATA_SIZE_OUT_OF_RANGE_ERR,
  JER2_ARAD_ALLOC_TO_NON_NULL_ERR,
  JER2_ARAD_DRAM_CONF_MODE_OUT_OF_RANGE_ERR,
  JER2_ARAD_CR_WD_DEL_TH_OUT_OF_RANGE,
  JER2_ARAD_SCH_FLOW_AND_SE_TYPE_MISMATCH_ERR,
  JER2_ARAD_PORT_LAG_MEMBER_ID_OUT_OF_RANGE_ERR,
  JER2_ARAD_FEATURE_NOT_SUPPORTED_AT_REVISION_ERR,
  JER2_ARAD_ITM_CR_REQ_TYPE_NDX_OUT_OF_RANGE_ERR,
  JER2_ARAD_ITM_CR_REQ_TYPE_NDX_NOT_ALLOCATED_ERR,
  JER2_ARAD_ITM_CREDIT_CLS_OUT_OF_RANGE_ERR,
  JER2_ARAD_ITM_RATE_CLS_OUT_OF_RANGE_ERR,
  JER2_ARAD_ITM_VSQ_CONNECTION_CLS_OUT_OF_RANGE_ERR,
  JER2_ARAD_ITM_VSQ_TRAFFIC_CLS_OUT_OF_RANGE_ERR,
  JER2_ARAD_UNSCHED_PRIO_OUT_OF_RANGE_ERR,
  JER2_ARAD_EGR_MC_16K_ENABLE_INCOMPATIBLE_ERR,
  JER2_ARAD_INGR_RST_INTERN_BLOCK_INIT_ERR,
  JER2_ARAD_SCH_ILLEGAL_COMPOSITE_AGGREGATE_ERR,
  JER2_ARAD_SCH_COMPOSITE_AGGREGATE_DUAL_SHAPER_ERR,
  JER2_ARAD_OFP_RATES_CHAN_ARB_INVALID_ERR,
  JER2_ARAD_OFP_RATES_TBL_NOF_ENTRIES_OUT_OF_RANGE_ERR,
  JER2_ARAD_OFP_RATES_SCH_RATE_OUT_OF_RANGE_ERR,
  JER2_ARAD_OFP_RATES_EGQ_RATE_OUT_OF_RANGE_ERR,
  JER2_ARAD_OFP_RATES_BURST_OUT_OF_RANGE_ERR,
  JER2_ARAD_OFP_RATES_PORTS_FROM_DIFFERENT_IFS_ERR,
  JER2_ARAD_OFP_RATES_CAL_LEN_INVALID_ERR,
  JER2_ARAD_OFP_RATES_SCH_CHAN_ARB_INVALID_ERR,
  JER2_ARAD_OFP_RATES_CAL_NO_SLOTS_AVAILABLE_ERR,
  JER2_ARAD_OFP_RATES_CAL_ALLOCATED_AND_REQUESTED_LEN_MISMATCH_ERR,
  JER2_ARAD_OFP_RATES_FAP_PORTS_OUT_OF_RANGE_ERR,
  JER2_ARAD_OFP_RATES_SCH_PORT_ID_OUT_OF_RANGE_ERR,
  JER2_ARAD_OFP_RATES_OFP_INDEX_MISMATCH_ERR,
  JER2_ARAD_OFP_RATES_NOF_ITERATIONS_EXCEEDS_LIMITS_ERR,
  JER2_ARAD_OFP_RATES_PORT_HAS_NO_IF_ERR,
  JER2_ARAD_OFP_RATES_CHAN_ARB_INVALID_NON_CHAN_ERR,
  JER2_ARAD_OFP_RATES_CONSECUTIVE_CREDIT_DISTRIBUTION_ERR,
  JER2_ARAD_OFP_RATES_ACTUAL_AND_EXPECTED_RATE_DIFFERENCE_ERR,
  JER2_ARAD_OFP_RATES_INVALID_PORT_ID_ERR,
  JER2_ARAD_OFP_RATES_PRIO_ID_OUT_OF_RANGE_ERR,
  JER2_ARAD_OFP_RATES_EGQ_CAL_INVALID_ERR,
  JER2_ARAD_OFP_RATES_TCG_NDX_OUT_OF_RANGE_ERR,
  JER2_ARAD_OFP_RATES_UNMAPPED_TCG_ERR,
  JER2_ARAD_NON_MESH_CONF_ERR,
  JER2_ARAD_FULL_PCKT_MODE_IN_MESH_CONF_ERR,
  JER2_ARAD_DRAM_INVALID_DRAM_TYPE_ERR,
  JER2_ARAD_FABRIC_CRC_NOT_SUPPORTED_ERR,
  JER2_ARAD_CELL_DATA_OUT_OF_RANGE_ERR,
  JER2_ARAD_CELL_FE_LOCATION_OUT_OF_RANGE_ERR,
  JER2_ARAD_CELL_PATH_LINKS_OUT_OF_RANGE_ERR,
  JER2_ARAD_MC_ID_NDX_OUT_OF_RANGE_ERR,
  JER2_ARAD_REG_ACCESS_NO_PA_ERR,
  JER2_ARAD_REG_ACCESS_NO_JER2_ARAD_ERR,
  JER2_ARAD_REG_ACCESS_UNKNOWN_DEVICE_ERR,
  JER2_ARAD_STAT_MULT_ID_UNSUPPORTED_ERR,
  JER2_ARAD_STAT_MODE_OUT_OF_RANGE_ERR,
  JER2_ARAD_SRD_LANE_AND_QRTT_ENABLE_MISMATCH_ERR,
  JER2_ARAD_NIF_MAL_SGMII_CONF_MISMATCH_ERR,
  JER2_ARAD_NIF_SPAUI_INTRLV_BCT_SIZE_ERR,
  JER2_ARAD_SRD_EYE_SCAN_CRC_NOT_FABRIC_ERR,
  JER2_ARAD_SRD_EYE_SCAN_FEC_NOT_CONFIGURED_ERR,
  JER2_ARAD_SRD_CNT_SRC_OUT_OF_RANGE_ERR,
  JER2_ARAD_SRD_TRAFFIC_SRC_OUT_OF_RANGE_ERR,
  JER2_ARAD_CORE_FREQ_OUT_OF_RANGE_ERR,
  JER2_ARAD_CR_WD_DELETE_BEFORE_STATUS_MSG_ERR,
  JER2_ARAD_CR_WD_DEL_TH_UNSUPPORTED_ERR,
  JER2_ARAD_RATE_CONF_MODE_INCONSISTENT_ERR,
  JER2_ARAD_FUNC_CALL_NO_PA_ERR,
  JER2_ARAD_FUNC_CALL_NO_JER2_ARAD_ERR,
  JER2_ARAD_FUNC_CALL_UNKNOWN_DEVICE_ERR,
  JER2_ARAD_SRD_REF_CLK_OF_RANGE_ERR,
  JER2_ARAD_NIF_SPAUI_DS_CONF_NON_DS_BUS_ERR,
  JER2_ARAD_NIF_SPAUI_DS_SOP_ODD_EVEN_ERR,
  JER2_ARAD_PORTS_NON_CH_IF_ERR,
  JER2_ARAD_DRAM_NOF_BANKS_OUT_OF_RANGE_ERR,
  JER2_ARAD_DRAM_NOF_COLS_OUT_OF_RANGE_ERR,
  JER2_ARAD_DRAM_DDR_TUNE_FAILED_ERR,
  JER2_ARAD_DRAM_DDR_TUNE_VALUES_RESTORED_ERR,
  JER2_ARAD_DRAM_CL_OUT_OF_RANGE_ERR,
  JER2_ARAD_DRAM_WL_OUT_OF_RANGE_ERR,
  JER2_ARAD_DRAM_WR_OUT_OF_RANGE_ERR,
  JER2_ARAD_DRAM_BL_OUT_OF_RANGE_ERR,
  JER2_ARAD_DRAM_RBUS_READ_ERR,
  JER2_ARAD_INTERRUPT_DEVICE_BETWEEN_ISR_TO_TCM_ERR,
  JER2_ARAD_INTERRUPT_INSUFFICIENT_MEMORY_ERR,
  JER2_ARAD_INTERRUPT_INVALID_MONITORED_CAUSE_ERR,
  JER2_ARAD_DRAM_NOF_CONF_MODE_OUT_OF_RANGE_ERR,
  JER2_ARAD_PORTS_EG_TM_PROFILE_FULL_ERR,
  JER2_ARAD_SRD_RATE_UNKNOWN_ERR,
  JER2_ARAD_DIAG_QDR_REPLY_LOCK_CANNOT_LOCK_ERR,
  JER2_ARAD_DIAG_QDR_DLL_NOT_READY_ERR,
  JER2_ARAD_DIAG_QDR_TRAINING_FAIL_ERR,
  JER2_ARAD_DIAG_QDR_CANNOT_ACCESS_ERR,
  JER2_ARAD_DEPRICATED_ERR,
  JER2_ARAD_DBG_PCM_COUNTER_NOT_EXPIRED_ERR,
  JER2_ARAD_END2END_SCHEDULER_MODE_OUT_OF_RANGE_ERR,
  JER2_ARAD_DIAGNOSTICS_COUNT_TYPE_OUT_OF_RANGE_ERR,
  JER2_ARAD_DIAGNOSTICS_PATTERN_OUT_OF_RANGE_ERR,
  JER2_ARAD_DIAGNOSTICS_SMS_OUT_OF_RANGE_ERR,
  JER2_ARAD_DIAGNOSTICS_ERR_SP_OUT_OF_RANGE_ERR,
  JER2_ARAD_DIAGNOSTICS_ERR_DP_OUT_OF_RANGE_ERR,
  JER2_ARAD_DIAGNOSTICS_ERR_RF_OUT_OF_RANGE_ERR,
  JER2_ARAD_INGR_SHP_Q_ABOVE_MAX_ERR,
  JER2_ARAD_DEBUG_RST_DOMAIN_OUT_OF_RANGE_ERR,
  JER2_ARAD_DBG_CFC_DB_CORRUPT_ERR,
  JER2_ARAD_MGMT_DRAM_INIT_RND_TRIP_FAILS_ERR,
  JER2_ARAD_API_NOT_FUNCTIONAL_ERR,
  JER2_ARAD_SRD_RATE_VCO_BELOW_MIN_ERR,
  JER2_ARAD_SRD_RATE_VCO_ABOVE_MAX_ERR,
  JER2_ARAD_DBG_STAT_IF_ENABLED_ERR,

  JER2_ARAD_GEN_NUM_CLEAR_ERR,
  JER2_ARAD_TBL_RANGE_OUT_OF_LIMIT,

  JER2_ARAD_PORT_NDX_OUT_OF_RANGE_ERR,
  JER2_ARAD_CELL_SIZE_OUT_OF_RANGE_ERR,
  JER2_ARAD_IS_TDM_OUT_OF_RANGE_ERR,
  JER2_ARAD_PROFILE_NDX_OUT_OF_RANGE_ERR,
  JER2_ARAD_SAMPLING_RATE_OUT_OF_RANGE_ERR,
  JER2_ARAD_CPQ_NDX_OUT_OF_RANGE_ERR,
  JER2_ARAD_PROCESSOR_NDX_OUT_OF_RANGE_ERR,
  JER2_ARAD_COUNTER_NDX_OUT_OF_RANGE_ERR,
  JER2_ARAD_QUEUE_NDX_OUT_OF_RANGE_ERR,
  JER2_ARAD_HDR_IN_COMPENSATION_OUT_OF_RANGE_ERR,
  JER2_ARAD_HDR_OUT_COMPENSATION_OUT_OF_RANGE_ERR,
  JER2_ARAD_ACTION_NDX_OUT_OF_RANGE_ERR,
  JER2_ARAD_D_DEST_TYPE_OUT_OF_RANGE_ERR,
  JER2_ARAD_D_DEST_VAL_OUT_OF_RANGE_ERR,
  JER2_ARAD_ITM_ENABLE_MODE_OUT_OF_RANGE_ERR,
  JER2_ARAD_ITM_OFFSET_4B_OUT_OF_RANGE_ERR,
  JER2_ARAD_ITM_GRNT_BYTES_OUT_OF_RANGE_ERR,
  JER2_ARAD_ITM_GRNT_BDS_OUT_OF_RANGE_ERR,
  JER2_ARAD_ITM_GRNT_OUT_OF_RESOURCE_ERR,
  JER2_ARAD_INGRESS_TRAFFIC_MGMT_RT_CLS4_NDX_OUT_OF_RANGE_ERR,
  JER2_ARAD_STAT_FAP20V_MODE_OUT_OF_RANGE_ERR,
  JER2_ARAD_STAT_BIILING_MODE_OUT_OF_RANGE_ERR,
  JER2_ARAD_STAT_REPORT_MODE_OUT_OF_RANGE_ERR,
  JER2_ARAD_STAT_FABRIC_MC_OUT_OF_RANGE_ERR,
  JER2_ARAD_STAT_ING_REP_MC_OUT_OF_RANGE_ERR,
  JER2_ARAD_STAT_CNM_REPORT_OUT_OF_RANGE_ERR,
  JER2_ARAD_STAT_PARITY_MODE_OUT_OF_RANGE_ERR,
  JER2_ARAD_STAT_WORD1_OUT_OF_RANGE_ERR,
  JER2_ARAD_STAT_WORD1_INVALID_ERR,
  JER2_ARAD_STAT_WORD2_INVALID_ERR,
  JER2_ARAD_STAT_WORD2_OUT_OF_RANGE_ERR,
  JER2_ARAD_STAT_EN_MODE_OUT_OF_RANGE_ERR,
  JER2_ARAD_STAT_NOF_DUPLICATIONS_OUT_OF_RANGE_ERR,
  JER2_ARAD_STAT_NOF_IDLE_CLOCKS_OUT_OF_RANGE_ERR,
  JER2_ARAD_TDM_PORT_NDX_OUT_OF_RANGE_ERR,
  JER2_ARAD_TDM_DIRECT_ROUTING_PROFILE_NDX_OUT_OF_RANGE_ERR,
  JER2_ARAD_TDM_CELL_SIZE_OUT_OF_RANGE_ERR,
  JER2_ARAD_TDM_IS_TDM_OUT_OF_RANGE_ERR,
  JER2_ARAD_TDM_DEST_IF_OUT_OF_RANGE_ERR,
  JER2_ARAD_TDM_DEST_FAP_ID_OUT_OF_RANGE_ERR,
  JER2_ARAD_TDM_MC_ID_OUT_OF_RANGE_ERR,
  JER2_ARAD_TDM_MC_USER_DEF_OUT_OF_RANGE_ERR,
  JER2_ARAD_TDM_DEST_FAP_PORT_OUT_OF_RANGE_ERR,
  JER2_ARAD_TDM_USER_DEFINE_2_OUT_OF_RANGE_ERR,
  JER2_ARAD_TDM_ACTION_ING_OUT_OF_RANGE_ERR,
  JER2_ARAD_TDM_ACTION_EG_OUT_OF_RANGE_ERR,
  JER2_ARAD_TDM_INVALID_TDM_MODE_ERR,
  JER2_ARAD_TDM_API_INVALID_PETRAB_IN_SYSTEM_ERR,
  JER2_ARAD_TDM_CUSTOMER_EMBED_IN_OPTIMIZED_MODE_ERR,
  JER2_ARAD_TDM_OUT_OF_AVAIABLE_TDM_CONTEXT_ERR,
  JER2_ARAD_CNM_Q_SET_OUT_OF_RANGE_ERR,
  JER2_ARAD_CNM_ING_VLAN_EDIT_CMD_OUT_OF_RANGE_ERR,
  JER2_ARAD_CNM_VERSION_OUT_OF_RANGE_ERR,
  JER2_ARAD_CNM_RES_V_OUT_OF_RANGE_ERR,
  JER2_ARAD_CNM_TC_OUT_OF_RANGE_ERR,
  JER2_ARAD_CNM_CP_ID_4_MSB_OUT_OF_RANGE_ERR,
  JER2_ARAD_CNM_PKT_GEN_MODE_OUT_OF_RANGE_ERR,
  JER2_ARAD_CNM_PROFILE_OUT_OF_RANGE_ERR,
  JER2_ARAD_CNT_START_Q_OUT_OF_RANGE_ERR,
  JER2_ARAD_CNT_Q_SET_SIZE_OUT_OF_RANGE_ERR,
  JER2_ARAD_CNT_SRC_TYPE_OUT_OF_RANGE_ERR,
  JER2_ARAD_CNT_MODE_STATISTICS_OUT_OF_RANGE_ERR,
  JER2_ARAD_CNT_MODE_EG_OUT_OF_RANGE_ERR,
  JER2_ARAD_CNT_STAG_LSB_OUT_OF_RANGE_ERR,
  JER2_ARAD_CNT_CACHE_LENGTH_OUT_OF_RANGE_ERR,
  JER2_ARAD_CNT_NOF_COUNTERS_OUT_OF_RANGE_ERR,
  JER2_ARAD_ACTION_SNOOP_SIZE_OUT_OF_RANGE_ERR,
  JER2_ARAD_ACTION_SNOOP_PROB_OUT_OF_RANGE_ERR,
  JER2_ARAD_ACTION_MIRROR_PROB_OUT_OF_RANGE_ERR,
  JER2_ARAD_WC_NDX_OUT_OF_RANGE_ERR,
  JER2_ARAD_PCKT_SIZE_OUT_OF_RANGE_ERR,
  JER2_ARAD_RATE_OUT_OF_RANGE_ERR,
  JER2_ARAD_ILKN_NDX_OUT_OF_RANGE_ERR,
  JER2_ARAD_CLK_NDX_OUT_OF_RANGE_ERR,
  JER2_ARAD_MODE_OUT_OF_RANGE_ERR,
  JER2_ARAD_FATP_NDX_OUT_OF_RANGE_ERR,
  JER2_ARAD_HW_MAL_NOF_ENTRIES_OUT_OF_RANGE_ERR,
  JER2_ARAD_HW_SPAUI_NOF_ENTRIES_OUT_OF_RANGE_ERR,
  JER2_ARAD_HW_GMII_NOF_ENTRIES_OUT_OF_RANGE_ERR,
  JER2_ARAD_HW_ILKN_NOF_ENTRIES_OUT_OF_RANGE_ERR,
  JER2_ARAD_HW_FATP_NOF_ENTRIES_OUT_OF_RANGE_ERR,
  JER2_ARAD_INIT_CREDIT_WORTH_OUT_OF_RANGE_ERR,
  JER2_ARAD_INIT_EXTENDED_Q_NOF_ENTRIES_OUT_OF_RANGE_ERR,
  JER2_ARAD_NIF_TYPE_OUT_OF_RANGE_ERR,
  JER2_ARAD_NIF_SIZE_OUT_OF_RANGE_ERR,
  JER2_ARAD_NIF_DIC_MODE_OUT_OF_RANGE_ERR,
  JER2_ARAD_NIF_BCT_CHANNEL_BYTE_NDX_OUT_OF_RANGE_ERR,
  JER2_ARAD_NIF_CRC_MODE_OUT_OF_RANGE_ERR,
  JER2_ARAD_NIF_FAULT_TYPE_OUT_OF_RANGE_ERR,
  JER2_ARAD_NIF_CLK_DIVIDER_OUT_OF_RANGE_ERR,
  JER2_ARAD_FC_INHERIT_OUT_OF_RANGE_ERR,
  JER2_ARAD_FC_VSQ_TRGR_OUT_OF_RANGE_ERR,
  JER2_ARAD_FC_REACT_POINT_OUT_OF_RANGE_ERR,
  JER2_ARAD_FC_PRIO_OUT_OF_RANGE_ERR,
  JER2_ARAD_FC_HANDLER_OUT_OF_RANGE_ERR,
  JER2_ARAD_FC_PRIORITY_OUT_OF_RANGE_ERR,
  JER2_ARAD_FC_OFP_ID_OUT_OF_RANGE_ERR,
  JER2_ARAD_FC_CAL_LEN_OUT_OF_RANGE_ERR,
  JER2_ARAD_FC_CAL_REPS_OUT_OF_RANGE_ERR,
  JER2_ARAD_FC_SOURCE_OUT_OF_RANGE_ERR,
  JER2_ARAD_FC_DESTINATION_OUT_OF_RANGE_ERR,
  JER2_ARAD_FC_CAL_CHANNEL_OUT_OF_RANGE_ERR,
  JER2_ARAD_FC_NOF_OFP_PRIORITIES_OUT_OF_RANGE_ERR,
  JER2_ARAD_FC_MODE_OUT_OF_RANGE_ERR,
  JER2_ARAD_FC_TRGR_NDX_OUT_OF_RANGE_ERR,
  JER2_ARAD_CPID_TC_NDX_OUT_OF_RANGE_ERR,
  JER2_ARAD_FC_SCH_OOB_RANGE_INVALID_ERR,
  JER2_ARAD_FC_SCH_OOB_WD_PERIOD_INVALID_ERR,
  JER2_ARAD_FC_CLASS_OUT_OF_RANGE_ERR,
  JER2_ARAD_FC_CAL_MODE_NDX_OUT_OF_RANGE_ERR,
  JER2_ARAD_FC_CAL_IF_NDX_OUT_OF_RANGE_ERR,
  JER2_ARAD_FC_OOB_NDX_OUT_OF_RANGE_ERR,
  JER2_ARAD_FC_NIF_OVRS_SCHEME_OUT_OF_RANGE_ERR,
  JER2_ARAD_FC_DIRECTION_NDX_OUT_OF_RANGE_ERR,
  JER2_ARAD_FC_NIF_PAUSE_QUANTA_OUT_OF_RANGE_ERR,
  JER2_ARAD_NIF_COUNTER_TYPE_OUT_OF_RANGE_ERR,
  JER2_ARAD_CODING_OUT_OF_RANGE_ERR,
  JER2_ARAD_FBR_NOF_LINKS_OUT_OF_RANGE_ERR,
  JER2_ARAD_FABRIC_SHAPER_BYTES_OUT_OF_RANGE_ERR,
  JER2_ARAD_FABRIC_SHAPER_CELLS_OUT_OF_RANGE_ERR,
  JER2_ARAD_INIT_HDR_TYPE_NOF_ENTRIES_OUT_OF_RANGE_ERR,
  JER2_ARAD_INIT_IF_MAP_NOF_ENTRIES_OUT_OF_RANGE_ERR,
  JER2_ARAD_INIT_PACKET_HDR_INFO_NOF_ENTRIES_OUT_OF_RANGE_ERR,
  JER2_ARAD_HW_MAL_NDX_OUT_OF_RANGE_ERR,
  JER2_ARAD_TC_NDX_OUT_OF_RANGE_ERR,
  JER2_ARAD_DP_NDX_OUT_OF_RANGE_ERR,
  JER2_ARAD_MAP_PROFILE_NDX_OUT_OF_RANGE_ERR,
  JER2_ARAD_MAP_TYPE_NDX_OUT_OF_RANGE_ERR,
  JER2_ARAD_EGR_TC_OUT_OF_RANGE_ERR,
  JER2_ARAD_EGR_DP_OUT_OF_RANGE_ERR,
  JER2_ARAD_EGR_BASE_Q_PAIR_NDX_OUT_OF_RANGE_ERR,
  JER2_ARAD_EGR_PRIORITY_OUT_OF_RANGE_ERR,    
  JER2_ARAD_MGMT_DEVICE_REVISION_REV_A1_OR_BELOW_ERR,
  JER2_ARAD_TDM_LINK_BITMAP_OUT_OF_RANGE_ERR,
  JER2_ARAD_CELL_VARIABLE_IN_FAP20_21_SYSTEM_ERR,
  JER2_ARAD_INIT_QDR_TYPE_ILLEGAL_ERR,
  JER2_ARAD_MGMT_TDM_MODE_OF_RANGE_ERR,
  JER2_ARAD_MGMT_FTMH_LB_EXT_MODE_OF_RANGE_ERR,
  JER2_ARAD_NIF_OVERSUBSCR_SCHEME_OUT_OF_RANGE_ERR,
  JER2_ARAD_INIT_OOR_NOF_ENTRIES_OUT_OF_RANGE_ERR,
  JER2_ARAD_CELL_RECEIVED_CELL_SIZE_ERR,
  /*
   *    Inner errors
   */
  JER2_ARAD_ILKN_INVALID_LANE_FOR_MIN_SIZE_ERR,
  JER2_ARAD_ILKN_INVALID_LANE_SPECIFIED_BUT_INACTIVE_ERR,
    JER2_ARAD_ILKN_NOF_LANES_OUT_OF_RANGE_ERR,
    JER2_ARAD_ILKN_RETRANSMIT_BUFFER_SIZE_ENTRIES_OUT_OF_RANGE_ERR,
    JER2_ARAD_ILKN_RETRANSMIT_NOF_SEQ_NUMBER_REPETITIONS_OUT_OF_RANGE_ERR,
    JER2_ARAD_ILKN_RETRANSMIT_NOF_REQUESTS_RESENT_OUT_OF_RANGE_ERR,
    JER2_ARAD_ILKN_RETRANSMIT_TIMEOUT_OUT_OF_RANGE_ERR,
  JER2_ARAD_ILKN_METAFRAME_SYNC_PERIOD_OUT_OF_RANGE_ERR,
  JER2_ARAD_UNKNOWN_NIF_TYPE_ERR,
    JER2_ARAD_NO_NIF_STAT_IF_FOUND_ERR,
    JER2_ARAD_NIF_WC_ALREADY_CONFIGURED_ERR,
    JER2_ARAD_NIF_ILKN_NOF_ENTRIES_OUT_OF_RANGE_ERR,
    JER2_ARAD_NIF_2ND_INTERFACE_IS_B_ERR,
    JER2_ARAD_NIF_NOF_LANES_OUT_OF_RANGE_ERR,
  JER2_ARAD_NIF_INVALID_ID_ERR,
  JER2_ARAD_NIF_CHAN_INTERFACE_OUT_OF_RANGE_ERR,
  JER2_ARAD_NIF_SYNCE_INVALID_CLK_ID_FOR_TWO_DIFF_CLK_ERR,
  JER2_ARAD_NIF_SYNCE_CLK_ID_AND_SRC_MISMATCH_ERR,
  JER2_ARAD_FATP_MODE_OUT_OF_RANGE_ERR,
  JER2_ARAD_NIF_CNT_RD_INVALID_ERR,
  JER2_ARAD_NIF_PREAMBLE_COMPRESS_SIZE_OUT_OF_RANGE_ERR,
  JER2_ARAD_NIF_FR_LOCAL_OUT_OF_RANGE_ERR,
  JER2_ARAD_NIF_FR_REMOTE_OUT_OF_RANGE_ERR,
  JER2_ARAD_NIF_GMII_INFO_MODE_OUT_OF_RANGE_ERR,
  JER2_ARAD_NIF_GMII_INFO_RATE_OUT_OF_RANGE_ERR,
  JER2_ARAD_NIF_GMII_STAT_FAULT_TYPE_OUT_OF_RANGE_ERR,
  JER2_ARAD_NIF_CLK_FREQ_KHZ_OUT_OF_RANGE_ERR,
  JER2_ARAD_NIF_SYNC_RESET_VAL_MSEC_OUT_OF_RANGE_ERR,
  JER2_ARAD_NIF_SYNC_AUTOINC_INTERVAL_MSEC_OUT_OF_RANGE_ERR,
  JER2_ARAD_NIF_WIRE_DELAY_NS_OUT_OF_RANGE_ERR,
  JER2_ARAD_NIF_SYNCE_MODE_OUT_OF_RANGE_ERR,
  JER2_ARAD_NIF_FATP_BASE_PORT_ID_OUT_OF_RANGE_ERR,
  JER2_ARAD_NIF_FATP_NOF_PORTS_OUT_OF_RANGE_ERR,
  JER2_ARAD_PMF_LOW_LEVEL_FEM_ID_OUT_OF_RANGE_ERR,
  JER2_ARAD_PMF_OFFSET_FEM_INVALID_ERR,
  JER2_ARAD_PMF_LOW_LEVEL_CE_INIT_UNSAFE,
  JER2_ARAD_PMF_CE_NOF_REAL_BITS_COMPUTE_HEADER,
  JER2_ARAD_PMF_LOW_LEVEL_PMF_PGM_NDX_OUT_OF_RANGE_ERR,
  JER2_ARAD_PMF_LOW_LEVEL_PMF_KEY_OUT_OF_RANGE_ERR,
  JER2_ARAD_PMF_LOW_LEVEL_CE_NDX_OUT_OF_RANGE_ERR,
  JER2_ARAD_PMF_LOW_LEVEL_IRPP_FIELD_OUT_OF_RANGE_ERR,
  JER2_ARAD_PMF_LOW_LEVEL_IS_CE_NOT_VALID_OUT_OF_RANGE_ERR,
  JER2_ARAD_PMF_LOW_LEVEL_DB_ID_NDX_OUT_OF_RANGE_ERR,
  JER2_ARAD_PMF_LOW_LEVEL_SUCCESS_OUT_OF_RANGE_ERR,
  JER2_ARAD_PMF_LOW_LEVEL_KEY_FORMAT_OUT_OF_RANGE_ERR,
  JER2_ARAD_PMF_LOW_LEVEL_KEY_SRC_OUT_OF_RANGE_ERR,
  JER2_ARAD_PMF_LOW_LEVEL_ENTRY_NDX_OUT_OF_RANGE_ERR,
  JER2_ARAD_PMF_LOW_LEVEL_FEM_NDX_OUT_OF_RANGE_ERR,
  JER2_ARAD_PMF_LOW_LEVEL_TAG_PROFILE_NDX_OUT_OF_RANGE_ERR,
  JER2_ARAD_PMF_LOW_LEVEL_TAG_TYPE_NDX_OUT_OF_RANGE_ERR,
  JER2_ARAD_PMF_LOW_LEVEL_FEM_PGM_NDX_OUT_OF_RANGE_ERR,
  JER2_ARAD_PMF_LOW_LEVEL_SELECTED_BITS_NDX_OUT_OF_RANGE_ERR,
  JER2_ARAD_PMF_LOW_LEVEL_ACTION_FOMAT_NDX_OUT_OF_RANGE_ERR,
  JER2_ARAD_PMF_LOW_LEVEL_STAGE_OUT_OF_RANGE_ERR,
  JER2_ARAD_PMF_LOW_LEVEL_SUB_HEADER_OUT_OF_RANGE_ERR,
  JER2_ARAD_PMF_LOW_LEVEL_INCORRECT_INSTRUCTION_ERR,
  JER2_ARAD_PMF_LOW_LEVEL_OFFSET_OUT_OF_RANGE_ERR,
  JER2_ARAD_PMF_LOW_LEVEL_NOF_BITS_OUT_OF_RANGE_ERR,
  JER2_ARAD_PMF_LOW_LEVEL_DB_ID_OUT_OF_RANGE_ERR,
  JER2_ARAD_PMF_LOW_LEVEL_ENTRY_ID_OUT_OF_RANGE_ERR,
  JER2_ARAD_PMF_LOW_LEVEL_PRIORITY_OUT_OF_RANGE_ERR,
  JER2_ARAD_PMF_LOW_LEVEL_ID_OUT_OF_RANGE_ERR,
  JER2_ARAD_PMF_LOW_LEVEL_CYCLE_NDX_OUT_OF_RANGE_ERR,
  JER2_ARAD_PMF_LOW_LEVEL_VAL_OUT_OF_RANGE_ERR,
  JER2_ARAD_PMF_LOW_LEVEL_PGM_COUNTER_PARSE,
  JER2_ARAD_PMF_LOW_LEVEL_PGM_ID_OUT_OF_RANGE_ERR,
    JER2_ARAD_PMF_FES_SHIFT_OUT_OF_RANGE_ERR,
  JER2_ARAD_PMF_LOW_LEVEL_SRC_OUT_OF_RANGE_ERR,
  JER2_ARAD_PMF_LOW_LEVEL_VAL_SRC_OUT_OF_RANGE_ERR,
  JER2_ARAD_PMF_LOW_LEVEL_STAT_TAG_LSB_POSITION_OUT_OF_RANGE_ERR,
  JER2_ARAD_PMF_LOW_LEVEL_SIZE_OUT_OF_RANGE_ERR,
  JER2_ARAD_PMF_LOW_LEVEL_FORMAT_OUT_OF_RANGE_ERR,
  JER2_ARAD_PMF_LOW_LEVEL_MODE_OUT_OF_RANGE_ERR,
  JER2_ARAD_PMF_LOW_LEVEL_IPV4_FLD_OUT_OF_RANGE_ERR,
  JER2_ARAD_PMF_LOW_LEVEL_IPV6_FLD_OUT_OF_RANGE_ERR,
  JER2_ARAD_PMF_LOW_LEVEL_LOC_OUT_OF_RANGE_ERR,
  JER2_ARAD_PMF_LOW_LEVEL_IRPP_FLD_OUT_OF_RANGE_ERR,
  JER2_ARAD_PMF_LOW_LEVEL_SUB_HEADER_OFFSET_OUT_OF_RANGE_ERR,
  JER2_ARAD_PMF_LOW_LEVEL_PMF_PGM_ID_OUT_OF_RANGE_ERR,
  JER2_ARAD_PMF_LOW_LEVEL_L2_OUT_OF_RANGE_ERR,
  JER2_ARAD_PMF_LOW_LEVEL_SEL_BIT_MSB_OUT_OF_RANGE_ERR,
  JER2_ARAD_PMF_LOW_LEVEL_ACTION_FOMAT_ID_OUT_OF_RANGE_ERR,
  JER2_ARAD_PMF_LOW_LEVEL_MAP_DATA_OUT_OF_RANGE_ERR,
  JER2_ARAD_PMF_LOW_LEVEL_TYPE_OUT_OF_RANGE_ERR,
  JER2_ARAD_PMF_LOW_LEVEL_BASE_VALUE_OUT_OF_RANGE_ERR,
  JER2_ARAD_PMF_LOW_LEVEL_PFQ_OUT_OF_RANGE_ERR,
  JER2_ARAD_PMF_LOW_LEVEL_SEM_13_8_NDX_OUT_OF_RANGE_ERR,
  JER2_ARAD_PMF_LOW_LEVEL_FWD_OUT_OF_RANGE_ERR,
  JER2_ARAD_PMF_LOW_LEVEL_TTC_OUT_OF_RANGE_ERR,
  JER2_ARAD_PMF_LOW_LEVEL_PRSR_OUT_OF_RANGE_ERR,
  JER2_ARAD_PMF_LOW_LEVEL_PORT_PMF_OUT_OF_RANGE_ERR,
  JER2_ARAD_PMF_LOW_LEVEL_LLVP_OUT_OF_RANGE_ERR,
  JER2_ARAD_PMF_LOW_LEVEL_PMF_PRO_OUT_OF_RANGE_ERR,
  JER2_ARAD_PMF_LOW_LEVEL_EEI_OUTLIF_15_8_OUT_OF_RANGE_ERR,
  JER2_ARAD_PMF_LOW_LEVEL_EEI_OUTLIF_7_0_OUT_OF_RANGE_ERR,
  JER2_ARAD_PMF_LOW_LEVEL_SEM_7_0_NDX_OUT_OF_RANGE_ERR,
  JER2_ARAD_PMF_LOW_LEVEL_HEADER_TYPE_OUT_OF_RANGE_ERR,
  JER2_ARAD_PMF_LOW_LEVEL_NOF_BYTES_OUT_OF_RANGE_ERR,
  JER2_ARAD_PMF_LOW_LEVEL_LKP_PROFILE_ID_OUT_OF_RANGE_ERR,
  JER2_ARAD_PMF_LOW_LEVEL_TAG_PROFILE_ID_OUT_OF_RANGE_ERR,
  JER2_ARAD_PMF_LOW_LEVEL_FC_TYPE_OUT_OF_RANGE_ERR,
  JER2_ARAD_PMF_LOW_LEVEL_COPY_PGM_VAR_OUT_OF_RANGE_ERR,
  JER2_ARAD_PMF_LOW_LEVEL_BIT_LOC_VAL_OUT_OF_RANGE_ERR,
  JER2_ARAD_PMF_LOW_LEVEL_AF_TYPE_OUT_OF_RANGE_ERR,
  JER2_ARAD_PMF_LOW_LEVEL_PGM_TYPE_OUT_OF_RANGE_ERR,
  JER2_ARAD_PMF_LOW_LEVEL_PGM_NOT_ENOUGH_ERR,
  JER2_ARAD_PMF_FEM_INVALID_FOR_OFFSET_ERR,
  JER2_ARAD_PMF_FEM_OUTPUT_SIZE_ERR,
  JER2_ARAD_PMF_FEM_TYPE_OUT_OF_RANGE_ERR,
  JER2_ARAD_PMF_LOW_LEVEL_PORT_HEADER_TYPE_OUT_OF_RANGE_ERR,
  JER2_ARAD_PMF_LOW_LEVEL_DIRECT_TBL_VAL_OUT_OF_RANGE_ERR,
  JER2_ARAD_PMF_LOW_LEVEL_TCAM_KEY_SRC_OUT_OF_RANGE_ERR,
  JER2_ARAD_PMF_TCAM_NOF_BANKS_VALID_MAX_ERR,
  JER2_ARAD_PMF_CE_FLD_NOT_FOUND_ERR,
  JER2_ARAD_TCAM_KEY_FLD_VAL_OUT_OF_RANGE_ERR,
  JER2_ARAD_PMF_LOW_LEVEL_DIAG_PROG_ID_OUT_OF_RANGE_ERR,
  JER2_ARAD_PMF_LOW_LEVEL_DIAG_ENABLE_OUT_OF_RANGE_ERR,
  JER2_ARAD_PMF_LOW_LEVEL_DIAG_BUFF_OUT_OF_RANGE_ERR,
  JER2_ARAD_PMF_LOW_LEVEL_DIAG_BUFF_LEN_OUT_OF_RANGE_ERR,
  JER2_ARAD_PMF_LOW_LEVEL_DIAG_TYPE_OUT_OF_RANGE_ERR,
  JER2_ARAD_PMF_LOW_LEVEL_DIAG_VAL_OUT_OF_RANGE_ERR,
  JER2_ARAD_PMF_LOW_LEVEL_DIAG_VALUE_OUT_OF_RANGE_ERR,
  JER2_ARAD_PMF_LOW_LEVEL_DIAG_STRENGTH_OUT_OF_RANGE_ERR,
  JER2_ARAD_PMF_LOW_LEVEL_DIAG_PASS_NUM_OUT_OF_RANGE_ERR,
  JER2_ARAD_PMF_LOW_LEVEL_DIAG_PRG_OUT_OF_RANGE_ERR,
  JER2_ARAD_PMF_LOW_LEVEL_DIAG_IN_KEY_OUT_OF_RANGE_ERR,
  JER2_ARAD_PMF_LOW_LEVEL_DIAG_IN_PRG_OUT_OF_RANGE_ERR,
  JER2_ARAD_PMF_LOW_LEVEL_DIAG_INVALID_ACTS_COMBINATION_ERR,
  JER2_ARAD_PP_TBLS_NOT_INITIALIZED,
  JER2_ARAD_PP_MAP_TBL_NDX_OUT_OF_RANGE_ERR,
  JER2_ARAD_PP_TBL_RANGE_OUT_OF_LIMIT,
  JER2_ARAD_TCAM_BANK_UNINITIALIZED_ERR,
  JER2_ARAD_TCAM_DB_DOESNT_EXIST_ERR,
  JER2_ARAD_TCAM_DB_BANK_NOT_USED_ERR,
  JER2_ARAD_TCAM_DB_BANK_ALREADY_USED_ERR,
  JER2_ARAD_TCAM_DB_ENTRY_SIZE_MISMATCH_ERR,
  JER2_ARAD_TCAM_DB_BANK_OWNER_MISMATCH_ERR,
  JER2_ARAD_TCAM_DB_ENTRY_NDX_OUT_OF_RANGE_ERR,
  JER2_ARAD_TCAM_ACCESS_PROFILE_ALREADY_EXISTS_ERR,
  JER2_ARAD_TCAM_BANK_NDX_OUT_OF_RANGE_ERR,
  JER2_ARAD_TCAM_BANK_ENTRY_SIZE_OUT_OF_RANGE_ERR,
  JER2_ARAD_TCAM_FWDING_USER_CYCLE_OUT_OF_RANGE_ERR,
  JER2_ARAD_TCAM_DATABASE_ALREADY_EXISTS_ERR,
  JER2_ARAD_TCAM_CYCLE_NDX_OUT_OF_RANGE_ERR,
  JER2_ARAD_TCAM_TCAM_USER_OUT_OF_RANGE_ERR,
  JER2_ARAD_TCAM_TCAM_USER_NDX_OUT_OF_RANGE_ERR,
  JER2_ARAD_TCAM_DB_ID_NDX_OUT_OF_RANGE_ERR,
  JER2_ARAD_TCAM_VAL_OUT_OF_RANGE_ERR,
  JER2_ARAD_TCAM_NOF_BITS_OUT_OF_RANGE_ERR,
  JER2_ARAD_TCAM_PREFIX_LENGTH_OUT_OF_RANGE_ERR,
  JER2_ARAD_TCAM_USE_TYPE_OUT_OF_RANGE_ERR,
  JER2_ARAD_TCAM_ACL_OUT_OF_RANGE_ERR,
  JER2_ARAD_TCAM_ACL_ACE_TYPE_OUT_OF_RANGE_ERR,
  JER2_ARAD_TCAM_ACCESS_PROFILE_DOESNT_EXIST_ERR,
  JER2_ARAD_PORTS_FC_TYPE_OUT_OF_RANGE_ERR,
  JER2_ARAD_PORTS_FIRST_HEADER_SIZE_OUT_OF_RANGE_ERR,
  JER2_ARAD_PORTS_HEADER_TYPE_OUT_OF_RANGE_ERR,
  JER2_ARAD_PORTS_PP_PORT_OUT_OF_RANGE_ERR,
  JER2_ARAD_PORTS_PROCESSOR_ID_OUT_OF_RANGE_ERR,
  JER2_ARAD_PORTS_ID_OUT_OF_RANGE_ERR,
  JER2_ARAD_PORTS_TR_CLS_OUT_OF_RANGE_ERR,
  JER2_ARAD_PORTS_DP_OUT_OF_RANGE_ERR,
  JER2_ARAD_PORTS_SNOOP_CMD_NDX_OUT_OF_RANGE_ERR,
  JER2_ARAD_PORTS_PROFILE_IS_HEADER_TYPE_INCONSISTENT_ERR,
  JER2_ARAD_PORTS_NOT_TM_PORT_IS_PPH_PRESENT_ERR,
  JER2_ARAD_PORTS_ETH_PORT_MIRROR_PROFILE_ERR,
  JER2_ARAD_PORTS_NOT_TM_PORT_ERR,
  JER2_ARAD_PORTS_NOT_PP_PORT_FOR_RAW_PORTS_ERR,
  JER2_ARAD_PORT_TRAP_CODE_NOT_SUPPORTED_ERR,
  JER2_ARAD_PORTS_PORT_NDX_OUT_OF_RANGE_ERR,
  JER2_ARAD_PORTS_NIF_CTXT_MAP_OUT_OF_RANGE_ERR,
  JER2_ARAD_PORTS_HDR_FORMAT_OUT_OF_RANGE_ERR,
  JER2_ARAD_PORTS_PP_PORT_FOR_TM_TRAFFIC_OUT_OF_RANGE_ERR,
  JER2_ARAD_PORTS_INVALID_CTXT_MAPPING_ERR,
  JER2_ARAD_PORT_SYMMETRIC_ASSUMPTION_ERR,
  JER2_ARAD_PP_TBL_ACCESS_UNKNOWN_MODULE_ID_ERR,
  JER2_ARAD_PORTS_NON_ZERO_CHANNEL_FOR_UNCHANNELIZED_IF_TYPE_ERR,
  JER2_ARAD_INIT_TM_PROFILE_OUT_OF_RSRC_ERR,
  JER2_ARAD_DIAG_LBG_TM_PROFILE_OUT_OF_RSRC_ERR,
  JER2_ARAD_DIAG_SAMPLE_NOT_ENABLED_ERR,
  JER2_ARAD_MGMT_PROFILE_NDX_OUT_OF_RANGE_ERR,
  JER2_ARAD_MGMT_PP_PORT_NOF_ENTRIES_OUT_OF_RANGE_ERR,
  JER2_ARAD_MGMT_TM_PROFILE_MAP_NOF_ENTRIES_OUT_OF_RANGE_ERR,
  JER2_ARAD_PORTS_OTMH_SRC_DEST_NOT_EQUAL_ERR,
  JER2_ARAD_EGR_PROG_PP_PORT_OUT_OF_RANGE_ERR,
  JER2_ARAD_NIF_SRD_LANES_COMP_ERR,
  JER2_ARAD_IQM_PRFSEL_TBL_INDEX_OUT_OF_RANGE_ERR,
  JER2_ARAD_IQM_PRFCFG_TBL_INDEX_OUT_OF_RANGE_ERR,
  JER2_ARAD_DIAG_LBG_PATH_NOF_PORTS_OUT_OF_RANGE_ERR,
  JER2_ARAD_CNT_DEFINED_LSB_STAG_ERR,
  JER2_ARAD_CNT_EG_MODE_AND_PROC_A,
  JER2_ARAD_CNT_EG_MODE_OUT_OF_RANGE_ERR,
  JER2_ARAD_CNT_ING_MODE_OUT_OF_RANGE_ERR,
  JER2_ARAD_CNT_VOQ_PARAMS_OUT_OF_RANGE_ERR,
  JER2_ARAD_CNT_START_FIFO_DMA_ERR,
  JER2_ARAD_FC_FLOW_CONTROL_NOT_SUPPORTED_ON_IF_ERR,
  JER2_ARAD_FC_TC_NOT_SUPPORTED_ON_IF_ERR,
  JER2_ARAD_NIF_FATP_DISABLED_BUT_SET_ERR,
  JER2_ARAD_CNT_DIRECT_READ_OUT_OF_TIME_ERR,
  JER2_ARAD_CNT_PROC_SRC_TYPE_NOT_VOQ_ERR,
  JER2_ARAD_ACTION_CMD_DEST_ID_OUT_OF_RANGE_ERR,
  JER2_ARAD_ACTION_CMD_SNOOP_SIZE_OUT_OF_RANGE_ERR,
  JER2_ARAD_MULT_EG_TABLE_FORMAT_INVALID_ERR,
  JER2_ARAD_MDIO_CLK_FREQ_ERR,
  JER2_ARAD_NIF_SYNCE_MALGB_CLK_WHEN_DISABLED_ERR,
  JER2_ARAD_SW_DB_MULTI_SET_OUT_OF_RANGE_ERR,
  JER2_ARAD_EGR_ACL_L2_OUT_OF_RANGE_ERR,
  JER2_ARAD_EGR_ACL_IPV4_OUT_OF_RANGE_ERR,
  JER2_ARAD_EGR_ACL_MPLS_OUT_OF_RANGE_ERR,
  JER2_ARAD_EGR_ACL_TM_OUT_OF_RANGE_ERR,
  JER2_ARAD_EGR_ACL_CODE_NDX_OUT_OF_RANGE_ERR,
  JER2_ARAD_EGR_ACL_PROTOCOL_CODE_OUT_OF_RANGE_ERR,
  JER2_ARAD_EGR_ACL_PP_PORT_NDX_OUT_OF_RANGE_ERR,
  JER2_ARAD_EGR_ACL_FWD_TYPE_OUT_OF_RANGE_ERR,
  JER2_ARAD_EGR_ACL_ACL_PROFILE_NDX_OUT_OF_RANGE_ERR,
  JER2_ARAD_EGR_ACL_KEY_PROFILE_OUT_OF_RANGE_ERR,
  JER2_ARAD_EGR_ACL_KEY_PROFILE_NDX_OUT_OF_RANGE_ERR,
  JER2_ARAD_EGR_ACL_DB_NDX_OUT_OF_RANGE_ERR,
  JER2_ARAD_EGR_ACL_DB_TYPE_OUT_OF_RANGE_ERR,
  JER2_ARAD_EGR_ACL_ENTRY_NDX_OUT_OF_RANGE_ERR,
  JER2_ARAD_EGR_ACL_IS_FOUND_OUT_OF_RANGE_ERR,
  JER2_ARAD_EGR_ACL_VAL_OUT_OF_RANGE_ERR,
  JER2_ARAD_EGR_ACL_PROFILE_OUT_OF_RANGE_ERR,
  JER2_ARAD_EGR_ACL_ACL_DATA_OUT_OF_RANGE_ERR,
  JER2_ARAD_EGR_ACL_TRAP_CODE_OUT_OF_RANGE_ERR,
  JER2_ARAD_EGR_ACL_OFP_OUT_OF_RANGE_ERR,
  JER2_ARAD_EGR_ACL_TC_OUT_OF_RANGE_ERR,
  JER2_ARAD_EGR_ACL_DP_OUT_OF_RANGE_ERR,
  JER2_ARAD_EGR_ACL_CUD_OUT_OF_RANGE_ERR,
  JER2_ARAD_EGR_ACL_PRIORITY_OUT_OF_RANGE_ERR,
  JER2_ARAD_EGR_ACL_DB_IDS_OUT_OF_RANGE_ERR,
  JER2_ARAD_EGR_ACL_TYPE_OUT_OF_RANGE_ERR,
  JER2_ARAD_EGR_ACL_TYPE_DIFFERENT_FROM_DB_ERR,
  JER2_ARAD_TCAM_KEY_FORMAT_TYPE_OUT_OF_RANGE_ERR,
  JER2_ARAD_NIF_API_NOT_APPLICABLE_FOR_ILKN_ERR,
  JER2_ARAD_FC_PFC_GENERIC_BITMAP_OUT_OF_RANGE,
  JER2_ARAD_FC_PFC_GENERIC_BITMAP_BIT_OUT_OF_RANGE,
  JER2_ARAD_FC_INVALID_CALENDAR_ENTRY_ERR,
  JER2_ARAD_DRAM_PLL_F_OUT_OF_RANGE_ERR,
  JER2_ARAD_DRAM_PLL_Q_OUT_OF_RANGE_ERR,
  JER2_ARAD_DRAM_PLL_R_OUT_OF_RANGE_ERR,
  JER2_ARAD_CNM_CP_MULTIPLE_GEN_MODE,
  JER2_ARAD_DRAM_PLL_CONF_NOT_SET_ERR,
  JER2_ARAD_DRAM_BIST_INVALID_TEST_PARAMETERS_ERR,
  JER2_ARAD_TCAM_BANK_ID_OUT_OF_RANGE_ERR,
  JER2_ARAD_TCAM_ENTRY_SIZE_OUT_OF_RANGE_ERR,
  JER2_ARAD_TCAM_ACTION_SIZE_OUT_OF_RANGE_ERR,
  JER2_ARAD_TCAM_TCAM_DB_ID_OUT_OF_RANGE_ERR,
  JER2_ARAD_TCAM_PREFIX_SIZE_OUT_OF_RANGE_ERR,
  JER2_ARAD_TCAM_PRIO_MODE_OUT_OF_RANGE_ERR,
  JER2_ARAD_TCAM_ENTRY_ID_OUT_OF_RANGE_ERR,
  JER2_ARAD_TCAM_PRIORITY_OUT_OF_RANGE_ERR,
  JER2_ARAD_TCAM_ACTION_OUT_OF_RANGE_ERR,
  JER2_ARAD_TCAM_BITS_OUT_OF_RANGE_ERR,
  JER2_ARAD_TCAM_LENGTH_OUT_OF_RANGE_ERR,
  JER2_ARAD_TCAM_ENTRY_OUT_OF_RANGE_ERR,
  JER2_ARAD_TCAM_DB_METADATA_INCONSISTENT_ERR,
  JER2_ARAD_TCAM_DB_ENTRY_SEARCH_SIZE_NOT_SUPPORTED_ERR,
  JER2_ARAD_TCAM_MGMT_PROFILE_ID_OUT_OF_RANGE_ERR,
  JER2_ARAD_TCAM_MGMT_TCAM_DB_ID_OUT_OF_RANGE_ERR,
  JER2_ARAD_TCAM_MGMT_CYCLE_OUT_OF_RANGE_ERR,
  JER2_ARAD_TCAM_MGMT_UNIFORM_PREFIX_OUT_OF_RANGE_ERR,
  JER2_ARAD_TCAM_MGMT_MIN_BANKS_OUT_OF_RANGE_ERR,
  JER2_ARAD_TCAM_MGMT_USER_DATA_OUT_OF_RANGE_ERR,
  JER2_ARAD_TCAM_MGMT_BANK_OWNER_OUT_OF_RANGE_ERR,
  JER2_ARAD_TCAM_MGMT_ENTRY_ID_OUT_OF_RANGE_ERR,
  JER2_ARAD_TCAM_MGMT_PRIORITY_OUT_OF_RANGE_ERR,
  JER2_ARAD_TCAM_MGMT_ACTION_OUT_OF_RANGE_ERR,
  JER2_ARAD_TCAM_MGMT_BANK_ID_OUT_OF_RANGE_ERR,
  JER2_ARAD_TCAM_MGMT_ACCESS_DEVICE_OUT_OF_RANGE_ERR,
  JER2_ARAD_STACK_LOCAL_STACK_PORT_NDX_OUT_OF_RANGE_ERR,
  JER2_ARAD_STACK_SYS_PHY_PORT_NDX_OUT_OF_RANGE_ERR,
  JER2_ARAD_STACK_SYS_FAP_ID_OUT_OF_RANGE_ERR,
  JER2_ARAD_STACK_STACK_PORT_LOCAL_ID_OUT_OF_RANGE_ERR,
  JER2_ARAD_STACK_MAX_NOF_TM_DOMAINS_OUT_OF_RANGE_ERR,
  JER2_ARAD_STACK_MY_TM_DOMAIN_OUT_OF_RANGE_ERR,
  JER2_ARAD_STACK_PEER_TM_DOMAIN_OUT_OF_RANGE_ERR,
  JER2_ARAD_FLOW_CONTROL_TRGR_NDX_OUT_OF_RANGE_ERR,
  JER2_ARAD_FLOW_CONTROL_REACT_POINT_NDX_OUT_OF_RANGE_ERR,
  JER2_ARAD_FLOW_CONTROL_VSQ_BITMAP_NOT_VALID_ERR,
  JER2_ARAD_FLOW_CONTROL_OFP_IS_NOT_DEFINED_AS_RCY_ERR,
  JER2_ARAD_FLOW_CONTROL_PRIO_NDX_OUT_OF_RANGE_ERR,
  JER2_ARAD_PMF_PGM_MGMT_PGM_SOURCE_OUT_OF_RANGE_ERR,
  JER2_ARAD_PMF_PGM_MGMT_PMF_PGM_NDX_OUT_OF_RANGE_ERR,
  JER2_ARAD_PMF_PGM_MGMT_PFG_OUT_OF_RANGE_ERR,
  JER2_ARAD_PMF_PGM_MGMT_RESTORE_ERR,
  JER2_ARAD_NIF_SYNCE_CLK_SQUELTCH_ON_DIV_20_ERR,
  JER2_ARAD_MGMT_TDM_MC_ROUTE_MODE_OUT_OF_RANGE_ERR,  
  JER2_ARAD_MGMT_OCB_MC_RANGE_INDEX_OUT_OF_RANGE_ERR,
  JER2_ARAD_MGMT_OCB_MC_RANGE_MIN_OUT_OF_RANGE_ERR,
  JER2_ARAD_MGMT_OCB_MC_RANGE_MAX_OUT_OF_RANGE_ERR,
  JER2_ARAD_MGMT_OCB_VOQ_ELIGIBLE_QUEUE_CATEGORY_INDEX_OUT_OF_RANGE_ERR,
  JER2_ARAD_MGMT_OCB_VOQ_ELIGIBLE_RATE_CLASS_INDEX_OUT_OF_RANGE_ERR,
  JER2_ARAD_MGMT_OCB_VOQ_ELIGIBLE_TRAFFIC_CLASS_INDEX_OUT_OF_RANGE_ERR,
  JER2_ARAD_FABRIC_MULT_INGRESS_MULT_NOT_SINGLE_COPY_ERR,
  JER2_ARAD_TDM_MC_ID_ROUTE_NDX_OUT_OF_RANGE_ERR,
  JER2_ARAD_DIAGNOSTICS_TM_PORT_OUT_OF_RANGE_ERR,
  JER2_ARAD_DIAGNOSTICS_PP_PORT_OUT_OF_RANGE_ERR,
  JER2_ARAD_INCONSISTENCY_ERR,
  JER2_ARAD_WB_DB_UNSUPPORTED_SCACHE_DATA_VERSION_ERR,
  JER2_ARAD_STAT_IF_REPORT_PKT_SIZE_OUT_OF_RANGE_ERR,
  JER2_ARAD_STAT_IF_REPORT_SCRUBBER_QUEUE_MIN_OUT_OF_RANGE_ERR,
  JER2_ARAD_STAT_IF_REPORT_SCRUBBER_QUEUE_MAX_OUT_OF_RANGE_ERR,
  JER2_ARAD_STAT_IF_REPORT_SCRUBBER_QUEUE_OUT_OF_RANGE_ERR,
  JER2_ARAD_STAT_IF_REPORT_SCRUBBER_RATE_OUT_OF_RANGE_ERR,
  JER2_ARAD_STAT_IF_REPORT_DESC_THRESHOLD_OUT_OF_RANGE_ERR,
  JER2_ARAD_STAT_IF_REPORT_BDB_THRESHOLD_OUT_OF_RANGE_ERR,
  JER2_ARAD_STAT_IF_REPORT_UC_DRAM_THRESHOLD_OUT_OF_RANGE_ERR,
  JER2_ARAD_STAT_IF_INIT_FAILED_ERR,
  JER2_ARAD_STAT_RATE_NOT_FOUND_ERR,
  JER2_ARAD_FABRIC_PRIORITY_OUT_OF_RANGE,
  JER2_ARAD_EGR_PROG_EDITOR_OUT_OF_PROGRAMS_ERR,
  JER2_ARAD_EGR_PROG_EDITOR_OUT_OF_EVEN_LFEMS_ERR,
  JER2_ARAD_EGR_PROG_EDITOR_OUT_OF_ODD_LFEMS_ERR,
  JER2_ARAD_EGR_PROG_EDITOR_OUT_OF_INSTRUCTION_ENTRIES_ERR,
  JER2_ARAD_EGR_PROG_EDITOR_OUT_OF_TM_PROFILES_ERR,
  JER2_ARAD_EGR_PROG_EDITOR_OUT_OF_SELECTION_CAM_LINES,
  JER2_ARAD_EGR_PROG_EDITOR_PROGRAM_NOT_USED_ERR,
  JER2_ARAD_PORTS_SWAP_OFFSET_OUT_OF_RANGE_ERR,
  JER2_ARAD_PORTS_SWAP_MODE_OUT_OF_RANGE_ERR,
  JER2_ARAD_PORTS_PON_PORT_NDX_OUT_OF_RANGE_ERR,
  JER2_ARAD_PORTS_PON_TUNNEL_ID_OUT_OF_RANGE_ERR,
  JER2_ARAD_PORTS_PON_IN_PP_OUT_OF_RANGE_ERR,
  JER2_ARAD_INTERNAL_ASSERT_ERR,
  JER2_ARAD_ITM_DP_DISCARD_OUT_OF_RANGE_ERR,
  JER2_ARAD_FABRIC_GCI_CONFIG_TYPE_OUT_OF_RANGE,
  JER2_ARAD_FABRIC_MIXED_CONFIGURATION_ERR,
  JER2_ARAD_NIF_API_NOT_APLICABLE_FOR_NON_PFC_PORT,
  JER2_ARAD_PORTS_RX_DISABLE_TDM_MIN_SIZE_ILLEGAL,
  JER2_ARAD_UNSUPPORTED_FC_TYPE_ERR,
  JER2_ARAD_PP_EG_ENCAP_LSB_MUST_BE_0,
  /*
   *    Must be the last element
   */
  JER2_ARAD_LAST_ERR
} JER2_ARAD_ERR;

/*
 * Error codes
 * }
 */

/* } */

/*************
 *  MACROS   *
 *************/
/* { */

#define JER2_ARAD_FLD_SIZE_BITS(msb, lsb) (msb - lsb + 1)

/* } */

/*************
 * TYPE DEFS *
 *************/
/* { */

/* } */

/*************
 * GLOBALS   *
 *************/
/* { */

/* } */

/*************
 * FUNCTIONS *
 *************/
/* { */

/*****************************************************
*NAME
* jer2_arad_add_jer2_arad_errors
*TYPE:
*  PROC
*DATE:
*  08/14/2007
*FUNCTION:
*  Add the pool of JER2_ARAD errors to the all-system
*  sorted pool.
*CALLING SEQUENCE:
*  jer2_arad_add_jer2_arad_errors()
*INPUT:
*  DNX_SAND_DIRECT:
*    None.
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    Non-Zero in case of an error
*  DNX_SAND_INDIRECT:
*    Updated all-system pool of sorted error descriptors.
*REMARKS:
*SEE ALSO:
*****************************************************/
uint32
  jer2_arad_add_jer2_arad_errors(void);

/*****************************************************
*NAME
* jer2_arad_add_jer2_arad_procedure_desc
*TYPE:
*  PROC
*DATE:
*  08/14/2007
*FUNCTION:
*  Add the pool of jer2_arad procedure descriptors to
*  the all-system sorted pool.
*CALLING SEQUENCE:
* jer2_arad_add_jer2_arad_procedure_desc()
*INPUT:
*  DNX_SAND_DIRECT:
*    None.
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    Non-Zero in case of an error
*  DNX_SAND_INDIRECT:
*    Updated all-system pool of sorted procedure
*    descriptors.
*REMARKS:
*SEE ALSO:
*****************************************************/
uint32
  jer2_arad_add_jer2_arad_procedure_desc(void);

/* } */

/*****************************************************
*NAME
*  jer2_arad_get_err_text
*TYPE:
*  PROC
*DATE:
*  25/SEP/2007
*FUNCTION:
*  This procedure finds the matching text describing
*  input error identifier.
*CALLING SEQUENCE:
*  jer2_arad_get_err_text(err_id,err_name,err_text)
*INPUT:
*  DNX_SAND_DIRECT:
*    uint32 err_id -
*      Identifier of the error for which description
*      is required. See list of ERROR RETURN VALUES above.
*    char         **err_name -
*      This procedure loads pointed memory with
*      pointer to null terminated string containing
*      error name. Caller may NOT change the
*      contents of this buffer.
*    char         **err_text -
*      This procedure loads pointed memory with
*      pointer to null terminated string containing
*      error description. Caller may NOT change the
*      contents of this buffer.
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    uint32 -
*      See formatting rules in JER2_ARAD_ERROR RETURN VALUES above.
*      If error code is not jer2_arad_NO_ERR then
*        specific error codes:
*          JER2_ARAD_GET_ERR_TEXT_001 -
*            Input variable 'err_id' has unknown value
*            (could not be translated, as required).
*      Otherwise, no error has been detected and text
*        has been retrieved.
*  DNX_SAND_INDIRECT:
*    err_text
*REMARKS:
*  Note that text returned by this procedure does not
*  change during runtime and may only be changed via a
*  new compilation.
*SEE ALSO:
*****************************************************/
uint32
  jer2_arad_get_err_text(
    uint32  err_id,
    char           **err_name,
    char           **err_text
  ) ;
/*****************************************************
*NAME
* jer2_arad_proc_id_to_string
*TYPE:
*  PROC
*DATE:
*  17/FEB/2003
*FUNCTION:
*  Get ASCII names of module and procedure from input
*  procedure id.
*CALLING SEQUENCE:
*  jer2_arad_proc_id_to_string(
*        proc_id,out_module_name,out_proc_name)
*INPUT:
*  DNX_SAND_DIRECT:
*    uint32  proc_id -
*      Procedure id to locate name and module of.
*    char           **out_module_name -
*      This procedure loads pointed memory with
*      pointer to null terminated string containing
*      the name of the module.
*    char           **out_proc_name -
*      This procedure loads pointed memory with
*      pointer to null terminated string containing
*      the name of the procedure.
*  DNX_SAND_INDIRECT:
*    All-system procedure descriptor pools
*    (e.g. Arad_procedure_desc_element).
*OUTPUT:
*  DNX_SAND_DIRECT:
*    int -
*      If non-zero then some error has occurred and
*      procedure string has not been located.
*  DNX_SAND_INDIRECT:
*    See out_module_name, out_module_name.
*REMARKS:
*SEE ALSO:
*****************************************************/
uint32
  jer2_arad_proc_id_to_string(
    uint32 in_proc_id,
    char          **out_module_name,
    char          **out_proc_name
  ) ;

/*****************************************************
*NAME
* jer2_arad_errors_ptr_get
*TYPE:
*  PROC
*DATE:
*  10/OCT/2007
*FUNCTION:
*  Get the pointer to the list of errors of the 'Arad'
*  module.
*CALLING SEQUENCE:
*  jer2_arad_errors_ptr_get()
*INPUT:
*  DNX_SAND_DIRECT:
*    None.
*  DNX_SAND_INDIRECT:
*    list of Arad errors: Arad_error_desc_element.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    DNX_ERROR_DESC_ELEMENT * -
*      Pointer to the static list of Arad errors.
*  DNX_SAND_INDIRECT:
*    .
*REMARKS:
*  This utility is mainly for external users (to the 'Arad'
*  module) such as 'dnx_sand module'.
*SEE ALSO:
*
*****************************************************/
CONST DNX_ERROR_DESC_ELEMENT
  *jer2_arad_errors_ptr_get(
    void
  );

/*****************************************************
*NAME
* jer2_arad_errors_add
*TYPE:
*  PROC
*DATE:
*  08/14/2007
*FUNCTION:
*  Add the pool of JER2_ARAD errors to the all-system
*  sorted pool.
*CALLING SEQUENCE:
*  jer2_arad_errors_add()
*INPUT:
*  DNX_SAND_DIRECT:
*    None.
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    Non-Zero in case of an error
*  DNX_SAND_INDIRECT:
*    Updated all-system pool of sorted error descriptors.
*REMARKS:
*SEE ALSO:
*****************************************************/

uint32
  jer2_arad_errors_add(void);

/*****************************************************
*NAME
* jer2_arad_procedure_desc_add
*TYPE:
*  PROC
*DATE:
*  08/14/2007
*FUNCTION:
*  Add the pool of jer2_arad procedure descriptors to
*  the all-system sorted pool.
*CALLING SEQUENCE:
* jer2_arad_procedure_desc_add()
*INPUT:
*  DNX_SAND_DIRECT:
*    None.
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    Non-Zero in case of an error
*  DNX_SAND_INDIRECT:
*    Updated all-system pool of sorted procedure
*    descriptors.
*REMARKS:
*SEE ALSO:
*****************************************************/
uint32
  jer2_arad_procedure_desc_add(void);

uint32
  jer2_arad_err_text_get(
    DNX_SAND_IN  uint32  err_id,
    DNX_SAND_OUT char      **err_name,
    DNX_SAND_OUT char      **err_text
  );

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>

/* } __JER2_ARAD_API_FRAMEWORK_H_INCLUDED__*/
#endif

/* } */

