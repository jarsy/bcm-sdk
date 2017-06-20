/*
 * $Id: fe1600_fabric_cell_snake_test.h,v 1.6 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * FE1600 FABRIC CELL SNAKE TEST H
 */
 
#ifndef _SOC_FE1600_FABRIC_CELL_SNAKE_TEST_H_
#define _SOC_FE1600_FABRIC_CELL_SNAKE_TEST_H_

#include <soc/dfe/cmn/dfe_fabric_cell_snake_test.h>
#include <soc/dfe/cmn/dfe_drv.h>
#include <sal/core/time.h>
#include <soc/error.h>


#ifdef BCM_88754_A0
#define SOC_FE1600_CELL_SNAKE_TEST_BCM88754_EXTERNAL_LOOPBACK               (0)
#define SOC_FE1600_CELL_SNAKE_TEST_BCM88754_MAC_LOOPBACK                    (1)
#define SOC_FE1600_CELL_SNAKE_TEST_BCM88754_SIF_CORE_LOOPBACK               (2)
#define SOC_FE1600_CELL_SNAKE_TEST_BCM88754_SIF_SERDES_LOOPBACK             (3)
#endif /*BCM_88754_A0*/

int
  soc_fe1600_cell_snake_test_prepare(
    int unit, 
    uint32 flags);
    
int
  soc_fe1600_cell_snake_test_run(
    int unit, 
    uint32 flags, 
    soc_fabric_cell_snake_test_results_t* results);

 
#endif /*_SOC_FE1600_FABRIC_CELL_SNAKE_TEST_H_*/
