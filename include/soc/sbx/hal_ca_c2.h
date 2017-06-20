/*
 *$Id$
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */

#ifndef SAND_HAL_CA_C2_H
#define SAND_HAL_CA_C2_H

#include "hal_common.h"
#include "hal_ca_auto.h"
#include "hal_c2_auto.h"



#ifndef SAND_HAL_FE2000
#define SAND_HAL_FE2000(unit, reg) \
    (SAND_HAL_IS_FE2KXT((int)unit) \
    ? (SAND_HAL_C2_##reg ) \
    : (SAND_HAL_CA_##reg ) )
#endif

#ifndef SAND_HAL_FE2000_WRITE_OFFS
#define SAND_HAL_FE2000_WRITE_OFFS(addr,offs,value) SAND_HAL_WRITE_OFFS(addr,offs,value)
#endif
#ifndef SAND_HAL_FE2000_READ_OFFS
#define SAND_HAL_FE2000_READ_OFFS(addr, offs) SAND_HAL_READ_OFFS(addr,offs)
#endif

#ifndef SAND_HAL_FE2000_WRITE_OFFS_RAW
#define SAND_HAL_FE2000_WRITE_OFFS_RAW(addr,offs,value) SAND_HAL_WRITE_OFFS_RAW(addr,offs,value)
#endif
#ifndef SAND_HAL_FE2000_READ_OFFS_RAW
#define SAND_HAL_FE2000_READ_OFFS_RAW(addr, offs) SAND_HAL_READ_OFFS_RAW(addr,offs)
#endif

#ifndef SAND_HAL_FE2000_REG_OFFSET
#define SAND_HAL_FE2000_REG_OFFSET(unit,reg) \
    (SAND_HAL_IS_FE2KXT((int)unit) \
    ? (SAND_HAL_##C2##_##reg##_OFFSET) \
    : (SAND_HAL_##CA##_##reg##_OFFSET) )
#endif

#ifndef SAND_HAL_FE2000_REG_OFFSET_STRIDE
#define SAND_HAL_FE2000_REG_OFFSET_STRIDE(unit,block,id,reg) \
    (SAND_HAL_IS_FE2KXT((int)unit) \
    ? (id*SAND_HAL_##C2##_##block##_INSTANCE_ADDR_STRIDE+SAND_HAL_REG_OFFSET(C2,reg)) \
    : (id*SAND_HAL_##CA##_##block##_INSTANCE_ADDR_STRIDE+SAND_HAL_REG_OFFSET(CA,reg)) )
#endif

#ifndef SAND_HAL_FE2000_FIELD_MASK
#define SAND_HAL_FE2000_FIELD_MASK(unit,reg,field) \
    (SAND_HAL_IS_FE2KXT((int)unit) \
    ? (SAND_HAL_##C2##_##reg##_##field##_MASK) \
    : (SAND_HAL_##CA##_##reg##_##field##_MASK) )
#endif
#ifndef SAND_HAL_FE2000_REG_MASK
#define SAND_HAL_FE2000_REG_MASK(unit, reg) \
    (SAND_HAL_IS_FE2KXT((int)unit) \
    ? (SAND_HAL_##C2##_##reg##_MASK) \
    : (SAND_HAL_##CA##_##reg##_MASK) )
#endif


#ifndef SAND_HAL_FE2000_READ
#define SAND_HAL_FE2000_READ(unit,reg) \
    (SAND_HAL_IS_FE2KXT((int)unit) \
    ? SAND_HAL_READ_OFFS((unit), SAND_HAL_REG_OFFSET(C2,reg)) \
    : SAND_HAL_READ_OFFS((unit), SAND_HAL_REG_OFFSET(CA,reg)) )
#endif

#ifndef SAND_HAL_FE2000_WRITE
#define SAND_HAL_FE2000_WRITE(unit,reg,value) \
    (SAND_HAL_IS_FE2KXT((int)unit) \
     ? SAND_HAL_WRITE_OFFS((unit),SAND_HAL_REG_OFFSET(C2,reg),(value)) \
     : SAND_HAL_WRITE_OFFS((unit),SAND_HAL_REG_OFFSET(CA,reg),(value)) )
#endif
#ifndef SAND_HAL_FE2000_READ_STRIDE
#define SAND_HAL_FE2000_READ_STRIDE(unit,block,id,reg) \
    (SAND_HAL_IS_FE2KXT((int)unit) \
    ? SAND_HAL_READ_OFFS((unit),(id*SAND_HAL_##C2##_##block##_INSTANCE_ADDR_STRIDE+SAND_HAL_REG_OFFSET(C2,reg))) \
    : SAND_HAL_READ_OFFS((unit),(id*SAND_HAL_##CA##_##block##_INSTANCE_ADDR_STRIDE+SAND_HAL_REG_OFFSET(CA,reg))) )
#endif

#ifndef SAND_HAL_FE2000_WRITE_STRIDE
#define SAND_HAL_FE2000_WRITE_STRIDE(unit,block,id,reg,value) \
    (SAND_HAL_IS_FE2KXT((int)unit) \
    ? SAND_HAL_WRITE_OFFS((unit),(id*SAND_HAL_##C2##_##block##_INSTANCE_ADDR_STRIDE+SAND_HAL_REG_OFFSET(C2,reg)),(value)) \
    : SAND_HAL_WRITE_OFFS((unit),(id*SAND_HAL_##CA##_##block##_INSTANCE_ADDR_STRIDE+SAND_HAL_REG_OFFSET(CA,reg)),(value)) )
#endif

#ifndef SAND_HAL_FE2000_WRITE_INDEX_STRIDE
#define SAND_HAL_FE2000_WRITE_INDEX_STRIDE(unit,block,id,reg,index,value) \
    (SAND_HAL_IS_FE2KXT((int)unit)) \
    ? SAND_HAL_WRITE_OFFS((unit),SAND_HAL_REG_OFFSET_STRIDE(C2,block,id,reg)+(index)*4,(value)) \
    : SAND_HAL_WRITE_OFFS((unit),SAND_HAL_REG_OFFSET_STRIDE(CA,block,id,reg)+(index)*4,(value))
#endif

#ifndef SAND_HAL_FE2000_GET_FIELD
#define SAND_HAL_FE2000_GET_FIELD(unit,reg,field,fromValue)\
    (SAND_HAL_IS_FE2KXT((int)unit) \
    ? (((fromValue)&SAND_HAL_FIELD_MASK(C2,reg,field))>>SAND_HAL_FIELD_SHIFT(C2,reg,field)) \
    : (((fromValue)&SAND_HAL_FIELD_MASK(CA,reg,field))>>SAND_HAL_FIELD_SHIFT(CA,reg,field)) )
#endif

#ifndef SAND_HAL_FE2000_FIELD_SHIFT
#define SAND_HAL_FE2000_FIELD_SHIFT(unit,reg,field) \
    (SAND_HAL_IS_FE2KXT((int)unit) \
    ? (SAND_HAL_##C2##_##reg##_##field##_SHIFT) \
    : (SAND_HAL_##CA##_##reg##_##field##_SHIFT) )
#endif

#ifndef SAND_HAL_FE2000_MOD_FIELD
#define SAND_HAL_FE2000_MOD_FIELD(unit,reg,field,regValue,fieldValue) \
    (SAND_HAL_IS_FE2KXT((int)unit) \
    ? (((regValue)&~(SAND_HAL_FIELD_MASK(C2,reg,field)))|(SAND_HAL_SET_FIELD(C2,reg,field,fieldValue))) \
    : (((regValue)&~(SAND_HAL_FIELD_MASK(CA,reg,field)))|(SAND_HAL_SET_FIELD(CA,reg,field,fieldValue))))
#endif

#ifndef SAND_HAL_FE2000_SET_FIELD
#define SAND_HAL_FE2000_SET_FIELD(unit,reg,field,toValue) \
    (SAND_HAL_IS_FE2KXT((int)unit) \
    ? (((toValue)<<SAND_HAL_FIELD_SHIFT(C2,reg,field))&SAND_HAL_FIELD_MASK(C2,reg,field)) \
    : (((toValue)<<SAND_HAL_FIELD_SHIFT(CA,reg,field))&SAND_HAL_FIELD_MASK(CA,reg,field)) )
#endif

#ifndef SAND_HAL_FE2000_RMW_FIELD_STRIDE
#define SAND_HAL_FE2000_RMW_FIELD_STRIDE(unit,block,id,reg,field,fieldValue) \
    if (SAND_HAL_IS_FE2KXT((int)unit)) {\
    do{unsigned int nRegValue; nRegValue=SAND_HAL_READ_STRIDE(unit,C2,block,id,reg); nRegValue=SAND_HAL_MOD_FIELD(C2,reg,field,nRegValue,fieldValue); SAND_HAL_WRITE_STRIDE(unit,C2,block,id,reg,nRegValue);}while(0); \
    }else{ \
    do{unsigned int nRegValue; nRegValue=SAND_HAL_READ_STRIDE(unit,CA,block,id,reg); nRegValue=SAND_HAL_MOD_FIELD(CA,reg,field,nRegValue,fieldValue); SAND_HAL_WRITE_STRIDE(unit,CA,block,id,reg,nRegValue);}while(0); \
    }
#endif
#ifndef SAND_HAL_FE2000_RMW_FIELD
#define SAND_HAL_FE2000_RMW_FIELD(unit,reg,field,fieldValue) \
    if (SAND_HAL_IS_FE2KXT((int)unit)) { do{unsigned int nRegValue; nRegValue=SAND_HAL_READ(unit,C2,reg); nRegValue=SAND_HAL_MOD_FIELD(C2,reg,field,nRegValue,fieldValue); SAND_HAL_WRITE(unit,C2,reg,nRegValue);}while(0); \
    }else{ do{unsigned int nRegValue; nRegValue=SAND_HAL_READ(unit,CA,reg); nRegValue=SAND_HAL_MOD_FIELD(CA,reg,field,nRegValue,fieldValue); SAND_HAL_WRITE(unit,CA,reg,nRegValue);}while(0); }
#endif

#ifndef SAND_HAL_FE2000_RMW_FIELD_INDEX_STRIDE
#define SAND_HAL_FE2000_RMW_FIELD_INDEX_STRIDE(unit,block,id,reg,index,field,fieldValue) \
    if (SAND_HAL_IS_FE2KXT((int)unit)) {\
    do{unsigned int nRegValue; nRegValue=SAND_HAL_READ_INDEX(unit,C2,block,id,reg,index); nRegValue=SAND_HAL_MOD_FIELD(C2,reg,field,nRegValue,fieldValue); SAND_HAL_WRITE_INDEX_STRIDE(unit,C2,block,id,reg,index,nRegValue);}while(0); \
    }else{ \
    do{unsigned int nRegValue; nRegValue=SAND_HAL_READ_INDEX(unit,CA,block,id,reg,index); nRegValue=SAND_HAL_MOD_FIELD(CA,reg,field,nRegValue,fieldValue); SAND_HAL_WRITE_INDEX_STRIDE(unit,CA,block,id,reg,index,nRegValue);}while(0); \
    }
#endif

#ifndef SAND_HAL_FE2000_FIELD_MSB
#define SAND_HAL_FE2000_FIELD_MSB(unit,reg,field) \
    (SAND_HAL_IS_FE2KXT((int)unit) \
    ? (SAND_HAL_##C2##_##reg##_##field##_MSB) \
    : (SAND_HAL_##CA##_##reg##_##field##_MSB))
#endif

#ifndef SAND_HAL_FE2000_FIELD_LSB
#define SAND_HAL_FE2000_FIELD_LSB(unit,reg,field) \
    (SAND_HAL_IS_FE2KXT((int)unit) \
    ? (SAND_HAL_##C2##_##reg##_##field##_LSB) \
    : (SAND_HAL_##CA##_##reg##_##field##_LSB))
#endif

#endif
