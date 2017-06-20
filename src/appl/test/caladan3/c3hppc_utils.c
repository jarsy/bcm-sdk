/*
 * $Id: c3hppc_utils.c,v 1.22 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:    c3hppc_utils.c
 * Purpose: Caladan3 test utils
 * Requires:
 */


#include <shared/bsl.h>

#include <soc/defs.h>


#ifdef BCM_CALADAN3_SUPPORT

#include <appl/test/caladan3/c3hppc_utils.h>
#include <appl/test/caladan3/c3hppc_etu.h>
#include <appl/diag/bslcons.h>
#include <appl/diag/bslfile.h>




/*
 * Function:
 *      c3hppcUtils_poll_field
 *
 * Purpose:
 *      Polls for a field to obtain a user supplied value then w1tc
 *
 * Parameters:
 *      nUnit       - Device unit number
 *      sRegName    - Register name for error messaging
 *      nRegIndex   - Register index used for soc routines
 *      nFieldIndex - Field index used for soc routines
 *      uFieldValue - Desired field value to obtain
 *      nTimeOut    - Time out value
 *
 * Returns:
 *      None
 */
int
c3hppcUtils_poll_field( int nUnit, int nPort, int nRegIndex, int nFieldIndex, uint32 uFieldValue, int nTimeOut,
                        uint8 bClear, sal_time_t *TimeStamp ) {

  uint32 u32bRegisterValue, uCurrentFieldValue;
  uint64 uuTimeOut;
  int nUsecs; 

  COMPILER_64_SET(uuTimeOut,0,nTimeOut);  /* nTimeOut is assumed to be in "seconds" */
  if ( !SAL_BOOT_QUICKTURN ) { 
    COMPILER_64_UMUL_32(uuTimeOut, 100000);  /* Multiply by 100000 instead of 1000000 to convert to the number of micro-seconds
                             taking into accout the register access time.
                          */ 
    nUsecs = 1;
  } else {
    uint64 uuTmp = COMPILER_64_INIT(0,1);
    nUsecs = 1000000;
    if ( COMPILER_64_EQ(uuTimeOut, uuTmp) ) COMPILER_64_ADD_32(uuTimeOut,1);  /* In emulation ensure minimum of 2 times through following while() loop. */
  }
  

  while ( !COMPILER_64_IS_ZERO(uuTimeOut) ) {
    uint64 uuTmp = COMPILER_64_INIT(0,1);
    soc_reg32_get( nUnit, nRegIndex, nPort, 0, &u32bRegisterValue );
    uCurrentFieldValue = soc_reg_field_get( nUnit, nRegIndex, u32bRegisterValue, nFieldIndex );
    if ( uFieldValue == uCurrentFieldValue ) {
      *TimeStamp = sal_time();
      COMPILER_64_SET(uuTimeOut,0, 1);
      if ( bClear ) {
        u32bRegisterValue = 0;
        soc_reg_field_set( nUnit, nRegIndex, &u32bRegisterValue, nFieldIndex, uFieldValue );
        soc_reg32_set( nUnit, nRegIndex, nPort, 0, u32bRegisterValue );
      }
    } else if ( COMPILER_64_EQ(uuTimeOut, uuTmp) ) {
      *TimeStamp = sal_time();
      return -1;
    }
    sal_usleep( nUsecs );
    COMPILER_64_SUB_32(uuTimeOut,1);
  }

  return 0;
}

int
c3hppcUtils_display_register_notzero(int nUnit, int nPort, int nRegIndex) {
  uint32 uRegisterValue;
  char sPrintBuf[2048];
  char sSuffix[16];
  int rc;
  char *pRegName;

  rc = 0;
  soc_reg32_get( nUnit, nRegIndex, nPort, 0, &uRegisterValue );
  if ( uRegisterValue ) {
    sSuffix[0] = 0x00;
    nPort &= 0x7fffffff;
    pRegName = SOC_REG_NAME(nUnit,nRegIndex);
    if ( pRegName[0] == 'C' && pRegName[1] == 'O' && pRegName[2] == '_' ) {
      sal_sprintf(sSuffix, ".cop%d", nPort);
    } else if ( pRegName[0] == 'T' && pRegName[1] == 'M' && pRegName[2] == '_' && pRegName[3] == 'Q' ) {
      sal_sprintf(sSuffix, ".qe%d", nPort);
    } else if ( pRegName[0] == 'C' && pRegName[1] == 'I' && pRegName[2] == '_' ) {
      sal_sprintf(sSuffix, ".ci%d", nPort);
    } else if ( (pRegName[0] == 'P' && pRegName[1] == 'T' && pRegName[2] == '_') ||
                (pRegName[0] == 'I' && pRegName[1] == 'P' && pRegName[2] == 'T' && pRegName[3] == 'E') ) {
      sal_sprintf(sSuffix, ".pt%d", nPort);
    } else if ( pRegName[0] == 'P' && pRegName[1] == 'R' && pRegName[2] == '_' ) {
      sal_sprintf(sSuffix, ".pr%d", nPort);
    } else if ( pRegName[0] == 'I' && pRegName[1] == 'L' && pRegName[2] == '_' ) {
      sal_sprintf(sSuffix, ".intl%d", nPort);
    }  
    soc_reg_sprint_data( nUnit, sPrintBuf, ",  ", nRegIndex, uRegisterValue );
    cli_out("\nRegister[%s%s] --> %s \n", pRegName, sSuffix, sPrintBuf);
    rc = 1;
  }

  return rc;
}


uint64
c3hppcUtils_generate_64b_mask(int nHiBit, int nLoBit)
{
  uint32 uIndex;
  uint64 uuMask = COMPILER_64_INIT(0,0);
  uint64 uuTmp;

  assert(nHiBit<64);
  assert(nLoBit>=0);
  assert(nHiBit>=nLoBit);
  for (uIndex=0;uIndex<64;uIndex++){
    if (uIndex>=nLoBit && uIndex<=nHiBit){
      COMPILER_64_SET(uuTmp, 0, 1);
      COMPILER_64_SHL(uuTmp, uIndex);
      COMPILER_64_OR(uuMask, uuTmp);
    }
  }

  return uuMask;
}


uint32
c3hppcUtils_generate_32b_mask(int nHiBit, int nLoBit)
{
  uint32 uIndex;
  uint32 uMask = 0;

  assert(nHiBit<32);
  assert(nLoBit>=0);
  assert(nHiBit>=nLoBit);
  for (uIndex=0;uIndex<32;uIndex++){
    if (uIndex>=(uint32)nLoBit && uIndex<=(uint32)nHiBit){
      uMask |= (0x1<<uIndex);
    }
  }

  return uMask;
}


uint64
c3hppcUtils_64bit_flip( uint64 uuDataIn )
{
  int nIndex;
  uint64 uuDataOut;
  uint64 uuMask;
  uint64 uuTmp;

  COMPILER_64_ZERO(uuDataOut);
  COMPILER_64_SET(uuMask,0,1);
  for ( nIndex = 0; nIndex < 32; nIndex++ ) {
    COMPILER_64_SET(uuTmp, COMPILER_64_HI(uuDataIn), COMPILER_64_LO(uuDataIn));
    COMPILER_64_AND(uuTmp, uuMask);
    COMPILER_64_SHL(uuTmp, (63 - (2*nIndex)));
    COMPILER_64_OR(uuDataOut, uuTmp);
    COMPILER_64_SHL(uuMask,1);
  }
  for ( nIndex = 0; nIndex < 32; nIndex++ ) {
    COMPILER_64_SET(uuTmp, COMPILER_64_HI(uuDataIn), COMPILER_64_LO(uuDataIn));
    COMPILER_64_AND(uuTmp, uuMask);
    COMPILER_64_SHR(uuTmp, ((2*nIndex) + 1));
    COMPILER_64_OR(uuDataOut, uuTmp);
    COMPILER_64_SHL(uuMask, 1);
  }

  return uuDataOut;
}


uint64
c3hppcUtils_64b_byte_reflect( uint64 uuDataIn )
{
  uint64 uuDataOut, uuTmp, uuMask;

  COMPILER_64_ZERO(uuDataOut);

  COMPILER_64_SET(uuMask, 0x00000000,0x000000ff);
  COMPILER_64_SET(uuTmp, COMPILER_64_HI(uuDataIn), COMPILER_64_LO(uuDataIn));
  COMPILER_64_AND(uuTmp, uuMask);
  COMPILER_64_SHL(uuTmp, 56);
  COMPILER_64_OR(uuDataOut, uuTmp);

  COMPILER_64_SET(uuMask, 0x00000000,0x0000ff00);
  COMPILER_64_SET(uuTmp, COMPILER_64_HI(uuDataIn), COMPILER_64_LO(uuDataIn));
  COMPILER_64_AND(uuTmp, uuMask);
  COMPILER_64_SHL(uuTmp, 40);
  COMPILER_64_OR(uuDataOut, uuTmp);

  COMPILER_64_SET(uuMask, 0x00000000,0x00ff0000);
  COMPILER_64_SET(uuTmp, COMPILER_64_HI(uuDataIn), COMPILER_64_LO(uuDataIn));
  COMPILER_64_AND(uuTmp, uuMask);
  COMPILER_64_SHL(uuTmp, 24);
  COMPILER_64_OR(uuDataOut, uuTmp);

  COMPILER_64_SET(uuMask, 0x00000000,0xff000000);
  COMPILER_64_SET(uuTmp, COMPILER_64_HI(uuDataIn), COMPILER_64_LO(uuDataIn));
  COMPILER_64_AND(uuTmp, uuMask);
  COMPILER_64_SHL(uuTmp, 8);
  COMPILER_64_OR(uuDataOut, uuTmp);

  COMPILER_64_SET(uuMask, 0x000000ff,0x00000000);
  COMPILER_64_SET(uuTmp, COMPILER_64_HI(uuDataIn), COMPILER_64_LO(uuDataIn));
  COMPILER_64_AND(uuTmp, uuMask);
  COMPILER_64_SHR(uuTmp, 8);
  COMPILER_64_OR(uuDataOut, uuTmp);

  COMPILER_64_SET(uuMask, 0x0000ff00,0x00000000);
  COMPILER_64_SET(uuTmp, COMPILER_64_HI(uuDataIn), COMPILER_64_LO(uuDataIn));
  COMPILER_64_AND(uuTmp, uuMask);
  COMPILER_64_SHR(uuTmp, 24);
  COMPILER_64_OR(uuDataOut, uuTmp);

  COMPILER_64_SET(uuMask, 0x00ff0000,0x00000000);
  COMPILER_64_SET(uuTmp, COMPILER_64_HI(uuDataIn), COMPILER_64_LO(uuDataIn));
  COMPILER_64_AND(uuTmp, uuMask);
  COMPILER_64_SHR(uuTmp, 40);
  COMPILER_64_OR(uuDataOut, uuTmp);

  COMPILER_64_SET(uuMask, 0xff000000,0x00000000);
  COMPILER_64_SET(uuTmp, COMPILER_64_HI(uuDataIn), COMPILER_64_LO(uuDataIn));
  COMPILER_64_AND(uuTmp, uuMask);
  COMPILER_64_SHR(uuTmp, 56);
  COMPILER_64_OR(uuDataOut, uuTmp);

  return uuDataOut;
}


uint32
c3hppcUtils_ceil_power_of_2_exp( uint32 uDataIn )
{
  uint32 uDataOut;
  
  uDataOut = 0;

  if ( uDataIn <= 0x00000001 )      uDataOut = 0;
  else if ( uDataIn <= 0x00000002 ) uDataOut = 1;
  else if ( uDataIn <= 0x00000004 ) uDataOut = 2;
  else if ( uDataIn <= 0x00000008 ) uDataOut = 3;
  else if ( uDataIn <= 0x00000010 ) uDataOut = 4;
  else if ( uDataIn <= 0x00000020 ) uDataOut = 5;
  else if ( uDataIn <= 0x00000040 ) uDataOut = 6;
  else if ( uDataIn <= 0x00000080 ) uDataOut = 7;
  else if ( uDataIn <= 0x00000100 ) uDataOut = 8;
  else if ( uDataIn <= 0x00000200 ) uDataOut = 9;
  else if ( uDataIn <= 0x00000400 ) uDataOut = 10;
  else if ( uDataIn <= 0x00000800 ) uDataOut = 11;
  else if ( uDataIn <= 0x00001000 ) uDataOut = 12;
  else if ( uDataIn <= 0x00002000 ) uDataOut = 13;
  else if ( uDataIn <= 0x00004000 ) uDataOut = 14;
  else if ( uDataIn <= 0x00008000 ) uDataOut = 15;
  else if ( uDataIn <= 0x00010000 ) uDataOut = 16;
  else if ( uDataIn <= 0x00020000 ) uDataOut = 17;
  else if ( uDataIn <= 0x00040000 ) uDataOut = 18;
  else if ( uDataIn <= 0x00080000 ) uDataOut = 19;
  else if ( uDataIn <= 0x00100000 ) uDataOut = 20;
  else if ( uDataIn <= 0x00200000 ) uDataOut = 21;
  else if ( uDataIn <= 0x00400000 ) uDataOut = 22;
  else if ( uDataIn <= 0x00800000 ) uDataOut = 23;
  else if ( uDataIn <= 0x01000000 ) uDataOut = 24;
  else if ( uDataIn <= 0x02000000 ) uDataOut = 25;
  else if ( uDataIn <= 0x04000000 ) uDataOut = 26;
  else if ( uDataIn <= 0x08000000 ) uDataOut = 27;
  else if ( uDataIn <= 0x10000000 ) uDataOut = 28;
  else if ( uDataIn <= 0x20000000 ) uDataOut = 29;
  else if ( uDataIn <= 0x40000000 ) uDataOut = 30;
  else if ( uDataIn <= 0x80000000 ) uDataOut = 31;

  return uDataOut;
}

uint32
c3hppcUtils_ceil_power_of_2( uint32 uDataIn )
{
  uint32 uDataOut;

  uDataOut = 0;

  if ( uDataIn <= 0x00000001 )      uDataOut = 0x00000001;
  else if ( uDataIn <= 0x00000002 ) uDataOut = 0x00000002;
  else if ( uDataIn <= 0x00000004 ) uDataOut = 0x00000004;
  else if ( uDataIn <= 0x00000008 ) uDataOut = 0x00000008;
  else if ( uDataIn <= 0x00000010 ) uDataOut = 0x00000010;
  else if ( uDataIn <= 0x00000020 ) uDataOut = 0x00000020;
  else if ( uDataIn <= 0x00000040 ) uDataOut = 0x00000040;
  else if ( uDataIn <= 0x00000080 ) uDataOut = 0x00000080;
  else if ( uDataIn <= 0x00000100 ) uDataOut = 0x00000100;
  else if ( uDataIn <= 0x00000200 ) uDataOut = 0x00000200;
  else if ( uDataIn <= 0x00000400 ) uDataOut = 0x00000400;
  else if ( uDataIn <= 0x00000800 ) uDataOut = 0x00000800;
  else if ( uDataIn <= 0x00001000 ) uDataOut = 0x00001000;
  else if ( uDataIn <= 0x00002000 ) uDataOut = 0x00002000;
  else if ( uDataIn <= 0x00004000 ) uDataOut = 0x00004000;
  else if ( uDataIn <= 0x00008000 ) uDataOut = 0x00008000;
  else if ( uDataIn <= 0x00010000 ) uDataOut = 0x00010000;
  else if ( uDataIn <= 0x00020000 ) uDataOut = 0x00020000;
  else if ( uDataIn <= 0x00040000 ) uDataOut = 0x00040000;
  else if ( uDataIn <= 0x00080000 ) uDataOut = 0x00080000;
  else if ( uDataIn <= 0x00100000 ) uDataOut = 0x00100000;
  else if ( uDataIn <= 0x00200000 ) uDataOut = 0x00200000;
  else if ( uDataIn <= 0x00400000 ) uDataOut = 0x00400000;
  else if ( uDataIn <= 0x00800000 ) uDataOut = 0x00800000;
  else if ( uDataIn <= 0x01000000 ) uDataOut = 0x01000000;
  else if ( uDataIn <= 0x02000000 ) uDataOut = 0x02000000;
  else if ( uDataIn <= 0x04000000 ) uDataOut = 0x04000000;
  else if ( uDataIn <= 0x08000000 ) uDataOut = 0x08000000;
  else if ( uDataIn <= 0x10000000 ) uDataOut = 0x10000000;
  else if ( uDataIn <= 0x20000000 ) uDataOut = 0x20000000;
  else if ( uDataIn <= 0x40000000 ) uDataOut = 0x40000000;
  else if ( uDataIn <= 0x80000000 ) uDataOut = 0x80000000;

  return uDataOut;
}

uint32
c3hppcUtils_floor_power_of_2_exp( uint32 uDataIn )
{
  uint32 uDataOut;

  uDataOut = 0;

  if ( uDataIn < 0x00000001 )      uDataOut = 0;
  else if ( uDataIn < 0x00000002 ) uDataOut = 1;
  else if ( uDataIn < 0x00000004 ) uDataOut = 2;
  else if ( uDataIn < 0x00000008 ) uDataOut = 3;
  else if ( uDataIn < 0x00000010 ) uDataOut = 4;
  else if ( uDataIn < 0x00000020 ) uDataOut = 5;
  else if ( uDataIn < 0x00000040 ) uDataOut = 6;
  else if ( uDataIn < 0x00000080 ) uDataOut = 7;
  else if ( uDataIn < 0x00000100 ) uDataOut = 8;
  else if ( uDataIn < 0x00000200 ) uDataOut = 9;
  else if ( uDataIn < 0x00000400 ) uDataOut = 10;
  else if ( uDataIn < 0x00000800 ) uDataOut = 11;
  else if ( uDataIn < 0x00001000 ) uDataOut = 12;
  else if ( uDataIn < 0x00002000 ) uDataOut = 13;
  else if ( uDataIn < 0x00004000 ) uDataOut = 14;
  else if ( uDataIn < 0x00008000 ) uDataOut = 15;
  else if ( uDataIn < 0x00010000 ) uDataOut = 16;
  else if ( uDataIn < 0x00020000 ) uDataOut = 17;
  else if ( uDataIn < 0x00040000 ) uDataOut = 18;
  else if ( uDataIn < 0x00080000 ) uDataOut = 19;
  else if ( uDataIn < 0x00100000 ) uDataOut = 20;
  else if ( uDataIn < 0x00200000 ) uDataOut = 21;
  else if ( uDataIn < 0x00400000 ) uDataOut = 22;
  else if ( uDataIn < 0x00800000 ) uDataOut = 23;
  else if ( uDataIn < 0x01000000 ) uDataOut = 24;
  else if ( uDataIn < 0x02000000 ) uDataOut = 25;
  else if ( uDataIn < 0x04000000 ) uDataOut = 26;
  else if ( uDataIn < 0x08000000 ) uDataOut = 27;
  else if ( uDataIn < 0x10000000 ) uDataOut = 28;
  else if ( uDataIn < 0x20000000 ) uDataOut = 29;
  else if ( uDataIn < 0x40000000 ) uDataOut = 30;
  else if ( uDataIn < 0x80000000 ) uDataOut = 31;

  if ( uDataOut ) --uDataOut;

  return uDataOut;
}

int
c3hppcUtils_floor_power_of_2_exp_real( double dDataIn )
{
  int nDataOut;

  nDataOut = 0;

  if ( dDataIn < 1 ) {
    if ( dDataIn > 0.5 ) nDataOut = 0; 
    else if ( dDataIn > 0.25 ) nDataOut = -1;
    else if ( dDataIn > 0.125 ) nDataOut = -2;
    else if ( dDataIn > 0.0625 ) nDataOut = -3;
    else if ( dDataIn > 0.03125 ) nDataOut = -4;
    else if ( dDataIn > 0.015625 ) nDataOut = -5;
    else if ( dDataIn > 0.0078125 ) nDataOut = -6;
    else if ( dDataIn > 0.00390625 ) nDataOut = -7;
    else if ( dDataIn > 0.001953125 ) nDataOut = -8;
    else if ( dDataIn > 0.0009765625 ) nDataOut = -9;
    else if ( dDataIn > 0.00048828125 ) nDataOut = -10;
    else if ( dDataIn > 0.000244140625 ) nDataOut = -11;
    else if ( dDataIn > 0.0001220703125 ) nDataOut = -12;
    else if ( dDataIn > 0.00006103515625 ) nDataOut = -13;
    else if ( dDataIn > 0.000030517578125 ) nDataOut = -14;
    else if ( dDataIn > 0.0000152587890625 ) nDataOut = -15;
    else if ( dDataIn > 0.00000762939453125 ) nDataOut = -16;
    else nDataOut = -17;
  } else if ( dDataIn < 0x00000002 ) nDataOut = 1;
  else if ( dDataIn < 0x00000004 ) nDataOut = 2;
  else if ( dDataIn < 0x00000008 ) nDataOut = 3;
  else if ( dDataIn < 0x00000010 ) nDataOut = 4;
  else if ( dDataIn < 0x00000020 ) nDataOut = 5;
  else if ( dDataIn < 0x00000040 ) nDataOut = 6;
  else if ( dDataIn < 0x00000080 ) nDataOut = 7;
  else if ( dDataIn < 0x00000100 ) nDataOut = 8;
  else if ( dDataIn < 0x00000200 ) nDataOut = 9;
  else if ( dDataIn < 0x00000400 ) nDataOut = 10;
  else if ( dDataIn < 0x00000800 ) nDataOut = 11;
  else if ( dDataIn < 0x00001000 ) nDataOut = 12;
  else if ( dDataIn < 0x00002000 ) nDataOut = 13;
  else if ( dDataIn < 0x00004000 ) nDataOut = 14;
  else if ( dDataIn < 0x00008000 ) nDataOut = 15;
  else if ( dDataIn < 0x00010000 ) nDataOut = 16;
  else if ( dDataIn < 0x00020000 ) nDataOut = 17;
  else if ( dDataIn < 0x00040000 ) nDataOut = 18;
  else if ( dDataIn < 0x00080000 ) nDataOut = 19;
  else if ( dDataIn < 0x00100000 ) nDataOut = 20;
  else if ( dDataIn < 0x00200000 ) nDataOut = 21;
  else if ( dDataIn < 0x00400000 ) nDataOut = 22;
  else if ( dDataIn < 0x00800000 ) nDataOut = 23;
  else if ( dDataIn < 0x01000000 ) nDataOut = 24;
  else if ( dDataIn < 0x02000000 ) nDataOut = 25;
  else if ( dDataIn < 0x04000000 ) nDataOut = 26;
  else if ( dDataIn < 0x08000000 ) nDataOut = 27;
  else if ( dDataIn < 0x10000000 ) nDataOut = 28;
  else if ( dDataIn < 0x20000000 ) nDataOut = 29;
  else if ( dDataIn < 0x40000000 ) nDataOut = 30;
  else if ( dDataIn < 0x80000000 ) nDataOut = 31;

  --nDataOut;

  return nDataOut;
}

double
c3hppcUtils_negative_exp_to_decimal_value( int nDataIn )
{
  double dDataOut;

  dDataOut = 0;

  if ( nDataIn == -1 )       dDataOut = 0.5; 
  else if ( nDataIn == -2 )  dDataOut = 0.25;
  else if ( nDataIn == -3 )  dDataOut = 0.125;
  else if ( nDataIn == -4 )  dDataOut = 0.0625;
  else if ( nDataIn == -5 )  dDataOut = 0.03125;
  else if ( nDataIn == -6 )  dDataOut = 0.015625;
  else if ( nDataIn == -7 )  dDataOut = 0.0078125;
  else if ( nDataIn == -8 )  dDataOut = 0.00390625;
  else if ( nDataIn == -9 )  dDataOut = 0.001953125;
  else if ( nDataIn == -10 ) dDataOut = 0.0009765625;
  else if ( nDataIn == -11 ) dDataOut = 0.00048828125;
  else if ( nDataIn == -12 ) dDataOut = 0.000244140625;
  else if ( nDataIn == -13 ) dDataOut = 0.0001220703125;
  else if ( nDataIn == -14 ) dDataOut = 0.00006103515625;
  else if ( nDataIn == -15 ) dDataOut = 0.000030517578125;
  else if ( nDataIn == -16 ) dDataOut = 0.0000152587890625;
  else if ( nDataIn == -17 ) dDataOut = 0.00000762939453125;

  return dDataOut;
}


int
c3hppcUtils_first_bit_set( uint32 uDataIn )
{
  int nIndex;
  uint32 uMask;

  uMask = 1;
  for ( nIndex = 0; nIndex < 32; nIndex++ ) {
    if ( uMask & uDataIn ) {
      break;
    } else { 
      uMask <<= 1;
    }
  }

  return nIndex;
}


int c3hppcUtils_enable_output_to_file( char *pFileName ) {
  int append = FALSE;
  

  if ( bslfile_open( pFileName, append ) < 0 ) {
    return 1;
  }

  bslcons_enable(FALSE);

  return 0;
}


int c3hppcUtils_disable_output_to_file(void) {

  bslfile_close();

  bslcons_enable(TRUE);

  return 0;
}


int c3hppcUtils_wc_aer_write( int nUnit, int nPhyID, uint16 uLane, uint16 uAddr, uint16 uData ) {

  uint16 uPage, uOffset;

  uPage = uAddr & 0xfff0;
  uOffset = 0x0010 | (uAddr & 0x000f);

  SOC_IF_ERROR_RETURN( soc_miim_write( nUnit, nPhyID, 0x1f, 0xffd0 ) );
  SOC_IF_ERROR_RETURN( soc_miim_write( nUnit, nPhyID, 0x1e, uLane ) );
  SOC_IF_ERROR_RETURN( soc_miim_write( nUnit, nPhyID, 0x1f, uPage ) );
  SOC_IF_ERROR_RETURN( soc_miim_write( nUnit, nPhyID, uOffset, uData ) );
  SOC_IF_ERROR_RETURN( soc_miim_write( nUnit, nPhyID, 0x1f, 0xffd0 ) );
  SOC_IF_ERROR_RETURN( soc_miim_write( nUnit, nPhyID, 0x1e, 0x0000 ) );

  return 0;
}


uint16 c3hppcUtils_wc_aer_read( int nUnit, int nPhyID, uint16 uLane, uint16 uAddr ) {

  uint16 uPage, uOffset, uRegValue;

  uPage = uAddr & 0xfff0;
  uOffset = 0x0010 | (uAddr & 0x000f);

  SOC_IF_ERROR_RETURN( soc_miim_write( nUnit, nPhyID, 0x1f, 0xffd0 ) );
  SOC_IF_ERROR_RETURN( soc_miim_write( nUnit, nPhyID, 0x1e, uLane ) );
  SOC_IF_ERROR_RETURN( soc_miim_write( nUnit, nPhyID, 0x1f, uPage ) );
  SOC_IF_ERROR_RETURN( soc_miim_read(  nUnit, nPhyID, uOffset, &uRegValue ) );
  SOC_IF_ERROR_RETURN( soc_miim_write( nUnit, nPhyID, 0x1f, 0xffd0 ) );
  SOC_IF_ERROR_RETURN( soc_miim_write( nUnit, nPhyID, 0x1e, 0x0000 ) );

  return uRegValue;
}


#endif  /* #ifdef BCM_CALADAN3_SUPPORT */



