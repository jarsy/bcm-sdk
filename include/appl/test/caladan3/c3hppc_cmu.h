/*
 * $Id: c3hppc_cmu.h,v 1.1 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * c3hppc_cmu.h : CMU defines
 *
 *-----------------------------------------------------------------------------*/
#ifndef _C3HPPC_CMU_H_
#define _C3HPPC_CMU_H_

#include <sal/appl/config.h>
#include <sal/appl/io.h>
#include <sal/types.h>
#include <appl/test/caladan3/c3hppc_utils.h>
#include <soc/types.h>
#include <soc/drv.h>


#define C3HPPC_CMU_SEGMENT_NUM                   (32) 
#define C3HPPC_CMU_OCM_PORT_NUM                  (2) 
#define C3HPPC_CMU_SEGMENT_TYPE__TURBO_64b       (0) 
#define C3HPPC_CMU_SEGMENT_TYPE__TURBO_32b       (1) 
#define C3HPPC_CMU_SEGMENT_TYPE__SIMPLE_64b      (2) 
#define C3HPPC_CMU_SEGMENT_TYPE__SIMPLE_32b      (3)

typedef struct c3hppc_cmu_segment_info_s {
  uint32   uSegment;
  uint32   uSegmentOcmBase;
  uint32   uSegmentLimit;
  uint32   uSegmentType;
  uint64   *pSegmentPciBase;
  uint32   uSegmentPort;
  int      nStartingPhysicalBlock;
  char     bValid;
} c3hppc_cmu_segment_info_t;

typedef struct c3hppc_cmu_control_info_s {
  c3hppc_cmu_segment_info_t    *pCmuSegmentInfo;
  uint32                       uLFSRseed;
} c3hppc_cmu_control_info_t;



int c3hppc_cmu_program_segment_table( int nUnit, int nSegment,
                                      c3hppc_cmu_segment_info_t *pCmuSegmentInfo );
int c3hppc_cmu_program_segment_enable( int nUnit, int nSegment );
int c3hppc_cmu_program_segment_ejection_enable( int nUnit, int nSegment );
int c3hppc_cmu_segment_flush( int nUnit, c3hppc_cmu_segment_info_t *pCmuSegmentInfo );




int c3hppc_cmu_hw_init( int nUnit, c3hppc_cmu_control_info_t *pC3CmuControlInfo );
int c3hppc_cmu_hw_cleanup( int nUnit );
int c3hppc_cmu_segments_config( int nUnit, c3hppc_cmu_segment_info_t *pCmuSegmentInfo );
int c3hppc_cmu_segments_enable( int nUnit, c3hppc_cmu_segment_info_t *pCmuSegmentInfo );
int c3hppc_cmu_segments_ejection_enable( int nUnit, c3hppc_cmu_segment_info_t *pCmuSegmentInfo );
int c3hppc_cmu_segments_flush( int nUnit, c3hppc_cmu_segment_info_t *pCmuSegmentInfo );
int c3hppc_cmu_display_error_state( int nUnit );


#endif /* _C3HPPC_CMU_H_ */
