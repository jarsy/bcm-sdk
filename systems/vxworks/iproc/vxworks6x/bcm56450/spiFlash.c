/* spiFlash.c - flash memory device driver */

/*
 * Copyright (c) 2010,2013 Wind River Systems, Inc.
 *
 * The right to copy, distribute, modify or otherwise make use
 * of this software may be licensed only pursuant to the terms
 * of an applicable Wind River license agreement.
 */
 
/*
modification history
--------------------
01a,13nov13,dnb written from spS25flxxSpiFlash.c v 01d
*/

/*
DESCRIPTION
This library contains routines to manipulate flash memory. Read and write
routines are included.
*/
#include <vxWorks.h>
#include <stdlib.h>
#include "config.h"
#include "iProcQspi.h"

LOCAL struct spi_slave *spiHandle = NULL; 

/*******************************************************************************
*
* spiFlashReadWrite
*
* This is the SPI flash data read/write routine.
*
* RETURNS: OK or ERROR if there is an error.
*/

LOCAL STATUS spiFlashReadWrite
    (
    struct spi_slave 	*spi, 
    UINT8  		*cmd,
    size_t        	 cmdLen,
    UINT8		*dataOut,
    UINT8  		*dataIn,
    size_t        	 dataLen
    )
    {
    STATUS stat;

    UINT32 flags = SPI_XFER_BEGIN;

    if(dataLen == 0)
	flags |= SPI_XFER_END;

    stat = spi_xfer(spi, cmdLen * 8, cmd, NULL, flags);
    if(stat == OK)
	if(dataLen != 0)
	    stat = spi_xfer(spi, dataLen * 8, dataOut, dataIn, SPI_XFER_END);

    return stat;
    }

/*******************************************************************************
* spiFlashStatusGet - return SPI Flash status
*
* This routine reads the status register.
*
* RETURNS: OK or ERROR if there is an error.
*/

LOCAL INT32 spiFlashStatusGet
    (
    struct spi_slave 	*spi
    )
    {
    UINT8 cmd = CMD_READ_STATUS;
    UINT8 buff;

    if(spiFlashReadWrite(spi, &cmd, 1, NULL, &buff, 1) == ERROR)
	return ERROR;

    return (INT32)buff;
    }


/*******************************************************************************
* spiFlashWaitReady - wait for wip to clear
*
* Wait a fixed amount of time for WIP to be cleared
*
* RETURNS: OK or ERROR if there is an error.
*/

LOCAL STATUS spiFlashWaitReady
    (
    struct spi_slave 	*spi,
    UINT32		timeout
    )
    {
    UINT32   i;
    INT32    status;

    for(i = 0 ; i < timeout; i++)
	{
	status = spiFlashStatusGet(spi);
	if((status & STATUS_WIP) == 0)
	    return OK;

	if(status == ERROR)
	    break;

	sysMsDelay(1);	
	}

    return ERROR;
    }

/*******************************************************************************
*
* sysSpiFlashProgram - SPI flash program
*
* This is the SPI flash program routine.
*
* RETURNS: OK or ERROR if there is an error.
*/
STATUS sysSpiFlashProgram
    (
    UINT32 startAddr,
    UINT32  len,
    UINT8 * buf         /* program data buf */
    )
    {
    UINT8	cmd[4];
    UINT8	cmdWriteEnable[1];
    STATUS	status = ERROR; 
    UINT32 	dataLen = 0;
    UINT32 	offset = 0;


    if(spiHandle == NULL)
	{
	spiHandle = spi_setup_slave(0,0,
				    CONFIG_SF_DEFAULT_SPEED,
				    CONFIG_SF_DEFAULT_MODE);
	if(spiHandle == NULL)
	    return status;
	}

    status = spi_claim_bus(spiHandle);

    if(status == OK)
	{
	cmdWriteEnable[0]  =  CMD_WRITE_ENABLE;
	while(offset < len)
	    {
            UINT32 curAddr = startAddr + offset;
	    dataLen = 
		min(len - offset, 
		    SPI_FLASH_PAGE_SIZE - 
                    (curAddr % SPI_FLASH_PAGE_SIZE));
 

	    status = spiFlashReadWrite(spiHandle, cmdWriteEnable,1,NULL,NULL,0);
            if(status != OK)
		break;

	    cmd[0] = CMD_PAGE_PROGRAM;
	    cmd[1] = curAddr >> 16;
	    cmd[2] = curAddr >> 8;
	    cmd[3] = curAddr & 0xff;

	    status = spiFlashReadWrite(spiHandle, cmd, 4, &buf[offset], NULL, dataLen);
            if (status != OK)
		break;

	    status = spiFlashWaitReady(spiHandle,SPI_FLASH_PROG_TIMEOUT);
            if (status != OK)
		break;

	    offset += dataLen;
	    }
	}

    return status;
    }

/*******************************************************************************
*
* sysSpiFlashSectorErase - erase flash sector
*
* Erase sector containing the specified offset.
*
* RETURNS: OK, or ERROR if offset is outside the flash memory range.
*
*
*/

STATUS sysSpiFlashSectorErase
    (
    UINT32    offset      /* offset into flash memory   */
    )
    {
    UINT8	cmd[4];
    UINT8       cmdWriteEnable[1];

    STATUS	status 		= ERROR;
    UINT32	eraseSize 	= SPI_FLASH_SECTOR_SIZE;


    if ((offset < 0) || (offset > SPI_FLASH_SIZE))
        return status;

    if(spiHandle == NULL)
	{
	spiHandle = spi_setup_slave(0,0,
				    CONFIG_SF_DEFAULT_SPEED,
				    CONFIG_SF_DEFAULT_MODE);
	if(spiHandle == NULL)
	    return status;
	}

    offset &= ~(eraseSize-1);

    status = spi_claim_bus(spiHandle);

    if(status == OK)
	{
	if (eraseSize == 4096)
		cmd[0] = CMD_ERASE_4K;
	else
		cmd[0] = CMD_ERASE_64K;

	cmd[1] = offset >> 16;
	cmd[2] = offset >> 8;
	cmd[3] = offset >> 0;

	cmdWriteEnable[0]  =  CMD_WRITE_ENABLE;
	status = spiFlashReadWrite(spiHandle, cmdWriteEnable,1,NULL,NULL,0);
	if(status == OK)
	    {
	    status = spiFlashReadWrite(spiHandle,cmd,sizeof(cmd),NULL,NULL,0);
	    if(status == OK)
		{
		status = spiFlashWaitReady(spiHandle, SPI_FLASH_PAGE_ERASE_TIMEOUT);
		}
	    }
	spi_release_bus(spiHandle);
	}
    return status;
    }

/*******************************************************************************
*
* spiFlashReadFast
*
* This is the SPI flash fast read routine.
*
* RETURNS: OK or ERROR if there is an error.
*/

LOCAL STATUS spiFlashReadFast
    (
    struct spi_slave 	*spi,
    UINT32		offset,
    size_t		len,
    void		*dataIn
    )
    {
    UINT8	cmd[5];
    STATUS	status;

    cmd[0] = CMD_READ_ARRAY_FAST;
    cmd[1] = offset >> 16;
    cmd[2] = offset >> 8;
    cmd[3] = offset >> 0;
    cmd[4] = 0x00;

    spi_claim_bus(spi);

    status = spiFlashReadWrite(spi, cmd, sizeof(cmd), NULL, dataIn, len);
    spi_release_bus(spi);
    return status;
    }

/*******************************************************************************
*
* sysFlashGet - get the contents of flash memory
*
* This routine copies the contents of flash memory into a specified
* string.
*
* RETURNS: OK, or ERROR if access is outside the flash memory range.
*
* SEE ALSO: sysFlashSet()
*
* INTERNAL
* If multiple tasks are calling sysFlashSet() and sysFlashGet(),
* they should use a semaphore to ensure mutually exclusive access.
*/

STATUS sysFlashGet
    (
    char *  string,     /* where to copy flash memory      */
    int     strLen,     /* maximum number of bytes to copy */
    int     offset      /* byte offset into flash memory   */
    )
    {
    STATUS  status = ERROR; 

    if ((offset < 0) || (strLen < 0) || ((offset + strLen) > SPI_FLASH_SIZE))
        return (status);

    if(spiHandle == NULL)
	{
	spiHandle = spi_setup_slave(0,0,
				    CONFIG_SF_DEFAULT_SPEED,
				    CONFIG_SF_DEFAULT_MODE);
	if(spiHandle == NULL)
	    return status;
	}

    status = spiFlashReadFast(spiHandle,offset,strLen,string);
   
    return status;
    }

/*******************************************************************************
*
* sysFlashSet - write to flash memory
*
* This routine copies a specified string into flash memory.
*
* If the specified string must be overlaid on the contents of flash memory,
* define FLASH_OVERLAY in config.h.
*
* RETURNS: OK, or ERROR if the write fails or the input parameters are
* out of range.
*
* SEE ALSO: sysFlashGet()
*
* INTERNAL
* If multiple tasks are calling sysFlashSet() and sysFlashGet(),
* they should use a semaphore to ensure mutually exclusive access to flash
* memory.
*/

STATUS sysFlashSet
    (
    char *  string,     /* string to be copied into flash memory */
    int     byteLen,    /* maximum number of bytes to copy       */
    int     offset      /* byte offset into flash memory         */
    )
    {
    char 	*pBuf;
    STATUS	status;

#ifdef FLASH_OVERLAY
    UINT32      sectorAddress;
#endif

    /* limited to one sector */

    if ((offset < 0) || (byteLen < 0) ||
        (((offset % SPI_FLASH_SECTOR_SIZE) + byteLen) > SPI_FLASH_SECTOR_SIZE))
        return ERROR;

    if(spiHandle == NULL)
	{
	spiHandle = spi_setup_slave(0,0,
				    CONFIG_SF_DEFAULT_SPEED,
				    CONFIG_SF_DEFAULT_MODE);
	if(spiHandle == NULL)
	    return ERROR;
	}


    pBuf = memalign (4, SPI_FLASH_SECTOR_SIZE);
    if(pBuf == NULL)
	return ERROR;

    status = spiFlashReadFast(spiHandle,offset,byteLen,pBuf);
    if(status != OK)
	goto exit;

    /* see if contents are actually changing */
    if (bcmp (pBuf, string, byteLen) == 0)
        {
	status = OK;
	goto exit;
        }


#ifdef FLASH_OVERLAY
    /* first save the current sector data */
    sectorAddress = offset & ~(SPI_FLASH_SECTOR_SIZE-1);
    status = spiFlashReadFast(spiHandle,sectorAddress,SPI_FLASH_SECTOR_SIZE,pBuf);
    if(status != OK)
	goto exit;

    bcopy( string, pBuf +(offset % SPI_FLASH_SECTOR_SIZE), byteLen);
#else
    bcopy( string, pBuf, byteLen);
#endif /* FLASH_OVERLAY */

    /* erase sector */
    status = sysSpiFlashSectorErase(offset);
    if(status != OK)
	goto exit;

    /* program device */

#ifdef FLASH_OVERLAY
    status = sysSpiFlashProgram(sectorAddress, SPI_FLASH_SECTOR_SIZE,pBuf);
#else
    status = sysSpiFlashProgram(offset,byteLen,pBuf);
#endif

exit:
    free(pBuf);
    return status;
    }


#ifdef INCLUDE_SHOW_ROUTINES

#include <stdio.h>

/*******************************************************************************
*
* spiFlashShow -  SPI flash device show routine
*
* This routine shows SPI flash information.
*
* RETURNS: OK or ERROR if there is an error.
*/

void spiFlashShow(void)
    {
    UINT8	cmd;
    int ret, i, shift;
    char idcode[IDCODE_LEN], *idp;

    spiHandle = 
	  spi_setup_slave(0,0,CONFIG_SF_DEFAULT_SPEED,CONFIG_SF_DEFAULT_MODE);

    if (!spiHandle) 
	{
	printf("SF: Failed to set up slave\n");
	return;
	}

    ret = spi_claim_bus(spiHandle);
    if (ret) 
	{
	printf("SF: Failed to claim SPI bus: %d\n", ret);
	return;
	}


    /* Read the ID codes */
    cmd = CMD_READ_ID;
    ret = spiFlashReadWrite(spiHandle,&cmd,1,NULL,idcode, sizeof(idcode));
    if (ret) 
	{
	printf("SF: Failed to Read the ID codes: %d\n", ret);
	return;
	}

    /* count the number of continuation bytes */
    for (shift = 0, idp = idcode; 
	shift < IDCODE_CONT_LEN && *idp == 0x7f;
	++shift, ++idp)
	continue;

    printf("SF: idcode: ");
    for(i= 0; i < IDCODE_LEN; i++)
	printf(" 0x%02x",idcode[i]);

    printf("\n");    
}
#endif

#ifdef TEST_SPI_FLASH
#include <stdio.h>
void testwrite(void)
{
	char *buff, *pbuf;
	int	i;
	int 	j;

	buff = malloc(256*256);
	pbuf = buff;

	for(i = 0 ; i < 256; i++)
	{
		for(j = 0 ; j < 256; j++)
			*pbuf++ = j;
	}
	
	sysSpiFlashProgram(0x00d00000, 256*256,buff);
	free(buff);	
}

void testread
    (
    unsigned int offset, 
    size_t len
    )
    {
    char *buff, *pbuf;
    unsigned int i;

    if(spiHandle == NULL)
	{
	spiHandle = spi_setup_slave(0,0,
				    CONFIG_SF_DEFAULT_SPEED,
				    CONFIG_SF_DEFAULT_MODE);
	}
    buff = malloc(len);
    pbuf = buff;

    spiFlashReadFast(spiHandle, offset, len, buff);

    for(i = 0 ; i < len; i++)
	{
	if((i % 16) == 0)
		printf("\n");
	printf("%02x ", *pbuf++);
	} 

    printf("\n");
    free(buff);
    }
		
void testEraseAndWrite(void)
{
	char *buff, *pbuf;
	int	i;
	int 	j;

	buff = malloc(256*256);
	pbuf = buff;

	for(i = 0 ; i < 256; i++)
	{
		for(j = 0 ; j < 256; j++)
			*pbuf++ = j;
	}
	i = sysSpiFlashSectorErase(0x00d00000);

        printf("erase result: %d\n", i);

	i = sysSpiFlashProgram(0x00d00000, 256*256,buff);

        printf("program result: %d\n", i);

	free(buff);	
}
#endif /* TEST_SPI_FLASH */	

