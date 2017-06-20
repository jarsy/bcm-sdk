/* iProcSmbus.c */

/*
 * Copyright (c) 2013 Wind River Systems, Inc.
 *
 * The right to copy, distribute, modify or otherwise make use
 * of this software may be licensed only pursuant to the terms
 * of an applicable Wind River license agreement.
 */
 
/*
modification history
--------------------
01a,15nov13,dnb  written from u-boot reference code
*/

#include <vxWorks.h>
#include "config.h"
#include <taskLib.h>
#include "iproc_i2c_regs.h"

#define IPROC_SMB_MAX_RETRIES 3

#define I2C_SPEED_100KHz	0
#define I2C_SPEED_400KHz	1
#define I2C_SPEED_INVALID	255

/* SMBUS protocol values defined in register 0x30 */
#define SMBUS_PROT_QUICK_CMD               0 
#define SMBUS_PROT_SEND_BYTE               1 
#define SMBUS_PROT_RECV_BYTE               2 
#define SMBUS_PROT_WR_BYTE                 3 
#define SMBUS_PROT_RD_BYTE                 4 
#define SMBUS_PROT_WR_WORD                 5 
#define SMBUS_PROT_RD_WORD                 6 
#define SMBUS_PROT_BLK_WR                  7 
#define SMBUS_PROT_BLK_RD                  8 
#define SMBUS_PROT_PROC_CALL               9 
#define SMBUS_PROT_BLK_WR_BLK_RD_PROC_CALL 10 
#define SMBUS_PROT_INVALID                 0xff

#define MSTR_STS_XACT_SUCCESS          0
#define MSTR_STS_LOST_ARB              1
#define MSTR_STS_NACK_FIRST_BYTE       2
#define MSTR_STS_NACK_NON_FIRST_BYTE   3 /* NACK on a byte other than 
                                            the first byte */
#define MSTR_STS_TTIMEOUT_EXCEEDED     4
#define MSTR_STS_TX_TLOW_MEXT_EXCEEDED 5
#define MSTR_STS_RX_TLOW_MEXT_EXCEEDED 6

void sysUsDelay(INT32 count);

LOCAL UINT32 IPROC_I2C_REG_READ
    (
    UINT32 offset
    )
    {
    return *((UINT32 *)(IPROC_SMBUS_BASE_ADDR + offset));
    }

LOCAL void IPROC_I2C_REG_WRITE
    (
    UINT32 offset, 
    UINT32 val
    )
    {
    UINT32 *p = (UINT32 *)(IPROC_SMBUS_BASE_ADDR + offset);
    *p = val;
    }

#undef LOCAL
#define LOCAL
/*******************************************************************************
*
* iProcI2CStartBusyWait
*/

LOCAL STATUS iProcI2CStartBusyWait(void)
    {
    UINT32	val;
   
    val = IPROC_I2C_REG_READ(CCB_SMB_MSTRCMD_REG);

    /* Check if an operation is in progress.
     */
    if (val & CCB_SMB_MSTRSTARTBUSYCMD_MASK) 
	{
        UINT32 i = 0;

        do 
	    {
	    taskDelay(1);
            i += 1;
	    val = IPROC_I2C_REG_READ(CCB_SMB_MSTRCMD_REG);
            }
	while((val & CCB_SMB_MSTRSTARTBUSYCMD_MASK) &&
	      (i < IPROC_SMB_MAX_RETRIES));

	if(val & CCB_SMB_MSTRSTARTBUSYCMD_MASK)
	    {
	    return ERROR;
	    }
	}
    return OK;
    }

/*******************************************************************************
*
* iProcI2CWriteTransData
*/

LOCAL VOID iProcI2CWriteTransData
    (
    UINT8	chip,
    UINT8	command,
    UINT8	smbProto,
    UINT8       *data,
    UINT16      dataLen

    )
    {
    UINT32	numDataBytes = 0;
    UINT32	val;
    UINT32	i;

    chip <<=1; /* shift 7-bit address up */

    /* write 7-bit address */
    IPROC_I2C_REG_WRITE(CCB_SMB_MSTRDATAWR_REG, chip);

    /* If the protocol needs command code, copy it */
    if(smbProto !=  SMBUS_PROT_INVALID)
    	IPROC_I2C_REG_WRITE(CCB_SMB_MSTRDATAWR_REG, command);
	

    /* Depending on the SMBus protocol, we need to write additional transaction
     * data in to Tx FIFO. Refer to section 5.5 of SMBus spec for sequence for a
     * transaction
     */
    switch (smbProto) 
	{
        case SMBUS_PROT_RECV_BYTE:
            /* No additional data to be written */
            break;

        case SMBUS_PROT_RD_BYTE:
        case SMBUS_PROT_RD_WORD:
        case SMBUS_PROT_BLK_RD:
            /* Write slave address with R/W~ set (bit #0) */
            IPROC_I2C_REG_WRITE(CCB_SMB_MSTRDATAWR_REG, chip | 0x1);
            break;

        case SMBUS_PROT_SEND_BYTE:
        case SMBUS_PROT_WR_BYTE:
        case SMBUS_PROT_WR_WORD:
            /* No additional bytes to be written. Data portion is written in the
             * 'for' loop below
             */
            numDataBytes = dataLen;
            break;

        case SMBUS_PROT_BLK_WR:
            /* 3rd byte is byte count */
            IPROC_I2C_REG_WRITE(CCB_SMB_MSTRDATAWR_REG, dataLen);
            numDataBytes = dataLen;
            break;

        default:
            break;

	}

    /* Copy actual data from caller, next. In general, for reads, no data is
     * copied
     */
    for (i = 0; numDataBytes; --numDataBytes, i++) 
	{

        /* For the last byte, set MASTER_WR_STATUS bit */
        val = (numDataBytes == 1) ?  
	    data[i] | CCB_SMB_MSTRWRSTS_MASK : data[i];

        IPROC_I2C_REG_WRITE(CCB_SMB_MSTRDATAWR_REG, val);
	}

    return;
    }


/*******************************************************************************
*
* iProcI2CDataSend
*/

LOCAL STATUS iProcI2CDataSend
    (
    UINT8	chip,
    UINT8	addr,
    UINT8	*pData,
    UINT16	len,
    UINT16	flags,
    UINT8	proto
    )
    {
    STATUS	retCode;
    UINT32	val;
    int		retry = 3;

    retCode = iProcI2CStartBusyWait();

    if(retCode == OK)
	{
	/* Write transaction bytes to Tx FIFO */
	iProcI2CWriteTransData(chip,addr,proto,pData,len);

	val = (proto << CCB_SMB_MSTRSMBUSPROTO_SHIFT) |
	      CCB_SMB_MSTRSTARTBUSYCMD_MASK;

	/* Program master command register (0x30) with protocol type and set
	 * start_busy_command bit to initiate the write transaction
	 */
	IPROC_I2C_REG_WRITE(CCB_SMB_MSTRCMD_REG,val);

	/* Check for Master status */
	do
	    {
            val = IPROC_I2C_REG_READ(CCB_SMB_MSTRCMD_REG);
            if(!(val & CCB_SMB_MSTRSTARTBUSYCMD_MASK))
		break;
	    sysUsDelay(100);
	    retry -=1;
	    }
	while(retry <=0);
	    
	if(!(val & CCB_SMB_MSTRSTARTBUSYCMD_MASK))
	    {
            val &= CCB_SMB_MSTRSTS_MASK;
            val >>= CCB_SMB_MSTRSTS_SHIFT;
	    if (val != MSTR_STS_XACT_SUCCESS) 
		retCode = ERROR;
	    }
	else
	    retCode = ERROR;
	}
    return retCode;
   }        

/*******************************************************************************
*
* iProcI2CDataRecv
*/

LOCAL STATUS iProcI2CDataRecv
    (
    UINT8	chip,
    UINT8	addr,
    UINT8	*pData,
    UINT16	len,
    UINT16	flags,
    UINT8	proto,
    UINT32	*numBytesRead
    )
    {
    STATUS	retCode;
    UINT32	val;
    int		retry = 3;

    retCode = iProcI2CStartBusyWait();

    if(retCode == OK)
	{
	/* Write transaction bytes to Tx FIFO */
	iProcI2CWriteTransData(chip,addr,proto,pData,len);

 	/* Program master command register (0x30) with protocol type and set
	 * start_busy_command bit to initiate the write transaction
	 */
	val = (proto << CCB_SMB_MSTRSMBUSPROTO_SHIFT) |
              CCB_SMB_MSTRSTARTBUSYCMD_MASK | len;

	IPROC_I2C_REG_WRITE(CCB_SMB_MSTRCMD_REG, val);

	/* Check for Master status */
	do
	    {
            val = IPROC_I2C_REG_READ(CCB_SMB_MSTRCMD_REG);
            if(!(val & CCB_SMB_MSTRSTARTBUSYCMD_MASK))
		break;
	    sysUsDelay(100);
	    retry -=1;
	    }
	while(retry <=0);

	/* If start_busy bit cleared, check if there are any errors */
	if (!(val & CCB_SMB_MSTRSTARTBUSYCMD_MASK)) 
	    {
	    /* start_busy bit cleared, check master_status field now */
	    val &= CCB_SMB_MSTRSTS_MASK;
	    val >>= CCB_SMB_MSTRSTS_SHIFT;

	    if (val != MSTR_STS_XACT_SUCCESS) 
		{
		return ERROR;
		}
	    }

	/* Read received byte(s), after TX out address etc */
	val = IPROC_I2C_REG_READ(CCB_SMB_MSTRDATARD_REG);

	/* For block read, protocol (hw) returns byte count, as the first byte */
	if (proto == SMBUS_PROT_BLK_RD) 
	    {
	    int i;

	    *numBytesRead = val & CCB_SMB_MSTRRDDATA_MASK;

	    for (i = 0; i < *numBytesRead; i++)
		{
		/* Read Rx FIFO for data bytes */
		val = IPROC_I2C_REG_READ(CCB_SMB_MSTRDATARD_REG);
		pData[i] = val & CCB_SMB_MSTRRDDATA_MASK;
		}
	    }
	else 
	    {
	    /* 1 Byte data */
	    *pData = val & CCB_SMB_MSTRRDDATA_MASK;
	    *numBytesRead = 1;
	    }

	retCode = OK;
	}

    return retCode;
    }

/*******************************************************************************
*
* iProcI2CWriteByte
*/

LOCAL STATUS iProcI2CWriteByte 
    (
    UINT8 chip,
    UINT8 addr,
    UINT8 val
    )
    {
    return iProcI2CDataSend(chip, addr, &val, 1, 0, SMBUS_PROT_WR_BYTE);
    }

/*******************************************************************************
*
* iProcI2CReadByte
*/

LOCAL STATUS iProcI2CReadByte
    (
    UINT8 chip,
    UINT8 addr,
    UINT8 *val
    )
    {
    UINT32 numBytesRead;
    return iProcI2CDataRecv(chip, addr, val, 0, 0, SMBUS_PROT_RD_BYTE,&numBytesRead);
    }

/*******************************************************************************
*
* iProcI2CSetClkFreq
*/

LOCAL STATUS iProcI2CSetClkFreq(UINT32 freq)
    {
    UINT32	val;

    if( freq > 1 )
	return ERROR;

    val = IPROC_I2C_REG_READ(CCB_SMB_TIMGCFG_REG);
    val &= ~CCB_SMB_TIMGCFG_MODE400_MASK;
    val |= (freq << CCB_SMB_TIMGCFG_MODE400_SHIFT);

    IPROC_I2C_REG_WRITE(CCB_SMB_TIMGCFG_REG, val);

    return OK;
    }

/*******************************************************************************
*
* iProcI2CInit
*/

void iProcI2CInit( void)
    {
    UINT32	val;

    /* Flush Tx, Rx FIFOs. Note we are setting the Rx FIFO threshold to 0.
     * May be OK since we are setting RX_EVENT and RX_FIFO_FULL interrupts
     */
    val = CCB_SMB_MSTRRXFIFOFLSH_MASK | CCB_SMB_MSTRTXFIFOFLSH_MASK;
    IPROC_I2C_REG_WRITE(CCB_SMB_MSTRFIFOCTL_REG, val);

    /* Enable SMbus block. Note, we are setting MASTER_RETRY_COUNT to zero
     * since there will be only one master
     */
    val = CCB_SMB_CFG_SMBEN_MASK;
    IPROC_I2C_REG_WRITE(CCB_SMB_CFG_REG, val);

    /* Wait a minimum of 50 Usec, as per SMB hw doc. But we wait longer */
    sysUsDelay(100);

    /* Set default clock frequency */
    iProcI2CSetClkFreq(I2C_SPEED_100KHz);

    /* Disable intrs */
    val = 0x0;
    IPROC_I2C_REG_WRITE(CCB_SMB_EVTEN_REG, val);

    /* Clear intrs (W1TC) */
    val = IPROC_I2C_REG_READ(CCB_SMB_EVTSTS_REG);    
    IPROC_I2C_REG_WRITE(CCB_SMB_EVTSTS_REG, val);

    return;
    }

/*******************************************************************************
*
* iProcI2CWrite
*/

STATUS iProcI2CWrite
    (
    UINT8	chip,
    UINT32	addr,
    UINT32      addrLen,
    UINT8	*data,
    UINT32	dataLen
    )
    {
    UINT32 i;
    STATUS retCode = ERROR;

    if((addrLen > 1) || ((addr + dataLen) > 256))
	return retCode;

    for(i = 0; i < dataLen; i++)
	{
	if((retCode = iProcI2CWriteByte(chip, addr+i, data[i])) != OK)
	    {
	    iProcI2CInit();
	    return retCode;
	    }
	}
    return retCode;
    }


/*******************************************************************************
*
* iProcI2CRead
*/

STATUS iProcI2CRead
    (
    UINT8	chip,
    UINT32	addr,
    UINT32      addrLen,
    UINT8	*data,
    UINT32	dataLen
    )
    {
    UINT32 i;
    STATUS retCode = ERROR;

    if((addrLen > 1) || ((addr + dataLen) > 256))
	return retCode;

    for(i = 0; i < dataLen; i++)
	{
	if((retCode = iProcI2CReadByte(chip, addr+i, &data[i])) != OK)
	    {
	    iProcI2CInit();
	    return retCode;
	    }
	}
    return retCode;
    }


#ifdef DEBUG
#include <stdio.h>
gedit qqq 
void iProcDumpI2CRegs(void)
{
    unsigned int regval;

    printf("\n----------------------------------------------\n");
    printf("Dumping SMBus registers... \n");

    regval = IPROC_I2C_REG_READ(CCB_SMB_CFG_REG);
    printf("CCB_SMB_CFG_REG=0x%08X\n", regval);

    regval = IPROC_I2C_REG_READ(CCB_SMB_TIMGCFG_REG);
    printf("CCB_SMB_TIMGCFG_REG=0x%08X\n", regval);

    regval = IPROC_I2C_REG_READ(CCB_SMB_ADDR_REG);
    printf("CCB_SMB_ADDR_REG=0x%08X\n", regval);

    regval = IPROC_I2C_REG_READ(CCB_SMB_MSTRFIFOCTL_REG);
    printf("CCB_SMB_MSTRFIFOCTL_REG=0x%08X\n", regval);

    regval = IPROC_I2C_REG_READ(CCB_SMB_SLVFIFOCTL_REG);
    printf("CCB_SMB_SLVFIFOCTL_REG=0x%08X\n", regval);

    regval = IPROC_I2C_REG_READ(CCB_SMB_BITBANGCTL_REG);
    printf("CCB_SMB_BITBANGCTL_REG=0x%08X\n", regval);

    regval = IPROC_I2C_REG_READ(CCB_SMB_MSTRCMD_REG);
    printf("CCB_SMB_MSTRCMD_REG=0x%08X\n", regval);

    regval = IPROC_I2C_REG_READ(CCB_SMB_SLVCMD_REG);
    printf("CCB_SMB_SLVCMD_REG=0x%08X\n", regval);

    regval = IPROC_I2C_REG_READ(CCB_SMB_EVTEN_REG);
    printf("CCB_SMB_EVTEN_REG=0x%08X\n", regval);

    regval = IPROC_I2C_REG_READ(CCB_SMB_EVTSTS_REG);
    printf("CCB_SMB_EVTSTS_REG=0x%08X\n", regval);

    regval = IPROC_I2C_REG_READ(CCB_SMB_MSTRDATAWR_REG);
    printf("CCB_SMB_MSTRDATAWR_REG=0x%08X\n", regval);

    regval = IPROC_I2C_REG_READ(CCB_SMB_MSTRDATARD_REG);
    printf("CCB_SMB_MSTRDATARD_REG=0x%08X\n", regval);

    regval = IPROC_I2C_REG_READ(CCB_SMB_SLVDATAWR_REG);
    printf("CCB_SMB_SLVDATAWR_REG=0x%08X\n", regval);

    regval = IPROC_I2C_REG_READ(CCB_SMB_SLVDATARD_REG);
    printf("CCB_SMB_SLVDATARD_REG=0x%08X\n", regval);

    printf("----------------------------------------------\n\n");
}
#endif







