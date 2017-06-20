/*
 *
 * ==================================================
 * ==  sbQe2000ElibMem.c - elib private memory access API  ==
 * ==================================================
 *
 * WORKING REVISION: $Id: sbQe2000ElibMem.c,v 1.9 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * MODULE NAME:
 *
 *     sbQe2000ElibMem.c
 *
 * ABSTRACT:
 *
 *     elib private memory access API
 *
 * LANGUAGE:
 *
 *     C
 *
 * AUTHORS:
 *
 *     Travis B. Sawyer
 *
 * CREATION DATE:
 *
 *     21-December-2004
 *
 */
#include "sbWrappers.h"
#include "sbTypes.h"
#include "sbQe2000Elib.h"
#include "sbQe2000ElibMem.h"
#include "sbQe2000ElibContext.h"
#include "soc/drv.h"

#if !defined(SAND_BIG_ENDIAN_HOST)
#define BSWAP32(__X) (__X)
#else
#define BSWAP32(__X) SAND_SWAP_32(__X)
#endif

sbElibStatus_et sbQe2000ElibClMemWrite( sbhandle HalCtxt,
                    uint32 ulOffset,
                    uint32 ulData0,
                    uint32 ulData1 )
{
    int nTimeout;
    int nAck;
    uint32 ulCtlReg;

    SB_ASSERT( ulOffset < SB_QE2000_ELIB_CL_MEM_MAX_OFFSET );

    /*
     * Clear out any previous acks in the mem ctrl register
     */
    ulCtlReg = (SAND_HAL_SET_FIELD( KA, EP_AM_CL_MEM_ACC_CTRL, ACK, 1) |
                SAND_HAL_SET_FIELD( KA, EP_AM_CL_MEM_ACC_CTRL, REQ, 0));
    SAND_HAL_WRITE( HalCtxt, KA, EP_AM_CL_MEM_ACC_CTRL, ulCtlReg );

    /*
     * Build up our command
     */
    ulCtlReg = SAND_HAL_SET_FIELD( KA, EP_AM_CL_MEM_ACC_CTRL, REQ, 1 ) |
        SAND_HAL_SET_FIELD( KA, EP_AM_CL_MEM_ACC_CTRL, RD_WR_N, 0 ) |
        SAND_HAL_SET_FIELD( KA, EP_AM_CL_MEM_ACC_CTRL, ADDR, ulOffset );

    /*
     * Write out our data
     */
    SAND_HAL_WRITE( HalCtxt, KA, EP_AM_CL_MEM_ACC_DATA0, BSWAP32(ulData0) );
    SAND_HAL_WRITE( HalCtxt, KA, EP_AM_CL_MEM_ACC_DATA1, BSWAP32(ulData1) );

    /*
     * Write out our command and wait for the acknowledgement
     */
    SAND_HAL_WRITE( HalCtxt, KA, EP_AM_CL_MEM_ACC_CTRL, ulCtlReg );

    nTimeout = SB_QE2000_ELIB_IND_MEM_TIMEOUT;
    nAck = 0;

    while ( nTimeout-- )
    {
        ulCtlReg = SAND_HAL_READ( HalCtxt, KA, EP_AM_CL_MEM_ACC_CTRL );
        if (1 == SAND_HAL_GET_FIELD( KA, EP_AM_CL_MEM_ACC_CTRL, ACK, ulCtlReg ) )
        {
            /*
             * Clear the ack
             */
            nAck = 1;
            ulCtlReg = SAND_HAL_MOD_FIELD( KA, EP_AM_CL_MEM_ACC_CTRL, ACK, ulCtlReg, 1 );
            ulCtlReg = SAND_HAL_MOD_FIELD( KA, EP_AM_CL_MEM_ACC_CTRL, REQ, ulCtlReg, 0 );
            SAND_HAL_WRITE( HalCtxt, KA, EP_AM_CL_MEM_ACC_CTRL, ulCtlReg );
            break;
        }

        thin_delay( 100 );
    }

    if ( 0 == nAck )
    {
        return( SB_ELIB_IND_MEM_TIMEOUT );
    }

    return( SB_ELIB_OK );

}

sbElibStatus_et sbQe2000ElibClMemRead( sbhandle HalCtxt,
                           uint32 ulOffset,
                           uint32 ulClrOnRd,
                           uint32 *pulData0,
                           uint32 *pulData1 )
{
    int nTimeout;
    int nAck;
    uint32 ulCtlReg;

    SB_ASSERT( ulOffset < SB_QE2000_ELIB_CL_MEM_MAX_OFFSET );
    SB_ASSERT( pulData0 );
    SB_ASSERT( pulData1 );

    /*
     * Clear out any previous acks in the mem ctrl register & any stale data
     */
    ulCtlReg = (SAND_HAL_SET_FIELD( KA, EP_AM_CL_MEM_ACC_CTRL, ACK, 1) |
                SAND_HAL_SET_FIELD( KA, EP_AM_CL_MEM_ACC_CTRL, REQ, 0));
    SAND_HAL_WRITE( HalCtxt, KA, EP_AM_CL_MEM_ACC_CTRL, ulCtlReg );
    SAND_HAL_WRITE( HalCtxt, KA, EP_AM_CL_MEM_ACC_DATA0, 0 );
    SAND_HAL_WRITE( HalCtxt, KA, EP_AM_CL_MEM_ACC_DATA1, 0 );

    /*
     * Build up our command
     */
    ulCtlReg = SAND_HAL_SET_FIELD( KA, EP_AM_CL_MEM_ACC_CTRL, REQ, 1 ) |
        SAND_HAL_SET_FIELD( KA, EP_AM_CL_MEM_ACC_CTRL, RD_WR_N, 1 ) |
        SAND_HAL_SET_FIELD( KA, EP_AM_CL_MEM_ACC_CTRL, CLR_ON_RD, ulClrOnRd ) |
        SAND_HAL_SET_FIELD( KA, EP_AM_CL_MEM_ACC_CTRL, ADDR, ulOffset );

    /*
     * Write out our command and wait for the acknowledgement
     */
    SAND_HAL_WRITE( HalCtxt, KA, EP_AM_CL_MEM_ACC_CTRL, ulCtlReg );

    nTimeout = SB_QE2000_ELIB_IND_MEM_TIMEOUT;
    nAck = 0;

    while ( nTimeout-- )
    {
        ulCtlReg = SAND_HAL_READ( HalCtxt, KA, EP_AM_CL_MEM_ACC_CTRL );
        if (1 == SAND_HAL_GET_FIELD( KA, EP_AM_CL_MEM_ACC_CTRL, ACK, ulCtlReg ) )
        {
            /*
             * Grab the data & clear the ack
             */
            nAck = 1;
            *pulData0 = BSWAP32(SAND_HAL_READ( HalCtxt, KA, EP_AM_CL_MEM_ACC_DATA0 ));
            *pulData1 = BSWAP32(SAND_HAL_READ( HalCtxt, KA, EP_AM_CL_MEM_ACC_DATA1 ));
            ulCtlReg = SAND_HAL_MOD_FIELD( KA, EP_AM_CL_MEM_ACC_CTRL, ACK, ulCtlReg, 1 );
            ulCtlReg = SAND_HAL_MOD_FIELD( KA, EP_AM_CL_MEM_ACC_CTRL, REQ, ulCtlReg, 0 );
            SAND_HAL_WRITE( HalCtxt, KA, EP_AM_CL_MEM_ACC_CTRL, ulCtlReg );
            break;
        }

        thin_delay( 100 );
    }

    if ( 0 == nAck )
    {
        return( SB_ELIB_IND_MEM_TIMEOUT );
    }

    return( SB_ELIB_OK );

}


sbElibStatus_et sbQe2000ElibPriMemWrite( sbhandle HalCtxt,
                     uint32 ulOffset,
                     uint32 ulData )
{
    int nTimeout;
    int nAck;
    uint32 ulCtlReg;

    SB_ASSERT( ulOffset < SB_QE2000_ELIB_PRI_MEM_MAX_OFFSET );

    /*
     * Clear out any previous acks in the mem ctrl register
     */
    ulCtlReg = (SAND_HAL_SET_FIELD( KA, EP_BM_BF_MEM_ACC_CTRL, ACK, 1 ) |
                SAND_HAL_SET_FIELD( KA, EP_BM_BF_MEM_ACC_CTRL, REQ, 0 ));
    SAND_HAL_WRITE( HalCtxt, KA, EP_BM_BF_MEM_ACC_CTRL, ulCtlReg );

    /*
     * Build up our command
     */
    ulCtlReg = SAND_HAL_SET_FIELD( KA, EP_BM_BF_MEM_ACC_CTRL, REQ, 1 ) |
        SAND_HAL_SET_FIELD( KA, EP_BM_BF_MEM_ACC_CTRL, RD_WR_N, 0 ) |
        SAND_HAL_SET_FIELD( KA, EP_BM_BF_MEM_ACC_CTRL, ADDR, ulOffset );

    /*
     * Write out our data
     */
    SAND_HAL_WRITE( HalCtxt, KA, EP_BM_BF_MEM_ACC_DATA, BSWAP32(ulData) );

    /*
     * Write out our command and wait for the acknowledgement
     */
    SAND_HAL_WRITE( HalCtxt, KA, EP_BM_BF_MEM_ACC_CTRL, ulCtlReg );

    nTimeout = SB_QE2000_ELIB_IND_MEM_TIMEOUT;
    nAck = 0;

    while ( nTimeout-- )
    {
        ulCtlReg = SAND_HAL_READ( HalCtxt, KA, EP_BM_BF_MEM_ACC_CTRL );
        if (1 == SAND_HAL_GET_FIELD( KA, EP_BM_BF_MEM_ACC_CTRL, ACK, ulCtlReg ) )
        {
            /*
             * Clear the ack
             */
            nAck = 1;
            ulCtlReg = SAND_HAL_MOD_FIELD( KA, EP_BM_BF_MEM_ACC_CTRL, ACK, ulCtlReg, 1 );
            ulCtlReg = SAND_HAL_MOD_FIELD( KA, EP_BM_BF_MEM_ACC_CTRL, REQ, ulCtlReg, 0 );
            SAND_HAL_WRITE( HalCtxt, KA, EP_BM_BF_MEM_ACC_CTRL, ulCtlReg );
            break;
        }

        thin_delay( 100 );
    }

    if ( 0 == nAck )
    {
        return( SB_ELIB_IND_MEM_TIMEOUT );
    }

    return( SB_ELIB_OK );

}

sbElibStatus_et sbQe2000ElibPriMemRead( sbhandle HalCtxt,
                    uint32 ulOffset,
                    uint32 *pulData )
{
    int nTimeout;
    int nAck;
    uint32 ulCtlReg;

    SB_ASSERT( ulOffset < SB_QE2000_ELIB_PRI_MEM_MAX_OFFSET );
    SB_ASSERT( pulData );

    /*
     * Clear out any previous acks in the mem ctrl register & any stale data
     */
    ulCtlReg = (SAND_HAL_SET_FIELD( KA, EP_BM_BF_MEM_ACC_CTRL, ACK, 1 ) |
                SAND_HAL_SET_FIELD( KA, EP_BM_BF_MEM_ACC_CTRL, REQ, 0 ));
    SAND_HAL_WRITE( HalCtxt, KA, EP_BM_BF_MEM_ACC_CTRL, ulCtlReg );
    SAND_HAL_WRITE( HalCtxt, KA, EP_BM_BF_MEM_ACC_DATA, 0 );

    /*
     * Build up our command
     */
    ulCtlReg = SAND_HAL_SET_FIELD( KA, EP_BM_BF_MEM_ACC_CTRL, REQ, 1 ) |
        SAND_HAL_SET_FIELD( KA, EP_BM_BF_MEM_ACC_CTRL, RD_WR_N, 1 ) |
        SAND_HAL_SET_FIELD( KA, EP_BM_BF_MEM_ACC_CTRL, ADDR, ulOffset );

    /*
     * Write out our command and wait for the acknowledgement
     */
    SAND_HAL_WRITE( HalCtxt, KA, EP_BM_BF_MEM_ACC_CTRL, ulCtlReg );

    nTimeout = SB_QE2000_ELIB_IND_MEM_TIMEOUT;
    nAck = 0;

    while ( nTimeout-- )
    {
        ulCtlReg = SAND_HAL_READ( HalCtxt, KA, EP_BM_BF_MEM_ACC_CTRL );
        if (1 == SAND_HAL_GET_FIELD( KA, EP_BM_BF_MEM_ACC_CTRL, ACK, ulCtlReg ) )
        {
            /*
             * Grab the data & clear the ack
             */
            nAck = 1;
            *pulData = BSWAP32(SAND_HAL_READ( HalCtxt, KA, EP_BM_BF_MEM_ACC_DATA ));
            ulCtlReg = SAND_HAL_MOD_FIELD( KA, EP_BM_BF_MEM_ACC_CTRL, ACK, ulCtlReg, 1 );
            ulCtlReg = SAND_HAL_MOD_FIELD( KA, EP_BM_BF_MEM_ACC_CTRL, REQ, ulCtlReg, 0 );
            SAND_HAL_WRITE( HalCtxt, KA, EP_BM_BF_MEM_ACC_CTRL, ulCtlReg );
            break;
        }

        thin_delay( 100 );
    }

    if ( 0 == nAck )
    {
        return( SB_ELIB_IND_MEM_TIMEOUT );
    }

    return( SB_ELIB_OK );

}



sbElibStatus_et sbQe2000ElibBfMemWrite( sbhandle HalCtxt,
                    uint32 ulOffset,
                    uint32 ulData0,
                    uint32 ulData1 )
{
    int nTimeout;
    int nAck;
    uint32 ulCtlReg;

    SB_ASSERT( ulOffset < SB_QE2000_ELIB_BF_MEM_MAX_OFFSET );

    /*
     * Clear out any previous acks in the mem ctrl register
     */
    ulCtlReg = (SAND_HAL_SET_FIELD( KA, EP_MM_BF_MEM_ACC_CTRL, ACK, 1 ) |
                SAND_HAL_SET_FIELD( KA, EP_MM_BF_MEM_ACC_CTRL, REQ, 0 ));
    SAND_HAL_WRITE( HalCtxt, KA, EP_MM_BF_MEM_ACC_CTRL, ulCtlReg );

    /*
     * Build up our command
     */
    ulCtlReg = SAND_HAL_SET_FIELD( KA, EP_MM_BF_MEM_ACC_CTRL, REQ, 1 ) |
        SAND_HAL_SET_FIELD( KA, EP_MM_BF_MEM_ACC_CTRL, RD_WR_N, 0 ) |
        SAND_HAL_SET_FIELD( KA, EP_MM_BF_MEM_ACC_CTRL, ADDR, ulOffset );

    /*
     * Write out our data
     */
    SAND_HAL_WRITE( HalCtxt, KA, EP_MM_BF_MEM_ACC_DATA0, BSWAP32(ulData0) );
    SAND_HAL_WRITE( HalCtxt, KA, EP_MM_BF_MEM_ACC_DATA1, BSWAP32(ulData1) );

    /*
     * Write out our command and wait for the acknowledgement
     */
    SAND_HAL_WRITE( HalCtxt, KA, EP_MM_BF_MEM_ACC_CTRL, ulCtlReg );

    nTimeout = SB_QE2000_ELIB_IND_MEM_TIMEOUT;
    nAck = 0;

    while ( nTimeout-- )
    {
        ulCtlReg = SAND_HAL_READ( HalCtxt, KA, EP_MM_BF_MEM_ACC_CTRL );
        if (1 == SAND_HAL_GET_FIELD( KA, EP_MM_BF_MEM_ACC_CTRL, ACK, ulCtlReg ) )
        {
            /*
             * Clear the ack
             */
            nAck = 1;
            ulCtlReg = SAND_HAL_MOD_FIELD( KA, EP_MM_BF_MEM_ACC_CTRL, ACK, ulCtlReg, 1 );
            ulCtlReg = SAND_HAL_MOD_FIELD( KA, EP_MM_BF_MEM_ACC_CTRL, REQ, ulCtlReg, 0 );
            SAND_HAL_WRITE( HalCtxt, KA, EP_MM_BF_MEM_ACC_CTRL, ulCtlReg );
            break;
        }

        thin_delay( 100 );
    }

    if ( 0 == nAck )
    {
        return( SB_ELIB_IND_MEM_TIMEOUT );
    }

    return( SB_ELIB_OK );

}

sbElibStatus_et sbQe2000ElibBfMemRead( sbhandle HalCtxt,
                           uint32 ulOffset,
                           uint32 ulClrOnRd,
                           uint32 *pulData0,
                           uint32 *pulData1 )
{
    int nTimeout;
    int nAck;
    uint32 ulCtlReg;

    SB_ASSERT( ulOffset < SB_QE2000_ELIB_BF_MEM_MAX_OFFSET );
    SB_ASSERT( pulData0 );
    SB_ASSERT( pulData1 );

    /*
     * Clear out any previous acks in the mem ctrl register & any stale data
     */
    ulCtlReg = (SAND_HAL_SET_FIELD( KA, EP_MM_BF_MEM_ACC_CTRL, ACK, 1 ) |
                SAND_HAL_SET_FIELD( KA, EP_MM_BF_MEM_ACC_CTRL, REQ, 0 ));
    SAND_HAL_WRITE( HalCtxt, KA, EP_MM_BF_MEM_ACC_CTRL, ulCtlReg );

    SAND_HAL_WRITE( HalCtxt, KA, EP_MM_BF_MEM_ACC_DATA0, 0 );
    SAND_HAL_WRITE( HalCtxt, KA, EP_MM_BF_MEM_ACC_DATA1, 0 );

    /*
     * Build up our command
     */
    ulCtlReg = SAND_HAL_SET_FIELD( KA, EP_MM_BF_MEM_ACC_CTRL, REQ, 1 ) |
        SAND_HAL_SET_FIELD( KA, EP_MM_BF_MEM_ACC_CTRL, RD_WR_N, 1 ) |
        SAND_HAL_SET_FIELD( KA, EP_MM_BF_MEM_ACC_CTRL, CLR_ON_RD, ulClrOnRd ) |
        SAND_HAL_SET_FIELD( KA, EP_MM_BF_MEM_ACC_CTRL, ADDR, ulOffset );

    /*
     * Write out our command and wait for the acknowledgement
     */
    SAND_HAL_WRITE( HalCtxt, KA, EP_MM_BF_MEM_ACC_CTRL, ulCtlReg );

    nTimeout = SB_QE2000_ELIB_IND_MEM_TIMEOUT;
    nAck = 0;

    while ( nTimeout-- )
    {
        ulCtlReg = SAND_HAL_READ( HalCtxt, KA, EP_MM_BF_MEM_ACC_CTRL );
        if (1 == SAND_HAL_GET_FIELD( KA, EP_MM_BF_MEM_ACC_CTRL, ACK, ulCtlReg ) )
        {
            /*
             * Grab the data & clear the ack
             */
            nAck = 1;
            *pulData0 = BSWAP32(SAND_HAL_READ( HalCtxt, KA, EP_MM_BF_MEM_ACC_DATA0 ));
            *pulData1 = BSWAP32(SAND_HAL_READ( HalCtxt, KA, EP_MM_BF_MEM_ACC_DATA1 ));
            ulCtlReg = SAND_HAL_MOD_FIELD( KA, EP_MM_BF_MEM_ACC_CTRL, ACK, ulCtlReg, 1 );
            ulCtlReg = SAND_HAL_MOD_FIELD( KA, EP_MM_BF_MEM_ACC_CTRL, REQ, ulCtlReg, 0 );
            SAND_HAL_WRITE( HalCtxt, KA, EP_MM_BF_MEM_ACC_CTRL, ulCtlReg );
            break;
        }

        thin_delay( 100 );
    }

    if ( 0 == nAck )
    {
        return( SB_ELIB_IND_MEM_TIMEOUT );
    }

    return( SB_ELIB_OK );

}



sbElibStatus_et sbQe2000ElibIpMemWrite( sbhandle HalCtxt,
                    uint32 ulOffset,
                    uint32 ulData0,
                    uint32 ulData1 )
{
    int nTimeout;
    int nAck;
    uint32 ulCtlReg;

    /*
      The proper way to bounds-check offset is to read ep_mm_config and compare depending
      on how much memory is allocated to IP. But this seems costly for a low-risk possibility.

    uint32 ep_mm_config = SAND_HAL_READ( HalCtxt, KA, EP_MM_CONFIG);
    if (SAND_HAL_GET_FIELD( KA, EP_MM_CONFIG, ENABLE,  ep_mm_config) == 1) {
      SB_ASSERT(ulOffset < 0x8000);
    } else if (SAND_HAL_GET_FIELD( KA, EP_MM_CONFIG, ENABLE,  ep_mm_config) == 3) {
          SB_ASSERT( ulOffset < 0x4000);
    } else {
      return SB_ELIB_BAD_ARGS;
    }
    */
    SB_ASSERT( ulOffset < SB_QE2000_ELIB_IP_MEM_MAX_OFFSET );

    /*
     * Clear out any previous acks in the mem ctrl register
     */
    ulCtlReg = (SAND_HAL_SET_FIELD( KA, EP_MM_IP_MEM_ACC_CTRL, ACK, 1 ) |
                SAND_HAL_SET_FIELD( KA, EP_MM_IP_MEM_ACC_CTRL, REQ, 0 ));
    SAND_HAL_WRITE( HalCtxt, KA, EP_MM_IP_MEM_ACC_CTRL, ulCtlReg );

    /*
     * Build up our command
     */
    ulCtlReg = SAND_HAL_SET_FIELD( KA, EP_MM_IP_MEM_ACC_CTRL, REQ, 1 ) |
        SAND_HAL_SET_FIELD( KA, EP_MM_IP_MEM_ACC_CTRL, RD_WR_N, 0 ) |
        SAND_HAL_SET_FIELD( KA, EP_MM_IP_MEM_ACC_CTRL, ADDR, ulOffset );

    /*
     * Write out our data
     */
    SAND_HAL_WRITE( HalCtxt, KA, EP_MM_IP_MEM_ACC_DATA0, BSWAP32(ulData0) );
    SAND_HAL_WRITE( HalCtxt, KA, EP_MM_IP_MEM_ACC_DATA1, BSWAP32(ulData1) );

    /*
     * Write out our command and wait for the acknowledgement
     */
    SAND_HAL_WRITE( HalCtxt, KA, EP_MM_IP_MEM_ACC_CTRL, ulCtlReg );

    nTimeout = SB_QE2000_ELIB_IND_MEM_TIMEOUT;
    nAck = 0;

    while ( nTimeout-- )
    {
        ulCtlReg = SAND_HAL_READ( HalCtxt, KA, EP_MM_IP_MEM_ACC_CTRL );
        if (1 == SAND_HAL_GET_FIELD( KA, EP_MM_IP_MEM_ACC_CTRL, ACK, ulCtlReg ) )
        {
            /*
             * Clear the ack
             */
            nAck = 1;
            ulCtlReg = SAND_HAL_MOD_FIELD( KA, EP_MM_IP_MEM_ACC_CTRL, ACK, ulCtlReg, 1 );
            ulCtlReg = SAND_HAL_MOD_FIELD( KA, EP_MM_IP_MEM_ACC_CTRL, REQ, ulCtlReg, 0 );
            SAND_HAL_WRITE( HalCtxt, KA, EP_MM_IP_MEM_ACC_CTRL, ulCtlReg );
            break;
        }

        thin_delay( 100 );
    }

    if ( 0 == nAck )
    {
        return( SB_ELIB_IND_MEM_TIMEOUT );
    }

    return( SB_ELIB_OK );

}

sbElibStatus_et sbQe2000ElibIpMemRead( sbhandle HalCtxt,
                                       uint32 ulOffset,
                                       uint32 ulClrOnRd,
                                       uint32 *pulData0,
                                       uint32 *pulData1 )
{
    int nTimeout;
    int nAck;
    uint32 ulCtlReg;

    SB_ASSERT( ulOffset < SB_QE2000_ELIB_IP_MEM_MAX_OFFSET );
    SB_ASSERT( pulData0 );
    SB_ASSERT( pulData1 );

    /*
     * Clear out any previous acks in the mem ctrl register & any stale data
     */
    ulCtlReg = (SAND_HAL_SET_FIELD( KA, EP_MM_IP_MEM_ACC_CTRL, ACK, 1 ) |
                SAND_HAL_SET_FIELD( KA, EP_MM_IP_MEM_ACC_CTRL, REQ, 0 ));
    SAND_HAL_WRITE( HalCtxt, KA, EP_MM_IP_MEM_ACC_CTRL, ulCtlReg );
    SAND_HAL_WRITE( HalCtxt, KA, EP_MM_IP_MEM_ACC_DATA0, 0 );
    SAND_HAL_WRITE( HalCtxt, KA, EP_MM_IP_MEM_ACC_DATA1, 0 );

    /*
     * Build up our command
     */
    ulCtlReg = SAND_HAL_SET_FIELD( KA, EP_MM_IP_MEM_ACC_CTRL, REQ, 1 ) |
        SAND_HAL_SET_FIELD( KA, EP_MM_IP_MEM_ACC_CTRL, CLR_ON_RD, ulClrOnRd ) |
        SAND_HAL_SET_FIELD( KA, EP_MM_IP_MEM_ACC_CTRL, RD_WR_N, 1 ) |
        SAND_HAL_SET_FIELD( KA, EP_MM_IP_MEM_ACC_CTRL, ADDR, ulOffset );

    /*
     * Write out our command and wait for the acknowledgement
     */
    SAND_HAL_WRITE( HalCtxt, KA, EP_MM_IP_MEM_ACC_CTRL, ulCtlReg );

    nTimeout = SB_QE2000_ELIB_IND_MEM_TIMEOUT;
    nAck = 0;

    while ( nTimeout-- )
    {
        ulCtlReg = SAND_HAL_READ( HalCtxt, KA, EP_MM_IP_MEM_ACC_CTRL );
        if (1 == SAND_HAL_GET_FIELD( KA, EP_MM_IP_MEM_ACC_CTRL, ACK, ulCtlReg ) )
        {
            /*
             * Grab the data & clear the ack
             */
            nAck = 1;
            *pulData0 = BSWAP32(SAND_HAL_READ( HalCtxt, KA, EP_MM_IP_MEM_ACC_DATA0 ));
            *pulData1 = BSWAP32(SAND_HAL_READ( HalCtxt, KA, EP_MM_IP_MEM_ACC_DATA1 ));
            ulCtlReg = SAND_HAL_MOD_FIELD( KA, EP_MM_IP_MEM_ACC_CTRL, ACK, ulCtlReg, 1 );
            ulCtlReg = SAND_HAL_MOD_FIELD( KA, EP_MM_IP_MEM_ACC_CTRL, REQ, ulCtlReg, 0 );
            SAND_HAL_WRITE( HalCtxt, KA, EP_MM_IP_MEM_ACC_CTRL, ulCtlReg );
            break;
        }

        thin_delay( 100 );
    }

    if ( 0 == nAck )
    {
        return( SB_ELIB_IND_MEM_TIMEOUT );
    }

    return( SB_ELIB_OK );

}




sbElibStatus_et sbQe2000ElibEbMemWrite( sbhandle HalCtxt,
                    uint32 ulOffset,
                    uint32 ulData[8])
{
    int nTimeout;
    int nAck;
    uint32 ulCtlReg;

    SB_ASSERT( ulOffset < (SB_QE2000_ELIB_EB_MEM_MAX_OFFSET + 1));

    /*
     * Clear out any previous acks in the mem ctrl register
     */
    ulCtlReg = (SAND_HAL_SET_FIELD( KA, EB_MEM_ACC_CTRL, ACK, 1 ) |
                SAND_HAL_SET_FIELD( KA, EB_MEM_ACC_CTRL, REQ, 0 ));
    SAND_HAL_WRITE( HalCtxt, KA, EB_MEM_ACC_CTRL, ulCtlReg );

    /*
     * Build up our command
     */
    /* BUG 21826: Use ENTIRE mem slice section */
    ulCtlReg = SAND_HAL_SET_FIELD( KA, EB_MEM_ACC_CTRL, REQ, 1 ) |
        SAND_HAL_SET_FIELD( KA, EB_MEM_ACC_CTRL, RD_WR_N, 0 ) |
        SAND_HAL_SET_FIELD( KA, EB_MEM_ACC_CTRL, WRITE_MASK, 0xFF) |
        SAND_HAL_SET_FIELD( KA, EB_MEM_ACC_CTRL, ADDR, ulOffset );

    /*
     * Write out our data
     */
    SAND_HAL_WRITE( HalCtxt, KA, EB_MEM_ACC_SLICE0, BSWAP32(ulData[0]) );
    SAND_HAL_WRITE( HalCtxt, KA, EB_MEM_ACC_SLICE1, BSWAP32(ulData[1]) );
    SAND_HAL_WRITE( HalCtxt, KA, EB_MEM_ACC_SLICE2, BSWAP32(ulData[2]) );
    SAND_HAL_WRITE( HalCtxt, KA, EB_MEM_ACC_SLICE3, BSWAP32(ulData[3]) );
    SAND_HAL_WRITE( HalCtxt, KA, EB_MEM_ACC_SLICE4, BSWAP32(ulData[4]) );
    SAND_HAL_WRITE( HalCtxt, KA, EB_MEM_ACC_SLICE5, BSWAP32(ulData[5]) );
    SAND_HAL_WRITE( HalCtxt, KA, EB_MEM_ACC_SLICE6, BSWAP32(ulData[6]) );
    SAND_HAL_WRITE( HalCtxt, KA, EB_MEM_ACC_SLICE7, BSWAP32(ulData[7]) );

    /*
     * Write out our command and wait for the acknowledgement
     */
    SAND_HAL_WRITE( HalCtxt, KA, EB_MEM_ACC_CTRL, ulCtlReg );

    nTimeout = SB_QE2000_ELIB_IND_MEM_TIMEOUT;
    nAck = 0;

    while ( nTimeout-- )
    {
        ulCtlReg = SAND_HAL_READ( HalCtxt, KA, EB_MEM_ACC_CTRL );
        if (1 == SAND_HAL_GET_FIELD( KA, EB_MEM_ACC_CTRL, ACK, ulCtlReg ) )
        {
            /*
             * Clear the ack
             */
            nAck = 1;
            ulCtlReg = SAND_HAL_MOD_FIELD( KA, EB_MEM_ACC_CTRL, ACK, ulCtlReg, 1 );
            ulCtlReg = SAND_HAL_MOD_FIELD( KA, EB_MEM_ACC_CTRL, REQ, ulCtlReg, 0 );
            SAND_HAL_WRITE( HalCtxt, KA, EB_MEM_ACC_CTRL, ulCtlReg );
            break;
        }

        thin_delay( 100 );
    }

    if ( 0 == nAck )
    {
        return( SB_ELIB_IND_MEM_TIMEOUT );
    }

    return( SB_ELIB_OK );

}

sbElibStatus_et sbQe2000ElibEbMemRead( sbhandle HalCtxt,
                   uint32 ulOffset,
                   uint32 pulData[8])
{
    int nTimeout;
    int nAck;
    uint32 ulCtlReg;

    SB_ASSERT( ulOffset < (SB_QE2000_ELIB_EB_MEM_MAX_OFFSET+1) );

    /*
     * Clear out any previous acks in the mem ctrl register
     */
    ulCtlReg = (SAND_HAL_SET_FIELD( KA, EB_MEM_ACC_CTRL, ACK, 1 ) |
                SAND_HAL_SET_FIELD( KA, EB_MEM_ACC_CTRL, REQ, 0 ));
    SAND_HAL_WRITE( HalCtxt, KA, EB_MEM_ACC_CTRL, ulCtlReg );

    /*
     * Build up our command
     */
    ulCtlReg = SAND_HAL_SET_FIELD( KA, EB_MEM_ACC_CTRL, REQ, 1 ) |
        SAND_HAL_SET_FIELD( KA, EB_MEM_ACC_CTRL, RD_WR_N, 1 ) |
        SAND_HAL_SET_FIELD( KA, EB_MEM_ACC_CTRL, ADDR, ulOffset );

    /*
     * Write out our command and wait for the acknowledgement
     */
    SAND_HAL_WRITE( HalCtxt, KA, EB_MEM_ACC_CTRL, ulCtlReg );

    nTimeout = SB_QE2000_ELIB_IND_MEM_TIMEOUT;
    nAck = 0;

    while ( nTimeout-- )
    {
        ulCtlReg = SAND_HAL_READ( HalCtxt, KA, EB_MEM_ACC_CTRL );
        if (1 == SAND_HAL_GET_FIELD( KA, EB_MEM_ACC_CTRL, ACK, ulCtlReg ) )
        {
            /*
             * Grab the data & clear the ack
             */
            nAck = 1;
            pulData[0] = BSWAP32(SAND_HAL_READ( HalCtxt, KA, EB_MEM_ACC_SLICE0 ));
            pulData[1] = BSWAP32(SAND_HAL_READ( HalCtxt, KA, EB_MEM_ACC_SLICE1 ));
            pulData[2] = BSWAP32(SAND_HAL_READ( HalCtxt, KA, EB_MEM_ACC_SLICE2 ));
            pulData[3] = BSWAP32(SAND_HAL_READ( HalCtxt, KA, EB_MEM_ACC_SLICE3 ));
            pulData[4] = BSWAP32(SAND_HAL_READ( HalCtxt, KA, EB_MEM_ACC_SLICE4 ));
            pulData[5] = BSWAP32(SAND_HAL_READ( HalCtxt, KA, EB_MEM_ACC_SLICE5 ));
            pulData[6] = BSWAP32(SAND_HAL_READ( HalCtxt, KA, EB_MEM_ACC_SLICE6 ));
            pulData[7] = BSWAP32(SAND_HAL_READ( HalCtxt, KA, EB_MEM_ACC_SLICE7 ));

            ulCtlReg = SAND_HAL_MOD_FIELD( KA, EB_MEM_ACC_CTRL, ACK, ulCtlReg, 1 );
            ulCtlReg = SAND_HAL_MOD_FIELD( KA, EB_MEM_ACC_CTRL, REQ, ulCtlReg, 0 );
            SAND_HAL_WRITE( HalCtxt, KA, EB_MEM_ACC_CTRL, ulCtlReg );
            break;
        }

        thin_delay( 100 );
    }

    if ( 0 == nAck )
    {
        return( SB_ELIB_IND_MEM_TIMEOUT );
    }

    return( SB_ELIB_OK );

}

sbElibStatus_et sbQe2000ElibMvtEntryReadRaw( sbhandle HalCtxt, uint32 ulIndex, uint32 pulData[8] )
{
    /* BUG 21826: change to volatile */
    volatile uint32 nGrpSize;
    volatile uint32 ulRow;
    volatile uint32 ulColumn;
    int nStatus;


    nGrpSize = SAND_HAL_READ( HalCtxt, KA, EG_MC_CONFIG0 );
    nGrpSize = SAND_HAL_GET_FIELD( KA, EG_MC_CONFIG0, MCGROUP_SIZE, nGrpSize );
    ulRow = SB_QE2000_ELIB_EB_MVT_ROW_GET(ulIndex, nGrpSize);
    ulColumn = SB_QE2000_ELIB_EB_MVT_COL_GET(ulIndex, nGrpSize);

    /* coverity[assert_side_effect] */
    SB_ASSERT( ulIndex < SB_QE2000_ELIB_EB_MVT_MIN(nGrpSize));
    if (ulIndex >= SB_QE2000_ELIB_EB_MVT_MIN(nGrpSize)) {
        return SOC_E_INTERNAL;
    }
    /* coverity[assert_side_effect] */
    SB_ASSERT( ulColumn < 3 );
    if (ulColumn >= 3) {
        return SOC_E_INTERNAL;
    }


    nStatus = sbQe2000ElibEbMemRead( HalCtxt, ulRow, pulData );

    return( nStatus );

}

sbElibStatus_et sbQe2000ElibMvtEntryWriteRaw( sbhandle HalCtxt, uint32 ulIndex, uint32 pulData[8] )
{
    /* BUG 21826: change to volatile */
    volatile uint32 nGrpSize;
    volatile uint32 ulRow;
    volatile uint32 ulColumn;
    int nStatus;


    nGrpSize = SAND_HAL_READ( HalCtxt, KA, EG_MC_CONFIG0 );
    nGrpSize = SAND_HAL_GET_FIELD( KA, EG_MC_CONFIG0, MCGROUP_SIZE, nGrpSize );
    ulRow = SB_QE2000_ELIB_EB_MVT_ROW_GET(ulIndex, nGrpSize);
    ulColumn = SB_QE2000_ELIB_EB_MVT_COL_GET(ulIndex, nGrpSize);

    /* coverity[assert_side_effect] */
    SB_ASSERT( ulIndex < SB_QE2000_ELIB_EB_MVT_MIN(nGrpSize));

    if (ulIndex >= SB_QE2000_ELIB_EB_MVT_MIN(nGrpSize)) {
        return SOC_E_INTERNAL;
    }
    /* coverity[assert_side_effect] */
    SB_ASSERT( ulColumn < 3 );
    if (ulColumn >= 3) {
        return SOC_E_INTERNAL;
    }

    nStatus = sbQe2000ElibEbMemWrite( HalCtxt, ulRow, pulData );

    return( nStatus );

}

