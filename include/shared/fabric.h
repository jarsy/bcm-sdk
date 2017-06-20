/*
 * $Id: fabric.h,v 1.6 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * This file defines common network port parameters.
 *
 * Its contents are not used directly by applications; it is used only
 * by header files of parent APIs which need to define port parameters.
 */

#ifndef _SHR_FABRIC_H
#define _SHR_FABRIC_H

#define _SHR_FABRIC_LINK_STATUS_CRC_ERROR    0x00000001 /* Non-zero CRC rate */
#define _SHR_FABRIC_LINK_STATUS_SIZE_ERROR   0x00000002 /* Non-zero size
                                                          error-count */
#define _SHR_FABRIC_LINK_STATUS_CODE_GROUP_ERROR 0x00000004 /* Non-zero code group
                                                          error-count */
#define _SHR_FABRIC_LINK_STATUS_MISALIGN     0x00000008 /* Link down,
                                                          misalignment error */
#define _SHR_FABRIC_LINK_STATUS_NO_SIG_LOCK  0x00000010 /* Link down, SerDes
                                                          signal lock error */
#define _SHR_FABRIC_LINK_STATUS_NO_SIG_ACCEP 0x00000020 /* Link up, but not
                                                          accepting reachability
                                                          cells */
#define _SHR_FABRIC_LINK_STATUS_ERRORED_TOKENS 0x00000040 /* Low value, indicates
                                                          bad link connectivity
                                                          or link down, based on
                                                          reachability cells */

#define _SHR_FABRIC_LINK_NO_CONNECTIVITY (0xFFFFFFFF) /* FABRIC_LINK_NO_CONNECTIVITY */


/* bcm_fabric_priority_* flags */
#define _SHR_FABRIC_QUEUE_PRIORITY_HIGH_ONLY 0x1        
#define _SHR_FABRIC_QUEUE_PRIORITY_LOW_ONLY  0x2             
#define _SHR_FABRIC_PRIORITY_MULTICAST       0x8  
#define _SHR_FABRIC_PRIORITY_TDM             0x10

/*bcm_fabric_multicast_* flgas*/
#define _SHR_FABRIC_MULTICAST_SET_ONLY       0x1
#define _SHR_FABRIC_MULTICAST_COMMIT_ONLY    0x2
#define _SHR_FABRIC_MULTICAST_STATUS_ONLY    0x4

/* FabricLinkCellFormat flags */
#define _SHR_FABRIC_LINK_CELL_FORMAT_VSC256_V1              (0)/* VS256_V1 cell format */
#define _SHR_FABRIC_LINK_CELL_FORMAT_VSC128                 (1)/* VSC128 cell format */
#define _SHR_FABRIC_LINK_CELL_FORMAT_VSC256_V2              (2)/* VS256_V2 cell format */

#endif	/* !_SHR_FABRIC_H */

