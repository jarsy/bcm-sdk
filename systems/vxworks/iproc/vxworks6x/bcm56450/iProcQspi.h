/* iProcQspi.h - qspi serial flash driver */

/*
 * Copyright (C) 2013, Broadcom Corporation. All Rights Reserved.
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.int spi_flash_cmd_wait_ready(struct spi_flash *flash, unsigned long timeout)
 */

#define SPI_FLASH_PAGE_ERASE_TIMEOUT    (5000) /* milliseconds */
#define SPI_FLASH_PROG_TIMEOUT    	(2000) /* milliseconds */

#define STATUS_WIP			0x01

#define QSPI_WAIT_TIMEOUT 200

/* Chip attributes */
#define SPBR_MIN                            8U      /* For safety */
#define SPBR_MAX                            255U
#define NUM_TXRAM                           32
#define NUM_RXRAM                           32
#define NUM_CDRAM                           16

/*
 * Register fields
 */
#define MSPI_SPCR0_MSB_BITS_8               (0x00000020)
#define BSPI_RAF_CONTROL_START_MASK         (0x00000001)
#define BSPI_RAF_STATUS_SESSION_BUSY_MASK   (0x00000001)
#define BSPI_RAF_STATUS_FIFO_EMPTY_MASK     (0x00000002)
#define BSPI_BITS_PER_PHASE_ADDR_MARK       (0x00010000)
#define BSPI_BITS_PER_CYCLE_DATA_SHIFT      0
#define BSPI_BITS_PER_CYCLE_ADDR_SHIFT      16


/*
 * Flash opcode and parameters
 */
#define OPCODE_RDSR                         0x05
#define OPCODE_FAST_READ                    0x0B
#define OPCODE_EN4B                         0xB7
#define OPCODE_EX4B                         0xE9
#define OPCODE_BRWR                         0x17

#define le32_to_cpu(x)			     (x)

#define SPI_XFER_BEGIN	0x01
#define SPI_XFER_END    0x02


#define CMD_READ_ARRAY_SLOW		0x03
#define CMD_READ_ARRAY_FAST		0x0b

#define CMD_WRITE_STATUS		0x01
#define CMD_PAGE_PROGRAM		0x02
#define CMD_WRITE_DISABLE		0x04
#define CMD_READ_STATUS			0x05
#define CMD_WRITE_ENABLE		0x06
#define CMD_ERASE_4K			0x20
#define CMD_ERASE_32K			0x52
#define CMD_ERASE_64K			0xd8
#define CMD_ERASE_CHIP			0xc7


#define	SPI_CPHA	0x01			/* clock phase */
#define	SPI_CPOL	0x02			/* clock polarity */
#define	SPI_MODE_0	(0|0)			/* (original MicroWire) */
#define	SPI_MODE_1	(0|SPI_CPHA)
#define	SPI_MODE_2	(SPI_CPOL|0)
#define	SPI_MODE_3	(SPI_CPOL|SPI_CPHA)
#define CMD_READ_ID			0x9f

#define IDCODE_CONT_LEN 0
#define IDCODE_PART_LEN 5
#define IDCODE_LEN (IDCODE_CONT_LEN + IDCODE_PART_LEN)


struct spi_slave 
    {
    unsigned int	bus;
    unsigned int	cs;
    };

int spi_xfer
    (
    struct spi_slave	*slave, 
    unsigned int	bitlen, 
    const void		*dout,
    void		*din, 
    unsigned long 	flags
    );

struct spi_slave *spi_setup_slave
    (
    unsigned int 	bus, 
    unsigned int 	cs, 
    unsigned int 	max_hz, 
    unsigned int 	mode
    );

int spi_claim_bus
    (
    struct spi_slave	*slave
    );

void spi_release_bus
    (
    struct spi_slave	*slave
    );

