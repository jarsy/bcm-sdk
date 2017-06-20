/*
 * $Id: sflash.c,v 1.1 2013/12/02 08:34:59 kevinwu Exp $
 * $Copyright: (c) 2013 Broadcom Corp.
 * All Rights Reserved.$
 *
 * File:    sflash.c
 */
#include <sflash.h>
#include <kernelLib.h>
#include <config.h>

/* Private global state */
static struct sflash sflash;

#define IPROC_BSPI_BASE          0x18027000
#define IPROC_QSPI_BASE          0x1E000000

/*
 * Register and bit definitions for the BSPI control
 * registers.
 */

/* Control and Interrupt Registers */
#define BCHP_BSPI_MAST_N_BOOT_CTRL    (IPROC_BSPI_BASE + 0x8)
#define BCHP_BSPI_BUSY_STATUS         (IPROC_BSPI_BASE + 0xC)
#define BCHP_BSPI_INTR_STATUS         (IPROC_BSPI_BASE + 0x10)
#define BCHP_BSPI_B0_STATUS           (IPROC_BSPI_BASE + 0x14)
#define BCHP_BSPI_B0_CTRL             (IPROC_BSPI_BASE + 0x18)
#define BCHP_BSPI_B1_STATUS           (IPROC_BSPI_BASE + 0x1C)
#define BCHP_BSPI_B1_CTRL             (IPROC_BSPI_BASE + 0x20)
#define BCHP_FLEX_MODE_ENABLE         (IPROC_BSPI_BASE + 0x28)
#define BCHP_BITS_PER_CYCLE           (IPROC_BSPI_BASE + 0x2C)
#define BCHP_BITS_PER_PHASE           (IPROC_BSPI_BASE + 0x30)
    
/*
 * Register and bit definitions for the MSPI control
 * registers.
 */
    
/* Control and Interrupt Registers */
#define BCHP_MSPI_SPCR0_LSB           (IPROC_BSPI_BASE + 0x200)
#define BCHP_MSPI_SPCR0_MSB           (IPROC_BSPI_BASE + 0x204)
#define BCHP_MSPI_SPCR1_LSB           (IPROC_BSPI_BASE + 0x208)
#define BCHP_MSPI_SPCR1_MSB           (IPROC_BSPI_BASE + 0x20C)
#define BCHP_MSPI_NEWQP               (IPROC_BSPI_BASE + 0x210)
#define BCHP_MSPI_ENDQP               (IPROC_BSPI_BASE + 0x214)
#define BCHP_MSPI_SPCR2               (IPROC_BSPI_BASE + 0x218)
#define BCHP_MSPI_MSPI_STATUS         (IPROC_BSPI_BASE + 0x220)
#define BCHP_MSPI_CPTQP               (IPROC_BSPI_BASE + 0x224)
#define BCHP_MSPI_TXRAM00             (IPROC_BSPI_BASE + 0x240)
#define BCHP_MSPI_RXRAM00             (IPROC_BSPI_BASE + 0x2C0)
#define BCHP_MSPI_RXRAM01             (IPROC_BSPI_BASE + 0x2C4)
#define BCHP_MSPI_CDRAM00             (IPROC_BSPI_BASE + 0x340)

/* MSPI Enable */
#define BCHP_MSPI_SPCR2_spe_MASK         (1 << (6))
/* MSPI Finished flag */
#define BCHP_MSPI_MSPI_STATUS_SPIF_MASK  (1 << (0))
/* SPI is configured to be master */
#define BCHP_MSPI_SPCR0_MSB_MSTR_MASK    (1 << (7))
/* Bits per transfer */
#define BCHP_MSPI_SPCR0_MSB_BITS_MASK    (0xF << (2))
/* 8 bits per transfer */
#define BCHP_MSPI_SPCR0_MSB_8BITS        (0x8 << (2))
/* Clock polarity */
#define BCHP_MSPI_SPCR0_MSB_CPOL_MASK    (1 << (1))
/* Clock phase */
#define BCHP_MSPI_SPCR0_MSB_CPHA_MASK    (1 << (0))

/* Sflash-specific commands */
#define SPI_WREN_CMD        (0x06)
#define SPI_WRDI_CMD        (0x04)
#define SPI_RDSR_CMD        (0x05)
#define SPI_READ_CMD        (0x03)
#define SPI_SE_CMD          (0x20)
#define SPI_PP_CMD          (0x02)
#define SPI_RDFSR_CMD       (0x70)
#define SPI_RDID_CMD        (0x9F)

#define SPI_CDRAM_CONT				0x80

#define SPI_CDRAM_PCS_PCS0			0x01
#define SPI_CDRAM_PCS_PCS1			0x02
#define SPI_CDRAM_PCS_PCS2			0x00
#define SPI_CDRAM_PCS_DISABLE_ALL	(SPI_CDRAM_PCS_PCS0 | SPI_CDRAM_PCS_PCS1 | SPI_CDRAM_PCS_PCS2)

#define SPI_SYSTEM_CLK		200000000	/* 200 MHz */
#define MAX_SPI_BAUD		12500000	/* SPBR = 8, 12.5MHZ */

#define ENTER_BSPI_DELAY(n) do {\
                               volatile int count;\
                               for (count = 0; count < (n); count++);\
                            } while(0)

#define SFLASH_PAGE_SIZE     (256)

#define MSPI_MAX_PROGRAM_LEN  (12)

#define MIN(a, b) ((a) <= (b)? (a):(b))

#define SYS_REG_READ32(reg)      \
            (*(volatile UINT32 *)(reg))
#define SYS_REG_WRITE32(reg,val) \
            do { *(volatile UINT32 *)(reg) = (val); } while(0)

/* Initialize serial flash access */
static void 
bspi_flush_prefetch_buffers(void)
{
    SYS_REG_WRITE32(BCHP_BSPI_B0_CTRL, 0);
    SYS_REG_WRITE32(BCHP_BSPI_B1_CTRL, 0);
    SYS_REG_WRITE32(BCHP_BSPI_B0_CTRL, 1);
    SYS_REG_WRITE32(BCHP_BSPI_B1_CTRL, 1);
}

static STATUS
spi_transaction(UINT8 *w_buf,
                UINT8 write_len,
                UINT8 *r_buf,
                UINT8 read_len)
{      

	UINT8 i, len = write_len + read_len;;

	for (i = 0; i < len; ++i)
	{
        if (i < write_len) {
            SYS_REG_WRITE32(BCHP_MSPI_TXRAM00 + (i * 8), (UINT32)w_buf[i]);
        }
        SYS_REG_WRITE32( BCHP_MSPI_CDRAM00 + (i * 4) , SPI_CDRAM_CONT | SPI_CDRAM_PCS_PCS1);
    }

    SYS_REG_WRITE32(BCHP_MSPI_CDRAM00 + ((len - 1) * 4), SPI_CDRAM_PCS_PCS1);

    /* Set queue pointers */
    SYS_REG_WRITE32(BCHP_MSPI_NEWQP, 0);
    SYS_REG_WRITE32(BCHP_MSPI_ENDQP, len - 1);

    /* Start SPI transfer */
    SYS_REG_WRITE32(BCHP_MSPI_SPCR2, BCHP_MSPI_SPCR2_spe_MASK | BCHP_MSPI_SPCR0_MSB_MSTR_MASK);

    /* Wait for SPI to finish */
    while(!(SYS_REG_READ32(BCHP_MSPI_MSPI_STATUS) & BCHP_MSPI_MSPI_STATUS_SPIF_MASK));
    SYS_REG_WRITE32(BCHP_MSPI_MSPI_STATUS, 0);

    for (i = write_len; i < len; ++i) {
        r_buf[i-write_len] = (UINT8)SYS_REG_READ32( BCHP_MSPI_RXRAM01 + (i * 8));
    }

    return (OK);
}

static STATUS
spi_program(UINT32 base, UINT8 *buf, UINT8 len)
{   
	int i, j;

    SYS_REG_WRITE32(BCHP_MSPI_TXRAM00, SPI_PP_CMD);
    SYS_REG_WRITE32(BCHP_MSPI_TXRAM00 + 8, (base >> 16) & 0xFF);
    SYS_REG_WRITE32(BCHP_MSPI_TXRAM00 + 16, (base >> 8) & 0xFF);
    SYS_REG_WRITE32(BCHP_MSPI_TXRAM00 + 24, base & 0xFF);

    for (i = 4, j = 0; j < len; i++, j++) {
        SYS_REG_WRITE32(BCHP_MSPI_TXRAM00 + (i * 8), (UINT32)buf[j]);
    }

	for (i = 0; i < len + 4; i++) {
        SYS_REG_WRITE32(BCHP_MSPI_CDRAM00 + (i * 4) , SPI_CDRAM_CONT | SPI_CDRAM_PCS_PCS1);
    }

    SYS_REG_WRITE32(BCHP_MSPI_CDRAM00 + ((i - 1) * 4), SPI_CDRAM_PCS_PCS1);

    /* Set queue pointers */
    SYS_REG_WRITE32(BCHP_MSPI_NEWQP, 0);
    SYS_REG_WRITE32(BCHP_MSPI_ENDQP, i - 1);

    /* Start SPI transfer */
    SYS_REG_WRITE32(BCHP_MSPI_SPCR2, BCHP_MSPI_SPCR2_spe_MASK | BCHP_MSPI_SPCR0_MSB_MSTR_MASK);

    /* Wait for SPI to finish */
    while(!(SYS_REG_READ32(BCHP_MSPI_MSPI_STATUS) & BCHP_MSPI_MSPI_STATUS_SPIF_MASK));
    SYS_REG_WRITE32(BCHP_MSPI_MSPI_STATUS, 0);

    return (OK);
}

struct sflash *
sflash_init(void)
{
    sflash.blocksize = 4096;
    sflash.numblocks = 8192;
    
    sflash.size = sflash.blocksize * sflash.numblocks;

    /* Initialize MSPI */
    {
        UINT32 lval;

        /* Serial clock baud rate = System clock / (2 * SPBR) */
        lval = SPI_SYSTEM_CLK / (2 * MAX_SPI_BAUD);
        SYS_REG_WRITE32(BCHP_MSPI_SPCR0_LSB, lval);

        /* 
         * Master [bit 7]
         * Bits per transfer [bit 5-2]: 0 = 16
         * Clock Polarity [bit 1]
         * MSPI clock phase [bit 0]
         */
        lval = SYS_REG_READ32(BCHP_MSPI_SPCR0_MSB);
        lval &= ~(BCHP_MSPI_SPCR0_MSB_CPOL_MASK | BCHP_MSPI_SPCR0_MSB_CPHA_MASK |
                  BCHP_MSPI_SPCR0_MSB_BITS_MASK);
        lval |= (BCHP_MSPI_SPCR0_MSB_MSTR_MASK | BCHP_MSPI_SPCR0_MSB_CPOL_MASK |
                 BCHP_MSPI_SPCR0_MSB_CPHA_MASK | BCHP_MSPI_SPCR0_MSB_8BITS);
        SYS_REG_WRITE32(BCHP_MSPI_SPCR0_MSB, lval );
    }

    return &sflash;
}

/* Read len bytes starting at offset into buf. Returns number of bytes read. */
int
sflash_read(UINT32 offset, UINT32 len, unsigned char *buf)
{
    UINT32 local_base = offset | IPROC_QSPI_BASE;
    UINT8* rx_ptr = (UINT8*)buf;
    UINT32 rx_bytes_left = (UINT32)len;

	while (rx_bytes_left) {
		*rx_ptr = *(volatile unsigned char *)(local_base);
		
        /* Update counters and data pointers for the next page. */
        local_base++;
        rx_ptr++;
        rx_bytes_left--;
	}

	return len;
}

/* Write len bytes starting at offset into buf. Returns number of bytes
 * written.
 */
int
sflash_write(UINT32 offset, UINT32 len, const unsigned char *buf)
{
    UINT32 local_base = offset;
    UINT8* tx_ptr = (UINT8*) buf;
    UINT32 tx_bytes_left = (UINT32) len;
    UINT32 tx_bytes, chunk;
    UINT8 cmd[16], rx_data;
    STATUS rv = OK;

    while(SYS_REG_READ32(BCHP_BSPI_BUSY_STATUS));
    SYS_REG_WRITE32(BCHP_BSPI_MAST_N_BOOT_CTRL,1);

    /* Perform page program operations. */
    while (tx_bytes_left) {
        tx_bytes = SFLASH_PAGE_SIZE - (((UINT32)local_base) & (SFLASH_PAGE_SIZE - 1));
        tx_bytes = MIN(tx_bytes, tx_bytes_left);
        tx_bytes_left -= tx_bytes;
        while (tx_bytes) {
            chunk = MIN(tx_bytes, MSPI_MAX_PROGRAM_LEN);

            cmd[0] = SPI_WREN_CMD;
            if (spi_transaction(cmd,1,NULL,0) != OK) {
                 rv = ERROR;
                 goto out;
            }

            if (spi_program(local_base, tx_ptr, (UINT8)chunk) != OK) {
                goto out;
            }

            do {
                cmd[0] = SPI_RDSR_CMD;
                if (spi_transaction(cmd,1,&rx_data,1) != OK) {
                    rv = ERROR;
                    goto out;
                }
            /* Bit 0 = Write-in-Progress */
            } while(rx_data & 0x01);
            
            do {
                cmd[0] = SPI_RDFSR_CMD;
                if (spi_transaction(cmd,1,&rx_data,1) != OK) {
                    rv = ERROR;
                    goto out;
                }
            }while((rx_data & 0x80) == 0x0);

            local_base += chunk;
            tx_ptr += chunk;
            tx_bytes -= chunk;
        }
    }
out:
    cmd[0] = SPI_WRDI_CMD;
    spi_transaction(cmd,1,NULL,0);
    bspi_flush_prefetch_buffers();
    SYS_REG_WRITE32(BCHP_BSPI_MAST_N_BOOT_CTRL,0);
    ENTER_BSPI_DELAY(10);

    if (rv == OK) {
        return (len - tx_bytes_left);
    } else {
        return 0;
    }
}

/* Erase a region. Returns number of bytes scheduled for erasure.
 */
int
sflash_erase(UINT32 offset)
{
    UINT32 local_base = offset;
    UINT8 cmd[4];
    UINT8 data;
    STATUS rv = OK;

    while(SYS_REG_READ32(BCHP_BSPI_BUSY_STATUS));
    SYS_REG_WRITE32(BCHP_BSPI_MAST_N_BOOT_CTRL,1);

    cmd[0] = SPI_WREN_CMD;

    if (spi_transaction(cmd,1,NULL,0) != OK) {
        rv = ERROR;
        goto out;
    }

    cmd[0] = SPI_SE_CMD;
    cmd[1] = (UINT8)(local_base >> 16);
    cmd[2] = (UINT8)(local_base >> 8);
    cmd[3] = (UINT8)local_base;

    if (spi_transaction(cmd,4,NULL,0) != OK) {
        rv = ERROR;
        goto out;
    }

    do {
        cmd[0] = SPI_RDSR_CMD;
        if (spi_transaction(cmd,1,&data,1) != OK) {
            rv = ERROR;
            goto out;
        }
    /* Bit 0 = Write-in-Progress */
    } while(data & 0x01);

    do {
        cmd[0] = SPI_RDFSR_CMD;
        if (spi_transaction(cmd,1,&data,1) != OK) {
            rv = ERROR;
            goto out;
        }
    }while((data & 0x80) == 0x0);

out:
    cmd[0] = SPI_WRDI_CMD;
    spi_transaction(cmd,1,NULL,0);
    bspi_flush_prefetch_buffers();
    SYS_REG_WRITE32(BCHP_BSPI_MAST_N_BOOT_CTRL,0);
    ENTER_BSPI_DELAY(10);

    if (rv == OK) {
        return sflash.blocksize;
    } else {
        return 0;
    }
}

STATUS sysFlashGet
    (
    char *  string,     /* where to copy flash memory      */
    int     strLen,     /* maximum number of bytes to copy */
    int     offset      /* byte offset into flash memory   */
    )
    {
    if ((offset < 0) || (strLen < 0) || ((offset + strLen) > SPI_FLASH_SIZE)) {
        return (ERROR);
    }

    sflash_read((UINT32)offset, (UINT32)strLen, (unsigned char *)string);

    return (OK);
}

STATUS sysFlashSet
    (
    char *  string,     /* string to be copied into flash memory */
    int     byteLen,    /* maximum number of bytes to copy       */
    int     offset      /* byte offset into flash memory         */
    )
    {
    int bytes;
    char 	*pBuf;
    
#ifdef FLASH_OVERLAY
    UINT32      sectorAddress;
#endif

    /* limited to one sector */

    if ((offset < 0) || (byteLen < 0) ||
        (((offset % SPI_FLASH_SECTOR_SIZE) + byteLen) > SPI_FLASH_SECTOR_SIZE)) {
        return (ERROR);
    }

    pBuf = (char *)memalign (4, SPI_FLASH_SECTOR_SIZE);
    if(pBuf == NULL) {
    	return (ERROR);
    }

    sflash_read((UINT32)offset, (UINT32)byteLen, (unsigned char *)pBuf);
    
    /* see if contents are actually changing */
    if (bcmp (pBuf, string, byteLen) == 0)
    {
        free(pBuf);
        return (OK);
    }

#ifdef FLASH_OVERLAY
    /* first save the current sector data */
    sectorAddress = offset & ~(SPI_FLASH_SECTOR_SIZE-1);
    sflash_read(sectorAddress, SPI_FLASH_SECTOR_SIZE, (unsigned char *)pBuf);
    
    bcopy(string, pBuf +(offset % SPI_FLASH_SECTOR_SIZE), byteLen);
#else
    bcopy(string, pBuf, byteLen);
#endif /* FLASH_OVERLAY */

    /* erase sector */
    if(sflash_erase(offset) != OK) {
        free(pBuf);
        return (ERROR);
    }

    /* program device */    
#ifdef FLASH_OVERLAY
    bytes = sflash_write(sectorAddress, SPI_FLASH_SECTOR_SIZE, (unsigned char *)pBuf);
#else
    bytes = sflash_write((UINT32)offset, (UINT32)byteLen, (unsigned char *)pBuf);
#endif
    free(pBuf);
    if (bytes) {
        return (OK);
    } else {
        return (ERROR);
    }
}
