/*
 * $Id: c3hppc_ocm.h,v 1.6 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * lrp.h : LRP defines
 *
 *-----------------------------------------------------------------------------*/
#ifndef _C3HPPC_OCM_H_
#define _C3HPPC_OCM_H_

#include <sal/appl/config.h>
#include <sal/appl/io.h>
#include <sal/types.h>
#include <soc/sbx/caladan3/ocm.h>
#include <appl/test/caladan3/c3hppc_utils.h>
#include <soc/types.h>
#include <soc/drv.h>

#define C3HPPC_NO_SEGMENT_TABLE           1

#define C3HPPC_LOGICAL_TO_PHYSICAL_SHIFT      14
#define C3HPPC_OCM_MAX_PHY_BLK                (128)
#define C3HPPC_OCM_PHY_BLK_PER_OCM            (64)
#define C3HPPC_OCM_MAX_PHY_BLK_SIZE_IN_ROWS   (0x4000)
#define C3HPPC_OCM_NUM_OF_TRANSFER_SIZES      (7)
#define C3HPPC_OCM_ALL_TRANSFER_SIZES         (C3HPPC_OCM_NUM_OF_TRANSFER_SIZES + 1)
#define C3HPPC_DATUM_SIZE_BIT                 0
#define C3HPPC_DATUM_SIZE_DBIT                1
#define C3HPPC_DATUM_SIZE_NIBBLE              2
#define C3HPPC_DATUM_SIZE_BYTE                3
#define C3HPPC_DATUM_SIZE_WORD                4
#define C3HPPC_DATUM_SIZE_LONGWORD            5
#define C3HPPC_DATUM_SIZE_QUADWORD            6

#define C3HPPC_ERROR_PROTECTION__NONE         0
#define C3HPPC_ERROR_PROTECTION__PARITY       1
#define C3HPPC_ERROR_PROTECTION__ECC          2

#define C3HPPC_NUM_OF_OCM_PORTS               (15)
#define C3HPPC_NUM_OF_OCM_LRP_PORTS           (10)
#define C3HPPC_NUM_OF_OCM_CMU_PORTS           (2)
#define C3HPPC_NUM_OF_OCM_COP_PORTS           (2)
#define C3HPPC_NUM_OF_OCM_BUBBLE_PORTS        (1)
#define C3HPPC_PHYSICAL_PROC_ACCESS           (C3HPPC_NUM_OF_OCM_PORTS + 1)
#define C3HPPC_RAW_PROC_ACCESS                (C3HPPC_PHYSICAL_PROC_ACCESS + 1)

typedef struct c3hppc_ocm_port_info_s {
  int      nStartingSegment;
  uint32   uSegmentTransferSize;
  uint32   uSegmentBase;
  uint32   uSegmentLimit;
  int      nStartingPhysicalBlock;
  char     bSegmentProtected;
  char     bValid;
} c3hppc_ocm_port_info_t;

typedef struct c3hppc_ocm_control_info_s {
  c3hppc_ocm_port_info_t    *pOcmPortInfo;
} c3hppc_ocm_control_info_t;


typedef struct c3hppc_64b_ocm_entry_template_s {
  uint32    uData[3];
} c3hppc_64b_ocm_entry_template_t;



int c3hppc_ocm_port_program_segment_table(int nUnit, int nOcmPort, int nSegment, 
                                          uint32 uSegmentTransferSize, uint32 uSegmentBase,
                                          uint32 uSegmentLimit, char bSegmentProtected);
int c3hppc_ocm_port_program_port_table(int nUnit, int nOcmPort, uint32 uSegmentBase, 
                                       uint32 uSegmentLimit, uint32 uSegmentTransferSize,
                                       int nStartingPhysicalBlock);
int c3hppc_ocm_port_modify_segment_error_protection(int nUnit, int nOcmPort, int nSegment, 
                                                    uint32 uProtectionScheme);




int c3hppc_ocm_hw_init( int nUnit, c3hppc_ocm_control_info_t *pC3OcmControlInfo );
int c3hppc_ocm_hw_cleanup( int nUnit );
int c3hppc_ocm_display_error_state( int nUnit );
int c3hppc_ocm_port_config( int nUnit, c3hppc_ocm_port_info_t *pOcmPortInfo);
int c3hppc_ocm_mem_read_write( int nUnit, int nOcmPort, int nSegment, uint32 uOffset,
                               uint8 bWrite, uint32 *puEntryData );
int c3hppc_ocm_dma_read_write( int nUnit, int nOcmPort, int nSegment, uint32 uStartOffset,
                               uint32 uEndOffset, uint8 bWrite, uint32 *puDmaData );
int c3hppc_ocm_map_lrp2ocm_port(int nLrpPort);
int c3hppc_ocm_map_cop2ocm_port(int nCopPort);
int c3hppc_ocm_map_cmu2ocm_port(int nCmuPort);
int c3hppc_ocm_map_bubble2ocm_port(int nBubblePort);
#endif /* _C3HPPC_OCM_H_ */
