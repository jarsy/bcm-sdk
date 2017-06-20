/* $Id: ocm_ut.c,v 1.10 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include <soc/defs.h>

#if defined(BCM_CALADAN3_SUPPORT)

#include "../c3sw_test.h"
#include <soc/sbx/caladan3/ocm.h>
#include <soc/error.h>
#include <soc/debug.h>


sbx_caladan3_ocm_port_alloc_t _ocm_ut_test_desc[] = {
    {SOC_SBX_CALADAN3_OCM_LRP4_PORT,  5, 1024, SOC_SBX_CALADAN3_DATUM_SIZE_QUADWORD},
    {SOC_SBX_CALADAN3_OCM_LRP4_PORT,  30, 1024, SOC_SBX_CALADAN3_DATUM_SIZE_QUADWORD},
    {SOC_SBX_CALADAN3_OCM_LRP0_PORT,  1, 20 * 1024, SOC_SBX_CALADAN3_DATUM_SIZE_QUADWORD},
    {SOC_SBX_CALADAN3_OCM_LRP0_PORT,  2, 2 * 1024, SOC_SBX_CALADAN3_DATUM_SIZE_BIT},
    {SOC_SBX_CALADAN3_OCM_LRP9_PORT, 15, 16 * 1024, SOC_SBX_CALADAN3_DATUM_SIZE_LONGWORD},
    {SOC_SBX_CALADAN3_OCM_COP1_PORT, -1, 64 * 1024, SOC_SBX_CALADAN3_DATUM_SIZE_QUADWORD}
};

#define _C3_DMA_SEGMENT_ID (0) /* use segment 0 for dma */

#define _C3_OCM_MAX_TEST_LIST_SIZE (sizeof(_ocm_ut_test_desc)/sizeof(sbx_caladan3_ocm_port_alloc_t))
#define _C3_OCM_PATTERN (0xdeadbeef)
#define _C3_OCM_BUF_SIZE (4096)
static int ocm_ut_test_result;


int
c3_ut_ocm_test1_init(c3sw_test_info_t *pc3swTestInfo, void *pUserData)
{
    int rv, index, unit;
    sbx_caladan3_ocm_port_alloc_t *desc;

    unit = pc3swTestInfo->unit;

    rv = soc_sbx_caladan3_ocm_driver_init(pc3swTestInfo->unit);
    if (SOC_FAILURE(rv) && rv != SOC_E_INIT) {
        cli_out("C3 %d OCM driver init failed\n",pc3swTestInfo->unit);
        return rv;
    } else {
        rv = SOC_E_NONE;
        for (index=0; index < _C3_OCM_MAX_TEST_LIST_SIZE; index++) {
          desc = &_ocm_ut_test_desc[index];
          if (desc->segment >= 0) {
              rv = soc_sbx_caladan3_ocm_port_segment_alloc(unit, desc);
          } else {
              rv = soc_sbx_caladan3_ocm_port_mem_alloc(unit, desc);
          }
          if (SOC_FAILURE(rv)) {
              cli_out("C3 %d Allocation failed for port %d\n",unit, desc->port);
              return rv;
          }
          /*soc_sbx_caladan3_ocm_util_allocator_dump(unit, desc->port);*/
        }
    }
    return SOC_E_NONE;
}

static
int c3_ut_ocm_verify_access(int unit, int testindex) 
{
    int rv=SOC_E_NONE, index;
    uint32 *write_buf;
    uint32 *read_buf;
    sbx_caladan3_ocm_port_alloc_t *desc = &_ocm_ut_test_desc[testindex];
    int bufidx;

    /* Set & read back memory */
    write_buf = sal_alloc(_C3_OCM_BUF_SIZE*sizeof(uint32), "write buff");
    if (write_buf == NULL) {
	return SOC_E_FAIL;
    }
    read_buf = sal_alloc(_C3_OCM_BUF_SIZE*sizeof(uint32), "read buff");
    if (read_buf == NULL) {
	if (write_buf) sal_free(write_buf);
	return SOC_E_FAIL;
    }
    sal_memset(&write_buf[0], 0, _C3_OCM_BUF_SIZE*sizeof(uint32));
    sal_memset(&read_buf[0], 0, _C3_OCM_BUF_SIZE*sizeof(uint32));

    bufidx = desc->datum_size/32;
    if (desc->datum_size % 32 > 0) {
        bufidx++;
    }

    for (index=0; index < bufidx; index++) {
        write_buf[index] = _C3_OCM_PATTERN;
        if (desc->datum_size < SOC_SBX_CALADAN3_DATUM_SIZE_LONGWORD) {
             write_buf[index] &= ((1<<desc->datum_size) - 1);
        }
    }

    rv = soc_sbx_caladan3_ocm_port_mem_write(unit, desc->port, desc->segment, 
                                             10, 10, &write_buf[0]);
    if (SOC_FAILURE(rv)) {
        cli_out("C3 %d Write to port %d memory failed\n",unit, desc->port);
	if (write_buf) sal_free(write_buf);
	if (read_buf) sal_free(read_buf);
        return rv;
    } 
   
    rv = soc_sbx_caladan3_ocm_port_mem_read(unit, desc->port, desc->segment, 
                                            10, 10, &read_buf[0]);
    if (SOC_FAILURE(rv)) {
        cli_out("C3 %d Read to port %d memory failed\n",unit, desc->port);
	if (write_buf) sal_free(write_buf);
	if (read_buf) sal_free(read_buf);
        return rv;
    } 

    cli_out("Written Value for Port %d: \n", desc->port);
    for (index=0; index < bufidx; index++) {
        cli_out("0x%x \t", write_buf[index]);
    }

    cli_out("\n Read Value for Port %d: \n", desc->port);
    for (index=0; index < bufidx; index++) {
        cli_out("0x%x \t", read_buf[index]);
    }

    if(sal_memcmp(&write_buf[0], &read_buf[0], bufidx) != 0) {
        cli_out("Mismatch:%d Memory Read does not compare to written value"
                " to port %d memory failed\n",unit,desc->port);
	if (write_buf) sal_free(write_buf);
	if (read_buf) sal_free(read_buf);
        return SOC_E_FAIL;
    } 

    if (write_buf) sal_free(write_buf);
    if (read_buf) sal_free(read_buf);
    return rv;
}

int
c3_ut_ocm_test1_run(c3sw_test_info_t *pc3swTestInfo, void *pUserData)
{
    int rv, unit, index;
    sbx_caladan3_ocm_port_alloc_t *desc;

    ocm_ut_test_result = -1;
    unit = pc3swTestInfo->unit;

    for (index=0; index < _C3_OCM_MAX_TEST_LIST_SIZE; index++) {
        desc = &_ocm_ut_test_desc[index];
        rv = c3_ut_ocm_verify_access(unit, index);
        if (SOC_FAILURE(rv)) {
            cli_out("FAIL!!!! C3 %d Memory Test failed for port %d\n",unit, desc->port);
            return rv;
        } else {
            cli_out("$PASS \n");
        }
    }

    ocm_ut_test_result = 0;
    return 0;
}

int
c3_ut_ocm_test1_done(c3sw_test_info_t *pc3swTestInfo, void *pUserData)
{
    int rv, index, unit;
    sbx_caladan3_ocm_port_alloc_t *desc;

    if (ocm_ut_test_result < 0) {
        /* dont cleanup, helps debugging */
        return -1;
    }

    unit = pc3swTestInfo->unit;

    for (index=0; index < _C3_OCM_MAX_TEST_LIST_SIZE; index++) {
        desc = &_ocm_ut_test_desc[index];
        cli_out("\n Freeing port %d memories", desc->port);
        if (desc->segment >= 0) {
            rv = soc_sbx_caladan3_ocm_port_segment_free(unit, desc);
        } else {
            rv = soc_sbx_caladan3_ocm_port_mem_free(unit, desc);
        }
        if (SOC_FAILURE(rv)) {
            cli_out("C3 %d Free failed for port %d\n",unit, desc->port);
            return rv;
        }
        /*soc_sbx_caladan3_ocm_util_allocator_dump(unit, desc->port);*/
    }

    return 0;
}

static
int c3_ut_ocm_verify_dma_access(int unit, int testindex) 
{
    int rv=SOC_E_NONE, index;
    uint32 *write_buf;
    uint32 *read_buf;
    sbx_caladan3_ocm_port_alloc_t *desc = &_ocm_ut_test_desc[testindex];
    unsigned int maxidx;

    /* Set & read back memory */
    write_buf = sal_alloc(_C3_OCM_BUF_SIZE*sizeof(uint32), "write buff");
    if (write_buf == NULL) {
	return SOC_E_FAIL;
    }
    read_buf = sal_alloc(_C3_OCM_BUF_SIZE*sizeof(uint32), "read buff");
    if (read_buf == NULL) {
	if (write_buf) sal_free(write_buf);
	return SOC_E_FAIL;
    }
    sal_memset(&write_buf[0], 0, _C3_OCM_BUF_SIZE*sizeof(uint32));
    sal_memset(&read_buf[0], 0, _C3_OCM_BUF_SIZE*sizeof(uint32));

    maxidx = (desc->size * desc->datum_size)/32;
    if (maxidx > _C3_OCM_BUF_SIZE) maxidx = _C3_OCM_BUF_SIZE;

    for (index=0; index < maxidx; index++) {
        write_buf[index] = index;
    }

    rv = soc_sbx_caladan3_ocm_port_mem_write(unit, desc->port, desc->segment, 
                                             0, ((maxidx*32)/desc->datum_size)-1, &write_buf[0]);
    if (SOC_FAILURE(rv)) {
        cli_out("C3 %d write to port %d memory failed\n",unit, desc->port);
	if (write_buf) sal_free(write_buf);
	if (read_buf) sal_free(read_buf);
        return rv;
    } 
   
    rv = soc_sbx_caladan3_ocm_port_mem_read(unit, desc->port, desc->segment, 
                                            0, ((maxidx*32)/desc->datum_size)-1, &read_buf[0]);
    if (SOC_FAILURE(rv)) {
        cli_out("C3 %d write to port %d memory failed\n",unit, desc->port);
	if (write_buf) sal_free(write_buf);
	if (read_buf) sal_free(read_buf);
        return rv;
    } 
#if 0
    cli_out("Written Value for Port %d: \n", desc->port);
    for (index=0; index < maxidx; index++) {
        cli_out("0x%x \t", write_buf[index]);
    }

    cli_out("\n Read Value for Port %d: \n", desc->port);
    for (index=0; index < maxidx; index++) {
        cli_out("0x%x \t", read_buf[index]);
    }
#endif
    if(sal_memcmp(&write_buf[0], &read_buf[0], maxidx) != 0) {
        cli_out("Mismatch:%d Memory Read does not compare to written value"
                " to port %d memory failed\n",unit,desc->port);
	if (write_buf) sal_free(write_buf);
	if (read_buf) sal_free(read_buf);
        return SOC_E_FAIL;
    } 
    if (write_buf) sal_free(write_buf);
    if (read_buf) sal_free(read_buf);
    return rv;
}

int
c3_ut_ocm_dma_test_run(c3sw_test_info_t *pc3swTestInfo, void *pUserData)
{
    int rv, unit, index;
    ocm_ut_test_result = -1;

    unit = pc3swTestInfo->unit;

    for (index=0; index < _C3_OCM_MAX_TEST_LIST_SIZE; index++) {
        rv = c3_ut_ocm_verify_dma_access(unit, index);
        if (SOC_FAILURE(rv)) {
            cli_out("FAIL!!!! C3 %d DMA OCM Memory Test failed\n",unit);
            return rv;
        } else {
            cli_out("$PASS \n");
        }
    }

    ocm_ut_test_result = 0;
    return 0;
}


#endif /* #ifdef BCM_CALADAN3_SUPPORT */
