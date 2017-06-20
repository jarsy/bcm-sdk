/*
 * $Id: nlmrangemgr2.c,v 1.1.6.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


 
#include "nlmrangemgr.h"
  


/****************************************
 * Function : NlmRangeMgr2__pvt_WriteRangeReg
 *
 * Parameters:
 *  void *devMgr_p    =  Device Manager pointer 
    nlm_u32 address   =  Address of the Register
    nlm_u8 *data      =  Array containing the data to be written to the register 
    NlmReasonCode *o_reason = Location to hold the reason for failure
 *
 * Summary: 
 * This function internally calls Device Manager function which writes to Range Register
 * This function takes care of writing the specified value to the specified register 
 * of all the devices in cascade
 * If the application of Range Manager uses its own implementation of device specific functions
  instead of device manager then they should replace this function with their implementation 
  of writing to Register of the specified address in all the devices.
 ****************************************/
NlmErrNum_t NlmRangeMgr2__pvt_WriteRangeReg(
    void *devMgr_p,
    NlmBankNum bankNum,
    NlmPortNum portNum,
    nlm_u32 address,
    nlm_u8 *data,
    NlmReasonCode *o_reason
    )
{         
    nlm_u32 iter;
    NlmErrNum_t errNum;
    NlmDevShadowDevice* shadow_p = NULL;
    nlm_u32 rangeRegNum = address - NLM_REG_RANGE_A_BOUNDS(0);
    NlmSMTNum smtNum = NLMDEV_SMT_0;
   
    NlmDevMgr *devMgr_ptr = (NlmDevMgr *) devMgr_p;
    NlmDev  **dev_pp = (NlmDev**)devMgr_ptr->m_devList_pp;

    if(rangeRegNum >= NLMDEV_NUM_RANGE_REG)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_REG_ADDRESS;
        return NLMERR_FAIL;
    }

#ifndef NLM_12K_11K
    if(bankNum == NLMDEV_BANK_1 && devMgr_ptr->m_smtMode == NLMDEV_DUAL_SMT_MODE)
        smtNum = NLMDEV_SMT_1;
#else
    (void)bankNum;
#endif

    /* Writing to specified range register for all the devices in cascade */
    for(iter = 0; iter < devMgr_ptr->m_devCount; iter++)
    {        
        if(dev_pp[iter])
        {
            /* retrieving shadow device */
            shadow_p = (NlmDevShadowDevice*)dev_pp[iter]->m_shadowDevice_p;

            NlmCm__memcpy(shadow_p->m_rangeReg[rangeRegNum].m_data, data,
                                NLMDEV_REG_LEN_IN_BYTES);   
    
            if((errNum = kbp_dm_range_reg_write(dev_pp[iter],
                                                portNum,
                                                smtNum,
                                                address,
                                                &(shadow_p->m_rangeReg[rangeRegNum]),
                                                o_reason
                                                )) != NLMERR_OK)
                return errNum;
        }
    }/* End of for-loop */

    return NLMERR_OK;
}


static NlmErrNum_t 
NlmRangeMgr2__pvt_InitCodeRegisters(NlmRangeMgr *self,
                                    NlmBankNum bankNum,
                                    NlmRangeType rangeType,
                                    nlm_u32 rangeCodeValue,
                                    NlmReasonCode *o_reason
                                    )
{
    nlm_u8      segNum;
    nlm_u8      data[NLMDEV_REG_LEN_IN_BYTES];
    NlmErrNum_t errNum;
    
    /* Writing the values in Range Control Register */
    NlmCm__memset(data, 0, NLMDEV_REG_LEN_IN_BYTES);

    /* The 16b range field is divided into 8 segments of 2b each and there is 12b look-up code 
    for each segment; Here all the segments are assumed to have same value     */
    for(segNum = 0; segNum < 4; segNum++) 
    {
        WriteBitsInRegs(data,  segNum * 12 + 11,
                               segNum * 12, 
                               rangeCodeValue);
    }
    
    if((errNum = NlmRangeMgr2__pvt_WriteRangeReg(self->m_devMgr_p,
                                               bankNum,
                                               self->m_portNum,
                                               (NLM_REG_RANGE_A_CODE0 + rangeType * 2),
                                               data,
                                               o_reason)) != NLMERR_OK)
            return errNum;      

    if((errNum = NlmRangeMgr2__pvt_WriteRangeReg(self->m_devMgr_p,
                                               bankNum,
                                               self->m_portNum,
                                               (NLM_REG_RANGE_A_CODE1 + rangeType * 2),
                                               data,
                                               o_reason)) != NLMERR_OK)
            return errNum;      
    
    if(o_reason)
        *o_reason = NLMRSC_REASON_OK;
    return NLMERR_OK;    
}

static NlmErrNum_t 
NlmRangeMgr2__pvt_InitBoundRegisters(NlmRangeMgr *self,
                                     NlmBankNum bankNum,
                                     nlm_u16 topRanges[][NLM_RANGE_NUM_BOUNDS],
                                     nlm_u8 validBits,
                                     NlmRangeType rangeType,                   
                                     NlmReasonCode *o_reason
                                     )
{
    nlm_u8      iter;
    nlm_u8      data[NLMDEV_REG_LEN_IN_BYTES];
    NlmErrNum_t errNum;
    nlm_u32 bitWidthSelect;
    nlm_u32 regAddr;

    /* Get the base address of Range Bound Reg based on Reg Type */
    switch(rangeType)
    {
        case NLM_RANGE_TYPE_A:
            regAddr = NLM_REG_RANGE_A_BOUNDS(0); 
            break;
        case NLM_RANGE_TYPE_B:
            regAddr = NLM_REG_RANGE_B_BOUNDS(0); 
            break;
        case NLM_RANGE_TYPE_C:
            regAddr = NLM_REG_RANGE_C_BOUNDS(0); 
            break;
        case NLM_RANGE_TYPE_D:
            regAddr = NLM_REG_RANGE_D_BOUNDS(0); 
            break;
        default:
             NlmCmAssert(0, "Invalid Range Type");
             return NLMERR_FAIL;
    }    

    /* Writing the values in Range Control Register */
    NlmCm__memset(data, 0, NLMDEV_REG_LEN_IN_BYTES);
    bitWidthSelect = (validBits >> 2) - 1;

    for(iter = 0; iter < NLMDEV_RANGE_NUM_MCOR_PER_RANGE_TYPE; iter++)
    {
        WriteBitsInRegs(data, 31, 16, topRanges[iter][1]);
        WriteBitsInRegs(data, 15, 0, topRanges[iter][0]);
        WriteBitsInRegs(data, 33, 32, bitWidthSelect);

        if((errNum = NlmRangeMgr2__pvt_WriteRangeReg(self->m_devMgr_p,
                                               bankNum,
                                               self->m_portNum,
                                               regAddr + iter,
                                               data,
                                               o_reason)) != NLMERR_OK)
            return errNum;      
    }       
    
    if(o_reason)
        *o_reason = NLMRSC_REASON_OK;
    return NLMERR_OK;    
}

static NlmErrNum_t
NlmRangeMgr2__pvt_CheckRangeSrchAttrs(NlmRangeDb *rangeDb,
                                        nlm_u8 *rangeInsertStartByte,
                                        nlm_u8 rangeExtractStartByte,
                                        nlm_u8 *rangeUsedFlag,
                                        NlmReasonCode *o_reason
                                        )
{
    nlm_u8 keyNum;    

    for(keyNum = 0; keyNum < NLMDEV_NUM_KEYS; keyNum++)
    {
        if(rangeInsertStartByte[keyNum] <= (NLMDEV_MAX_CB_WRITE_IN_BYTES - 2))
        {
            *rangeUsedFlag = NlmTrue;
            break;        
        }
    }
    if(keyNum != NLMDEV_NUM_KEYS)
    {
        if(rangeDb == NULL)
        {
            /* If any of the key is using the specified Range Type but application has not
                specified DB then throw error */
            if(o_reason)
                *o_reason = NLMRSC_INVALID_RANGE_ATTRIBUTES;
            return NLMERR_FAIL;
        }
        else
        {            
            if(rangeExtractStartByte > (NLMDEV_MAX_CB_WRITE_IN_BYTES - 2))
            {
                /* If any of the key is using the Range A but extaction start Byte is not correct then throw error*/
                if(o_reason)
                    *o_reason = NLMRSC_INVALID_RANGE_ATTRIBUTES;
                return NLMERR_FAIL;
            }
        }                
    }
    return NLMERR_OK;
}

static NlmErrNum_t 
NlmRangeMgr2__pvt_InitRangeLtr(NlmRangeMgr *self,
                             nlm_u8 ltrNum,
                             NlmRangeSrchAttrs *srchAttrs,                             
                             NlmReasonCode *o_reason)
{
    NlmErrNum_t err;
    nlm_u8 keyNum = 0;

    nlm_u8 rangeAUsedFlag = NlmFalse;
    nlm_u8 rangeBUsedFlag = NlmFalse;
    nlm_u8 rangeCUsedFlag = NlmFalse;
    nlm_u8 rangeDUsedFlag = NlmFalse;

#ifndef NLM_12K_11K
    if(ltrNum >= NLMDEV_MAX_NUM_LTRS)
#else 
    if(ltrNum >= (NLMDEV_MAX_NUM_LTRS/2))
#endif
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_LTR_NUM;
        return NLMERR_FAIL;
    }

    if((NLMERR_OK != NlmRangeMgr2__pvt_CheckRangeSrchAttrs(srchAttrs->m_rangeA_db,
                                                             srchAttrs->m_keyInsert_startByte_rangeA, 
                                                             srchAttrs->m_extraction_startByte_rangeA,
                                                             &rangeAUsedFlag,
                                                             o_reason
                                                             )))
        return NLMERR_FAIL;

    if((NLMERR_OK != NlmRangeMgr2__pvt_CheckRangeSrchAttrs(srchAttrs->m_rangeB_db,
                                                             srchAttrs->m_keyInsert_startByte_rangeB, 
                                                             srchAttrs->m_extraction_startByte_rangeB,
                                                             &rangeBUsedFlag,
                                                             o_reason
                                                             )))
        return NLMERR_FAIL;

     if((NLMERR_OK != NlmRangeMgr2__pvt_CheckRangeSrchAttrs(srchAttrs->m_rangeC_db,
                                                             srchAttrs->m_keyInsert_startByte_rangeC, 
                                                             srchAttrs->m_extraction_startByte_rangeC,
                                                             &rangeCUsedFlag,
                                                             o_reason
                                                             )))
        return NLMERR_FAIL;
     
     if((NLMERR_OK != NlmRangeMgr2__pvt_CheckRangeSrchAttrs(srchAttrs->m_rangeD_db,
                                                             srchAttrs->m_keyInsert_startByte_rangeD, 
                                                             srchAttrs->m_extraction_startByte_rangeD,
                                                             &rangeDUsedFlag,
                                                             o_reason
                                                             )))
        return NLMERR_FAIL; 

     /* process further */
    {
        NlmDevRangeInsertion0Reg *rangeInsert0LtrData_p;
        NlmDevRangeInsertion1Reg *rangeInsert1LtrData_p;
        NlmDevExtCap0Reg *rangeExtractLtrData_p; 
        NlmDevMgr *devMgr_p;        
        nlm_u8 devNum;
        NlmDevShadowDevice *shadowDev_p; 
        nlm_u8 portNum = self->m_portNum;
        devMgr_p = (NlmDevMgr*)self->m_devMgr_p;

#if defined NLM_MT_OLD || defined NLM_MT
        
    NlmCmMt__SpinLock(&devMgr_p->m_spinLock);
#endif


        for(devNum =0; devNum < devMgr_p->m_devCount; devNum++)
        {
            shadowDev_p = NLM_GET_SHADOW_MEM_FROM_DEVMGR_PTR(devMgr_p, devNum);
            rangeInsert0LtrData_p = &shadowDev_p->m_ltr[ltrNum].m_rangeInsert0;
            rangeInsert1LtrData_p = &shadowDev_p->m_ltr[ltrNum].m_rangeInsert1;
            rangeExtractLtrData_p = &shadowDev_p->m_ltr[ltrNum].m_extCap0;

            /* Clear the LTR Registers data  */
            NlmCm__memset(rangeInsert0LtrData_p, 0, sizeof(rangeInsert0LtrData_p));
            NlmCm__memset(rangeInsert1LtrData_p, 0, sizeof(rangeInsert1LtrData_p)); 

            rangeExtractLtrData_p->m_rangeAExtractStartByte = 0;
            rangeExtractLtrData_p->m_rangeBExtractStartByte = 0;
            rangeExtractLtrData_p->m_rangeCExtractStartByte = 0;
            rangeExtractLtrData_p->m_rangeDExtractStartByte = 0;

            /* Initialization of Range Insert Fields for each key and for each range */
            for(keyNum = 0; keyNum < NLMDEV_NUM_KEYS; keyNum++)
            {
                if(rangeAUsedFlag == NlmFalse)                
                    rangeInsert0LtrData_p->m_rangeAInsertStartByte[keyNum] = NLMDEV_RANGE_DO_NOT_INSERT;
                else 
                    rangeInsert0LtrData_p->m_rangeAInsertStartByte[keyNum] = srchAttrs->m_keyInsert_startByte_rangeA[keyNum];
                
                if(rangeBUsedFlag == NlmFalse)                
                    rangeInsert0LtrData_p->m_rangeBInsertStartByte[keyNum] = NLMDEV_RANGE_DO_NOT_INSERT;
                else
                    rangeInsert0LtrData_p->m_rangeBInsertStartByte[keyNum] = srchAttrs->m_keyInsert_startByte_rangeB[keyNum];

                if(rangeCUsedFlag == NlmFalse)                
                    rangeInsert1LtrData_p->m_rangeCInsertStartByte[keyNum] = NLMDEV_RANGE_DO_NOT_INSERT;
                else
                    rangeInsert1LtrData_p->m_rangeCInsertStartByte[keyNum] = srchAttrs->m_keyInsert_startByte_rangeC[keyNum];

                if(rangeDUsedFlag == NlmFalse)                
                    rangeInsert1LtrData_p->m_rangeDInsertStartByte[keyNum] = NLMDEV_RANGE_DO_NOT_INSERT;                
                else
                    rangeInsert1LtrData_p->m_rangeDInsertStartByte[keyNum] = srchAttrs->m_keyInsert_startByte_rangeD[keyNum];                
            }                      
            
            /* Initialization of range fields such as extract byte, range encoding type and num of bytes 
                of insertion for each range type. */
            if(rangeAUsedFlag == NlmTrue)
            {
                rangeExtractLtrData_p->m_rangeAExtractStartByte = srchAttrs->m_extraction_startByte_rangeA;                
                rangeInsert0LtrData_p->m_rangeAEncodedBytes = 
                    (NlmDevRangeEncodedValueBytes)((srchAttrs->m_rangeA_db->m_num_bits/8) - 1);
                rangeInsert0LtrData_p->m_rangeAEncodingType = 
                    (NlmDevRangeEncodingType)srchAttrs->m_rangeA_db->m_range_code;                      
            }           

            if(rangeBUsedFlag == NlmTrue)
            {
                rangeExtractLtrData_p->m_rangeBExtractStartByte = srchAttrs->m_extraction_startByte_rangeB;                
                rangeInsert0LtrData_p->m_rangeBEncodedBytes = 
                    (NlmDevRangeEncodedValueBytes)((srchAttrs->m_rangeB_db->m_num_bits/8) - 1);
                rangeInsert0LtrData_p->m_rangeBEncodingType = 
                    (NlmDevRangeEncodingType)srchAttrs->m_rangeB_db->m_range_code;                   
            }
            
            if(rangeCUsedFlag == NlmTrue)
            {
                rangeExtractLtrData_p->m_rangeCExtractStartByte = srchAttrs->m_extraction_startByte_rangeC;                
                rangeInsert1LtrData_p->m_rangeCEncodedBytes = 
                    (NlmDevRangeEncodedValueBytes)((srchAttrs->m_rangeC_db->m_num_bits/8) - 1);
                rangeInsert1LtrData_p->m_rangeCEncodingType = 
                    (NlmDevRangeEncodingType)srchAttrs->m_rangeC_db->m_range_code;                                           
            }
            
            if(rangeDUsedFlag == NlmTrue)
            {
                rangeExtractLtrData_p->m_rangeDExtractStartByte = srchAttrs->m_extraction_startByte_rangeD;                
                rangeInsert1LtrData_p->m_rangeDEncodedBytes = 
                    (NlmDevRangeEncodedValueBytes)((srchAttrs->m_rangeD_db->m_num_bits/8) - 1);
                rangeInsert1LtrData_p->m_rangeDEncodingType = 
                    (NlmDevRangeEncodingType)srchAttrs->m_rangeD_db->m_range_code;                                           
            }
            
            /* Write to the range ltr registers of the specified device using the Device manager API */
            if((err = kbp_dm_ltr_write((NlmDev*)devMgr_p->m_devList_pp[devNum],
                                                              portNum,
                                                              ltrNum, 
                                                              NLMDEV_RANGE_INSERTION_0_LTR,
                                                              rangeInsert0LtrData_p,
                                                              o_reason)) != NLMERR_OK)
            {
#if defined NLM_MT_OLD || defined NLM_MT
                NlmCmMt__SpinUnlock(&devMgr_p->m_spinLock);
#endif
                return err;
            }

            if((err = kbp_dm_ltr_write((NlmDev*)devMgr_p->m_devList_pp[devNum],
                                                              portNum,
                                                              ltrNum, 
                                                              NLMDEV_RANGE_INSERTION_1_LTR,
                                                              rangeInsert1LtrData_p,
                                                              o_reason)) != NLMERR_OK)
            {
#if defined NLM_MT_OLD || defined NLM_MT
                NlmCmMt__SpinUnlock(&devMgr_p->m_spinLock);
#endif
                return err;
            }

            if((err = kbp_dm_ltr_write((NlmDev*)devMgr_p->m_devList_pp[devNum],
                                                              portNum,
                                                              ltrNum, 
                                                              NLMDEV_EXT_CAPABILITY_REG_0_LTR,
                                                              rangeExtractLtrData_p,
                                                              o_reason)) != NLMERR_OK)
            {
#if defined NLM_MT_OLD || defined NLM_MT
                NlmCmMt__SpinUnlock(&devMgr_p->m_spinLock);
#endif
                return err;                                 
            }
        }

#if defined NLM_MT_OLD || defined NLM_MT
    NlmCmMt__SpinUnlock(&devMgr_p->m_spinLock);
#endif
    }

    return NLMERR_OK;
}

/****************************************
 * Function : NlmRangeMgr2__pvt_Init
 *
 * Parameters:
 * NlmRangeMgr *self        = Pointer to the Range Manager
 * NlmReasonCode *o_reason  = Location to save the reason code
 *
 * Summary: * 
 * This function hooks the device specific functions to the Range Manager
 ****************************************/
NlmErrNum_t NlmRangeMgr2__pvt_Init(
    NlmRangeMgr         *self,
    NlmReasonCode    *o_reason
    )
{
    nlm_u8 ltrNum;
    nlm_u8 keyNum;
    NlmRangeSrchAttrs rangeSrchAttrs;
    NlmErrNum_t catchErr;
    
    /* Check the inputs. */
    if(NULL == self)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_RANGE_MGR;
        return NLMERR_NULL_PTR;
    }


    if(NULL == self->m_devMgr_p) /* Check if devmgr is not null */
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_DEVMGR_PTR;
        return NLMERR_NULL_PTR;
    }

    self->m_vtbl.m_initCodeRegsFnPtr    =   NlmRangeMgr2__pvt_InitCodeRegisters;
    self->m_vtbl.m_initBoundRegsFnPtr = NlmRangeMgr2__pvt_InitBoundRegisters;
    self->m_vtbl.m_initRangeLtrFnPtr = NlmRangeMgr2__pvt_InitRangeLtr;
    
    /* Initialize all LTRs with the Range Insert Values as DO_NOT_INSERT; This will
    allow the user to avoid the calls to the RangeMatchingLTRConfig API for LTRs which
    do not use Range */
    for(keyNum = 0; keyNum < NLMDEV_NUM_KEYS; keyNum++)
    {
        rangeSrchAttrs.m_keyInsert_startByte_rangeA[keyNum] = NLMDEV_RANGE_DO_NOT_INSERT;
        rangeSrchAttrs.m_keyInsert_startByte_rangeB[keyNum] = NLMDEV_RANGE_DO_NOT_INSERT;
        rangeSrchAttrs.m_keyInsert_startByte_rangeC[keyNum] = NLMDEV_RANGE_DO_NOT_INSERT;
        rangeSrchAttrs.m_keyInsert_startByte_rangeD[keyNum] = NLMDEV_RANGE_DO_NOT_INSERT;
    }

    for(ltrNum = 0; ltrNum < NLMDEV_MAX_NUM_LTRS; ltrNum++)
    {
#ifdef NLM_12K_11K
        if(((NlmDevMgr*)self->m_devMgr_p)->m_11kDevMgr_p)
            if(ltrNum == (NLMDEV_MAX_NUM_LTRS/2))
                break;
#endif
        if(NLMERR_OK != (catchErr = NlmRangeMgr2__pvt_InitRangeLtr(self,
                                                                ltrNum,
                                                                &rangeSrchAttrs,
                                                                o_reason
                                                                )))
            return catchErr;     
    }

    if(o_reason)
        *o_reason = NLMRSC_REASON_OK;
    return NLMERR_OK;
}

