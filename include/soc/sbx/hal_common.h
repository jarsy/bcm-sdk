
/*
 *$Id: hal_common.h,v 1.9 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */

#ifndef SAND_HAL_COMMON_H
#define SAND_HAL_COMMON_H

/* structure used for common memory tests */
#define _SAND_HAL_RT_RN_SIZE 64
typedef struct _halRegTestInfo {
  char          m_sRegName[_SAND_HAL_RT_RN_SIZE];
  unsigned int m_nOffset;
  unsigned int m_nNoWrMask;
  unsigned int m_nWrMask;
  unsigned int m_nDefaultValue;
  unsigned int m_nNoRdMask; /* ab 021102  */
} halRegTestInfo_t;

void InitRegTestInfo(halRegTestInfo_t *pRegInfo, char *sRegName, unsigned int nOffset,
                     unsigned int nNoWrMask, unsigned int nWrMask, unsigned int nDefaultValue);
unsigned int HalRegTest(unsigned int nBaseAddr, halRegTestInfo_t aRegs[], unsigned int nNumRegs, unsigned int nHalDevice);

/* Defines for register field types */
#define SAND_HAL_TYPE_READ                   (0x1)
#define SAND_HAL_TYPE_WRITE                  (0x2)
#define SAND_HAL_TYPE_WRITE_0_TO_CLEAR       (0x4)
#define SAND_HAL_TYPE_WRITE_1_TO_CLEAR       (0x8)
#define SAND_HAL_TYPE_READ_TO_CLEAR          (0x10)
#define SAND_HAL_TYPE_SELF_CLEARING          (0x20)
#define SAND_HAL_TYPE_PUNCH                  (0x40)
#define SAND_HAL_TYPE_WRITE_BLOCK_WRITABLE   (0x80)
#define SAND_HAL_TYPE_CLEAR_TO_READ_COUNTER  (0x100)
#define SAND_HAL_TYPE_CLEAR_TO_READ_COUNTER_COUNT_BY  (SAND_HAL_TYPE_CLEAR_TO_READ_COUNTER)
#define SAND_HAL_TYPE_WRITE_NOTIFY           (0x200)
#define SAND_HAL_TYPE_READ_NOTIFY            (0x400)
#define SAND_HAL_TYPE_WRITE_VALUE_TO_CLEAR   (0x800)


#ifndef SAND_HAL_REG_OFFSET
#define SAND_HAL_REG_OFFSET(device,reg) (SAND_HAL_##device##_##reg##_OFFSET)
#endif

#ifndef SAND_HAL_REG_OFFSET_STRIDE
#define SAND_HAL_REG_OFFSET_STRIDE(device,block,id,reg) (id*SAND_HAL_##device##_##block##_INSTANCE_ADDR_STRIDE+SAND_HAL_REG_OFFSET(device,reg))
#endif

#ifndef SAND_HAL_REG_NO_TEST_MASK
#define SAND_HAL_REG_NO_TEST_MASK(device,reg) (SAND_HAL_##device##_##reg##_NO_TEST_MASK)
#endif

#ifndef SAND_HAL_REG_MASK
#define SAND_HAL_REG_MASK(device,reg) (SAND_HAL_##device##_##reg##_MASK)
#endif

#ifndef SAND_HAL_REG_LSB
#define SAND_HAL_REG_LSB(device,reg) (SAND_HAL_##device##_##reg##_LSB)
#endif

#ifndef SAND_HAL_REG_MSB
#define SAND_HAL_REG_MSB(device,reg) (SAND_HAL_##device##_##reg##_MSB)
#endif

#ifndef SAND_HAL_FIELD_MASK
#define SAND_HAL_FIELD_MASK(device,reg,field) (SAND_HAL_##device##_##reg##_##field##_MASK)
#endif

#ifndef SAND_HAL_FIELD_SHIFT
#define SAND_HAL_FIELD_SHIFT(device,reg,field) (SAND_HAL_##device##_##reg##_##field##_SHIFT)
#endif

#ifndef SAND_HAL_FIELD_MSB
#define SAND_HAL_FIELD_MSB(device,reg,field) (SAND_HAL_##device##_##reg##_##field##_MSB)
#endif

#ifndef SAND_HAL_FIELD_LSB
#define SAND_HAL_FIELD_LSB(device,reg,field) (SAND_HAL_##device##_##reg##_##field##_LSB)
#endif

#ifndef SAND_HAL_FIELD_TYPE
#define SAND_HAL_FIELD_TYPE(device,reg,field) (SAND_HAL_##device##_##reg##_##field##_TYPE)
#endif

#ifndef SAND_HAL_FIELD_DEFAULT
#define SAND_HAL_FIELD_DEFAULT(device,reg,field) (SAND_HAL_##device##_##reg##_##field##_DEFAULT)
#endif

/* see ./gu2/hal_user.h for the lowest level r/w access macros */

/***************************************************************************
 ***************************************************************************
 ***                                                                     ***
 ***                  Derived Register Access Functions                  ***
 ***                                                                     ***
 ***************************************************************************
 ***************************************************************************/

/* Polling for indirect memory reads when using easy reload */
#ifndef SAND_HAL_READ_OFFS_POLL
#if defined(BCM_EASY_RELOAD_SUPPORT) || defined(BCM_WARM_BOOT_SUPPORT)
#define SAND_HAL_READ_OFFS_POLL(addr, offs) \
     (SOC_IS_RELOADING((int32)addr) || SOC_WARM_BOOT((int32)addr)) ? 0xffffffff: SAND_HAL_READ_OFFS(addr, offs)
#else
#define SAND_HAL_READ_OFFS_POLL(addr,offs) SAND_HAL_READ_OFFS(addr, offs)
#endif
#endif

#ifndef SAND_HAL_READ
#define SAND_HAL_READ(addr,device,reg) SAND_HAL_READ_OFFS((addr), SAND_HAL_REG_OFFSET(device,reg))
#endif

/* Polling for indirect memory reads when using easy reload */
#ifndef SAND_HAL_READ_POLL
#if defined(BCM_EASY_RELOAD_SUPPORT) || defined(BCM_WARM_BOOT_SUPPORT)
#define SAND_HAL_READ_POLL(addr, device, reg) \
    (SOC_IS_RELOADING((int32)addr) || SOC_WARM_BOOT((int32)addr)) ? 0xffffffff: SAND_HAL_READ_OFFS((addr), SAND_HAL_REG_OFFSET(device,reg))
#else
#define SAND_HAL_READ_POLL(addr,device,reg) SAND_HAL_READ_OFFS((addr), SAND_HAL_REG_OFFSET(device,reg))
#endif
#endif

#ifndef SAND_HAL_READ_STRIDE
#define SAND_HAL_READ_STRIDE(addr,device,block,id,reg) SAND_HAL_READ_OFFS((addr),(id*SAND_HAL_##device##_##block##_INSTANCE_ADDR_STRIDE+SAND_HAL_REG_OFFSET(device,reg)))
#endif

/* For easy reload */
#ifndef SAND_HAL_READ_STRIDE_POLL
#if defined(BCM_EASY_RELOAD_SUPPORT) || defined(BCM_WARM_BOOT_SUPPORT)
#define SAND_HAL_READ_STRIDE_POLL(addr,device,block,id,reg) \
(SOC_IS_RELOADING((int32)addr) || SOC_WARM_BOOT((int32)addr)) ? 0xffffffff: SAND_HAL_READ_OFFS((addr),(id*SAND_HAL_##device##_##block##_INSTANCE_ADDR_STRIDE+SAND_HAL_REG_OFFSET(device,reg)))
#else
#define SAND_HAL_READ_STRIDE_POLL(addr,device,block,id,reg) SAND_HAL_READ_OFFS((addr),(id*SAND_HAL_##device##_##block##_INSTANCE_ADDR_STRIDE+SAND_HAL_REG_OFFSET(device,reg)))
#endif
#endif

#ifndef SAND_HAL_READ_RAW
#define SAND_HAL_READ_RAW(addr,device,reg) SAND_HAL_READ_OFFS_RAW((addr),SAND_HAL_REG_OFFSET(device,reg))
#endif

#ifndef SAND_HAL_READ_STRIDE_RAW
#define SAND_HAL_READ_STRIDE_RAW(addr, device, block, id, reg) SAND_HAL_READ_OFFS_RAW((addr),(id*SAND_HAL_##device##_##block##_INSTANCE_ADDR_STRIDE+SAND_HAL_REG_OFFSET(device,reg)))
#endif

#ifndef SAND_HAL_READ_INDEX
#define SAND_HAL_READ_INDEX(addr,device,reg,index) SAND_HAL_READ_OFFS((addr),SAND_HAL_REG_OFFSET(device,reg)+(index)*4)
#endif

#ifndef SAND_HAL_READ_INDEX_RAW
#define SAND_HAL_READ_INDEX_RAW(addr,device,reg,index) SAND_HAL_READ_OFFS_RAW((addr)+SAND_HAL_REG_OFFSET(device,reg)+(index)*4)
#endif

#ifndef SAND_HAL_READ_INDEX_STRIDE
#define SAND_HAL_READ_INDEX_STRIDE(addr,device,block,id,reg,index) SAND_HAL_READ_OFFS((addr),SAND_HAL_REG_OFFSET_STRIDE(device,block,id,reg)+(index)*4)
#endif

#ifndef SAND_HAL_READ_INDEX_STRIDE_RAW
#define SAND_HAL_READ_INDEX_STRIDE_RAW(addr,device,block,id,reg,index) SAND_HAL_READ_OFFS_RAW((addr),SAND_HAL_REG_OFFSET_STRIDE(device,block,id,reg)+(index)*4)
#endif

/* For easy reload, there are some writes to read indirect memory we need to do */
#ifndef SAND_HAL_WRITE_EASY_RELOAD
#define SAND_HAL_WRITE_EASY_RELOAD(addr,device,reg,value) SAND_HAL_WRITE_OFFS_FORCE((addr),SAND_HAL_REG_OFFSET(device,reg),(value))
#endif

/* Force the write ignoring the easy-reload & Warm-boot states */
#ifndef SAND_HAL_WRITE_FORCE
#define SAND_HAL_WRITE_FORCE(addr,device,reg,value) SAND_HAL_WRITE_OFFS_FORCE((addr),SAND_HAL_REG_OFFSET(device,reg),(value))
#endif

#ifndef SAND_HAL_WRITE
#if defined(BCM_EASY_RELOAD_SUPPORT) || defined(BCM_WARM_BOOT_SUPPORT)
#define SAND_HAL_WRITE(addr,device,reg,value) (SOC_IS_RELOADING((int32)addr) || SOC_WARM_BOOT((int32)addr)) ? (void)0: SAND_HAL_WRITE_OFFS((addr),(SAND_HAL_REG_OFFSET(device,reg)),(value))
#else
#define SAND_HAL_WRITE(addr,device,reg,value)     SAND_HAL_WRITE_OFFS((addr),SAND_HAL_REG_OFFSET(device,reg),(value))
#endif
#endif


#ifndef SAND_HAL_WRITE_STRIDE
#if defined(BCM_EASY_RELOAD_SUPPORT) || defined(BCM_WARM_BOOT_SUPPORT)
#define SAND_HAL_WRITE_STRIDE(addr,device,block,id,reg,value) \
    (SOC_IS_RELOADING((int32)addr) || SOC_WARM_BOOT((int32)addr)) ? (void)0: SAND_HAL_WRITE_OFFS((addr),(id*SAND_HAL_##device##_##block##_INSTANCE_ADDR_STRIDE+SAND_HAL_REG_OFFSET(device,reg)),(value))
#else
#define SAND_HAL_WRITE_STRIDE(addr,device,block,id,reg,value) SAND_HAL_WRITE_OFFS((addr),(id*SAND_HAL_##device##_##block##_INSTANCE_ADDR_STRIDE+SAND_HAL_REG_OFFSET(device,reg)),(value))
#endif
#endif

/* For easy reload, there are some writes to read indirect memory that we need to do */
#ifndef SAND_HAL_WRITE_STRIDE_EASY_RELOAD
#define SAND_HAL_WRITE_STRIDE_EASY_RELOAD(addr,device,block,id,reg,value) SAND_HAL_WRITE_OFFS_FORCE((addr),(id*SAND_HAL_##device##_##block##_INSTANCE_ADDR_STRIDE+SAND_HAL_REG_OFFSET(device,reg)),(value))
#endif

/* Force the write ignoring the easy-reload & Warm-boot states */
#ifndef SAND_HAL_WRITE_STRIDE_FORCE
#define SAND_HAL_WRITE_STRIDE_FORCE(addr,device,block,id,reg,value) SAND_HAL_WRITE_OFFS_FORCE((addr),(id*SAND_HAL_##device##_##block##_INSTANCE_ADDR_STRIDE+SAND_HAL_REG_OFFSET(device,reg)),(value))
#endif

#ifndef SAND_HAL_WRITE_RAW
#if defined(BCM_EASY_RELOAD_SUPPORT) || defined(BCM_WARM_BOOT_SUPPORT)
#define SAND_HAL_WRITE_RAW(addr,device,reg,value) \
    (SOC_IS_RELOADING((int32)addr) || SOC_WARM_BOOT((int32)addr)) ? (void)0: SAND_HAL_WRITE_OFFS_RAW((addr),SAND_HAL_REG_OFFSET(device,reg),(value))
#else
#define SAND_HAL_WRITE_RAW(addr,device,reg,value) SAND_HAL_WRITE_OFFS_RAW((addr),SAND_HAL_REG_OFFSET(device,reg),(value))
#endif
#endif

#ifndef SAND_HAL_WRITE_STRIDE_RAW
#if defined(BCM_EASY_RELOAD_SUPPORT) || defined(BCM_WARM_BOOT_SUPPORT)
#define SAND_HAL_WRITE_STRIDE_RAW(addr,device,block,id,reg,value) \
    (SOC_IS_RELOADING((int32)addr) || SOC_WARM_BOOT((int32)addr)) ? (void)0: SAND_HAL_WRITE_OFFS_RAW((addr),(id*SAND_HAL_##device##_##block##_INSTANCE_ADDR_STRIDE+SAND_HAL_REG_OFFSET_RAW(device,reg)),(value))
#else
#define SAND_HAL_WRITE_STRIDE_RAW(addr,device,block,id,reg,value) SAND_HAL_WRITE_OFFS_RAW((addr),(id*SAND_HAL_##device##_##block##_INSTANCE_ADDR_STRIDE+SAND_HAL_REG_OFFSET_RAW(device,reg)),(value))
#endif
#endif

#ifndef SAND_HAL_WRITE_INDEX
#if defined(BCM_EASY_RELOAD_SUPPORT) || defined(BCM_WARM_BOOT_SUPPORT)
#define SAND_HAL_WRITE_INDEX(addr,device,reg,index,value) \
    (SOC_IS_RELOADING((int32)addr) || SOC_WARM_BOOT((int32)addr)) ?  (void)0: SAND_HAL_WRITE_OFFS((addr),SAND_HAL_REG_OFFSET(device,reg)+(index)*4,(value))
#else
#define SAND_HAL_WRITE_INDEX(addr,device,reg,index,value)  SAND_HAL_WRITE_OFFS((addr),SAND_HAL_REG_OFFSET(device,reg)+(index)*4,(value))
#endif
#endif

#ifndef SAND_HAL_WRITE_INDEX_RAW
#if defined(BCM_EASY_RELOAD_SUPPORT) || defined(BCM_WARM_BOOT_SUPPORT)
#define SAND_HAL_WRITE_INDEX_RAW(addr,device,reg,index,value) \
    (SOC_IS_RELOADING((int32)addr) || SOC_WARM_BOOT((int32)addr)) ? (void)0: SAND_HAL_WRITE_OFFS_RAW((addr),SAND_HAL_REG_OFFSET(device,reg)+(index)*4,(value))
#else
#define SAND_HAL_WRITE_INDEX_RAW(addr,device,reg,index,value) SAND_HAL_WRITE_OFFS_RAW((addr),SAND_HAL_REG_OFFSET(device,reg)+(index)*4,(value))
#endif
#endif

#ifndef SAND_HAL_WRITE_INDEX_STRIDE
#if defined(BCM_EASY_RELOAD_SUPPORT) || defined(BCM_WARM_BOOT_SUPPORT)
#define SAND_HAL_WRITE_INDEX_STRIDE(addr,device,block,id,reg,index,value) \
    (SOC_IS_RELOADING((int32)addr) || SOC_WARM_BOOT((int32)addr)) ? (void)0:  SAND_HAL_WRITE_OFFS((addr),SAND_HAL_REG_OFFSET_STRIDE(device,block,id,reg)+(index)*4,(value))
#else
#define SAND_HAL_WRITE_INDEX_STRIDE(addr,device,block,id,reg,index,value) SAND_HAL_WRITE_OFFS((addr),SAND_HAL_REG_OFFSET_STRIDE(device,block,id,reg)+(index)*4,(value))
#endif
#endif

#ifndef SAND_HAL_WRITE_INDEX_STRIDE_RAW
#if defined(BCM_EASY_RELOAD_SUPPORT) || defined(BCM_WARM_BOOT_SUPPORT)
#define SAND_HAL_WRITE_INDEX_STRIDE_RAW(addr,device,block,id,reg,index,value) \
    (SOC_IS_RELOADING((int32)addr) || SOC_WARM_BOOT((int32)addr)) ? (void)0: SAND_HAL_WRITE_OFFS_RAW((addr),SAND_HAL_REG_OFFSET_STRIDE(device,block,id,reg)+(index)*4,(value))
#else
#define SAND_HAL_WRITE_INDEX_STRIDE_RAW(addr,device,block,id,reg,index,value) SAND_HAL_WRITE_OFFS_RAW((addr),SAND_HAL_REG_OFFSET_STRIDE(device,block,id,reg)+(index)*4,(value))
#endif
#endif

#ifndef SAND_HAL_GET_FIELD
#define SAND_HAL_GET_FIELD(device,reg,field,fromValue) (((fromValue)&SAND_HAL_FIELD_MASK(device,reg,field))>>SAND_HAL_FIELD_SHIFT(device,reg,field))
#endif

#ifndef SAND_HAL_SET_FIELD
#define SAND_HAL_SET_FIELD(device,reg,field,toValue) (((toValue)<<SAND_HAL_FIELD_SHIFT(device,reg,field))&SAND_HAL_FIELD_MASK(device,reg,field))
#endif

#ifndef SAND_HAL_MOD_FIELD
#define SAND_HAL_MOD_FIELD(device,reg,field,regValue,fieldValue) (((regValue)&~(SAND_HAL_FIELD_MASK(device,reg,field)))|(SAND_HAL_SET_FIELD(device,reg,field,fieldValue)))
#endif

#ifndef SAND_HAL_RMW_FIELD
#define SAND_HAL_RMW_FIELD(addr,device,reg,field,fieldValue) do{unsigned int nRegValue; nRegValue=SAND_HAL_READ(addr,device,reg); nRegValue=SAND_HAL_MOD_FIELD(device,reg,field,nRegValue,fieldValue); SAND_HAL_WRITE(addr,device,reg,nRegValue);}while(0)
#endif

#ifndef SAND_HAL_RMW_FIELD_STRIDE
#define SAND_HAL_RMW_FIELD_STRIDE(addr,device,block,id,reg,field,fieldValue) do{unsigned int nRegValue; nRegValue=SAND_HAL_READ_STRIDE(addr,device,block,id,reg); nRegValue=SAND_HAL_MOD_FIELD(device,reg,field,nRegValue,fieldValue); SAND_HAL_WRITE_STRIDE(addr,device,block,id,reg,nRegValue);}while(0)
#endif

#ifndef SAND_HAL_RMW_FIELD_INDEX
#define SAND_HAL_RMW_FIELD_INDEX(addr,device,reg,index,field,fieldValue) do{unsigned int nRegValue; nRegValue=SAND_HAL_READ_INDEX(addr,device,reg,index); nRegValue=SAND_HAL_MOD_FIELD(device,reg,field,nRegValue,fieldValue); SAND_HAL_WRITE_INDEX(addr,device,reg,index,nRegValue);}while(0)
#endif

#ifndef SAND_HAL_RMW_FIELD_INDEX_STRIDE
#define SAND_HAL_RMW_FIELD_INDEX_STRIDE(addr,device,block,id,reg,index,field,fieldValue) do{unsigned int nRegValue; nRegValue=SAND_HAL_READ_INDEX(addr,device,block,id,reg,index); nRegValue=SAND_HAL_MOD_FIELD(device,reg,field,nRegValue,fieldValue); SAND_HAL_WRITE_INDEX_STRIDE(addr,device,block,id,reg,index,nRegValue);}while(0)
#endif

#ifndef SAND_HAL_READ_VERIFY
#define SAND_HAL_READ_VERIFY(_this, sMsg, base, chip, reg, nExpected) do{ INT nData = SAND_HAL_READ(base,chip,reg); if (nData!=(INT)(nExpected)) ZSIM_ERRORF(_this, "%s: Reg: %s: Read value=0x%x does not match expected=0x%x\n", _T sMsg, _T #reg, nData, (nExpected));} while(0)
#endif

/* For supporting both Caladan/Caladan2 run-time differentiation 
 * The glue must provide a defintion for this macro */
#define SAND_HAL_IS_FE2KXT(addr) (SBX_THIN_IS_FE2KXT(addr))

#endif
