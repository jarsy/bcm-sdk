/*
 * $Id: dfe_fabric_cell_snake_test.h,v 1.8 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * DFE FABRIC CELL SNAKE TEST H
 */
 
#ifndef _SOC_DFE_FABRIC_CELL_SNAKE_TEST_H_
#define _SOC_DFE_FABRIC_CELL_SNAKE_TEST_H_

#include <bcm/types.h>

/* using this leads to a recursive include #include <soc/dfe/cmn/dfe_drv.h> */


#define SOC_DFE_ENABLE_MAC_LOOPBACK         (0x1)
#define SOC_DFE_ENABLE_PHY_LOOPBACK         (0x0)
/* NOTE: all loops should be set at the same level. */ 
#define SOC_DFE_ENABLE_EXTERNAL_LOOPBACK    (0x2)
#define SOC_DFE_DONT_TOUCH_MAC_INTERRUPTS   (0x4)
#define SOC_DFE_ENABLE_ASYNC_FIFO_LOOPBACK  (0x8)


#define SOC_DFE_SNAKE_INFINITE_RUN 0x1
#define SOC_DFE_SNAKE_STOP_TEST 0x2 

/* Failure stage flags */
#define SOC_DFE_SNAKE_TEST_FAILURE_STAGE_REGISTER_CONFIG 0x1
#define SOC_DFE_SNAKE_TEST_FAILURE_STAGE_GET_OUT_OF_RESET 0x2
#define SOC_DFE_SNAKE_TEST_FAILURE_STAGE_DATA_CELL_GENERATION 0x4
#define SOC_DFE_SNAKE_TEST_FAILURE_STAGE_CONTROL_CELL_GENERATION 0x8
#define SOC_DFE_SNAKE_TEST_FAILURE_STAGE_DATA_CELL_FILTER_WRITE_COMMAND 0x10
#define SOC_DFE_SNAKE_TEST_FAILURE_STAGE_CONTROL_CELL_FILTER_WRITE_COMMAND 0x20
#define SOC_DFE_SNAKE_TEST_FAILURE_STAGE_DATA_CELL_FILTER_READ_COMMAND 0x40
#define SOC_DFE_SNAKE_TEST_FAILURE_STAGE_CONTROL_CELL_FILTER_READ_COMMAND 0x80

/*************
 * TYPE DEFS *
 *************/

typedef struct soc_fabric_cell_snake_test_results_s
{
    int                    test_failed;
    uint32                 interrupts_status[SOC_REG_ABOVE_64_MAX_SIZE_U32];
    uint32                 failure_stage_flags;
    int                    tdm_lfsr_value;                         /*Relevant for FE1600 only*/
    int                    non_tdm_lfsr_value;                     /*Relevant for FE1600 only*/
    int                    lfsr_per_pipe[SOC_DFE_MAX_NOF_PIPES];   /*Relevant from FE3200 and above*/
} soc_fabric_cell_snake_test_results_t;


/*************
 * FUNCTIONS *
 *************/

int
  soc_dfe_cell_snake_test_prepare(
    int unit, 
    uint32 flags);
    
int
  soc_dfe_cell_snake_test_run(
    int unit, 
    uint32 flags,
    soc_fabric_cell_snake_test_results_t* results);




#endif /*_SOC_DFE_FABRIC_CELL_SNAKE_TEST_H_*/
