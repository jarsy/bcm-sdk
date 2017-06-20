/*
 * $Id: t3p1_cmu_sim.c,v 1.1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * t3p1_cmu_sim.c: Guadalupe2k V1.3 CMU sim
 *
 */
 
#include <shared/bsl.h>

#include <sal/core/libc.h> 
#include <soc/types.h>
#include <soc/drv.h>

#if defined(BCM_CALADAN3_SUPPORT) && defined(BCM_CALADAN3_T3P1_SUPPORT)
#include <stdlib.h>

#include <soc/sbx/t3p1/t3p1_int.h>
#include <soc/sbx/t3p1/t3p1_tmu.h>
#include <soc/sbx/t3p1/t3p1_cmu_sim.h>
#include <soc/sbx/caladan3/simintf.h>

static char buffer[MAX_BUFFER_SIZE];

int soc_sbx_t3p1_cmu_counter_read_ext(int unit, int segment, int start, int numCounters, uint64 *counters) {
    int nextPrintPos = 0;
    int bufferSizeRemain = MAX_BUFFER_SIZE;
    int numCharPrinted;
    int i, rv, status;
    char *token;
    char *delimiters = " \t";
    uint64 cntr;
    char *tokstr=NULL;
        
    numCharPrinted = sal_snprintf(&buffer[nextPrintPos], bufferSizeRemain, "cmu read %d %d %d", segment, start, numCounters);
    nextPrintPos += numCharPrinted;
    bufferSizeRemain -= numCharPrinted;
    if (bufferSizeRemain == 0) return SOC_E_PARAM;
 
    rv = soc_sbx_caladan3_sim_sendrcv(unit, buffer, &nextPrintPos);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "sim sendrcv failed (%d)\n"), rv));
        return rv;
    }
    
    token = sal_strtok_r(buffer, delimiters, &tokstr);
    if ((token == NULL) || (sal_strcasecmp(token, "status"))){
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Recv format unrecognized: %s\n"), buffer));
        return SOC_E_PARAM;
    }
    token = sal_strtok_r(NULL, delimiters, &tokstr);
    if (token == NULL){
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Missing status value\n")));
        return SOC_E_PARAM;
    }    
    status = _shr_ctoi(token);
    if (status != 0) {
        return SOC_E_NOT_FOUND;
    }
    
    token = sal_strtok_r(NULL, delimiters, &tokstr);
    if ((token == NULL) || (sal_strcasecmp(token, "result"))){
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Missing result token\n")));
        return SOC_E_PARAM;
    }
    
    /* Simulator sends pkt count and byte counter */
    for (i=0; i<2*numCounters; i++) {
        token = sal_strtok_r(NULL, delimiters, &tokstr);
        if (token == NULL){
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Missing counter %d\n"), i));
            return SOC_E_PARAM;
        }
     
        COMPILER_64_SET(cntr,0, strtoul(token, NULL, 0));
        counters[i] = cntr;
    }
    
    return SOC_E_NONE;
}

int soc_sbx_t3p1_cmu_counter_clear_ext(int unit, int segment)  {
   int nextPrintPos = 0;
    int bufferSizeRemain = MAX_BUFFER_SIZE;
    int numCharPrinted;
    int rv, status;
    char *token;
    char *delimiters = " \t";
    char *tokstr=NULL;
      
    numCharPrinted = sal_snprintf(&buffer[nextPrintPos], bufferSizeRemain, "cmu clear %d", segment);
    nextPrintPos += numCharPrinted;
    bufferSizeRemain -= numCharPrinted;
    if (bufferSizeRemain == 0) return SOC_E_PARAM;
 
    rv = soc_sbx_caladan3_sim_sendrcv(unit, buffer, &nextPrintPos);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "sim sendrcv failed (%d)\n"), rv));
        return rv;
    }
    
    token = sal_strtok_r(buffer, delimiters, &tokstr);
    if ((token == NULL) || (sal_strcasecmp(token, "status"))){
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Recv format unrecognized: %s\n"), buffer));
        return SOC_E_PARAM;
    }
    token = sal_strtok_r(NULL, delimiters, &tokstr);
    if (token == NULL){
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Missing status value\n")));
        return SOC_E_PARAM;
    }    
    status = _shr_ctoi(token);
    if (status != 0) {
        return SOC_E_NOT_FOUND;
    }
     
    return SOC_E_NONE;
}
  
#endif
