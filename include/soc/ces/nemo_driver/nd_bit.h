/*
 * $Id$
 *
 * Copyright: (c) 2011 BATM 
 */
#ifndef __NEMO_BIT_H__
#define __NEMO_BIT_H__

#include "nd_platform.h"
#include "pub/nd_hw.h"
#include "nd_registers.h"
#include "pub/nd_api.h"
         
#ifdef __cplusplus
extern "C"
{
#endif

AgResult ag_nd_opcode_read_bit(AgNdDevice*, AgNdMsgBit*);

#ifdef  BCM_CES_SDK
void ag_nd_bit_print(AgNdDevice*, AG_U32 n_verbose, const char *format, ...);
#else
#pragma check_printf_formats
void ag_nd_bit_print(AgNdDevice*, AG_U32 n_verbose, const char *format, ...);
#pragma no_check_printf_formats
#endif

void ag_nd_bit_one_reg(AgNdDevice*, AgNdRegProperties*);
void ag_nd_bit_all_regs(AgNdDevice *);
void ag_nd_bit_rw_reg16_one_pass(AgNdDevice *, AgNdRegProperties *, AG_U16, AG_U32);
void ag_nd_bit_mem(AgNdDevice *, AgNdMemUnit *);
void ag_nd_bit_rw_word16_report(AgNdDevice *, AG_U16, AG_U16, AG_U32, AG_CHAR *, AG_CHAR *, AG_BOOL);
void ag_nd_bit_rw_word32_report(AgNdDevice *, AG_U32, AG_U32, AG_U32, AG_CHAR *, AG_CHAR *, AG_BOOL);

void ag_nd_bit_rw_reg16(AgNdDevice*, AgNdRegProperties*, AG_U32, AG_BOOL b_test);
void ag_nd_bit_ro_reg16(AgNdDevice*, AgNdRegProperties*, AG_U32, AG_BOOL b_test);
void ag_nd_bit_rw1clr_reg16(AgNdDevice*, AgNdRegProperties*, AG_U32, AG_BOOL b_test);
void ag_nd_bit_rclr_reg16(AgNdDevice*, AgNdRegProperties*, AG_U32, AG_BOOL b_test);

#ifdef __cplusplus
}
#endif


#endif /* __NEMO_BIT_H__ */

