/* iProcQspi.c - qspi serial flash driver */

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

#include <vxWorks.h>
#include <config.h>
#include <stdlib.h>
#include <tickLib.h>
#include <taskLib.h>
#include <sysLib.h>
#include "iProcQspi.h"


typedef UINT32 u32;
typedef UINT8  u8;
typedef unsigned int uint;

/* MSPI registers */
struct bcm_mspi_hw {
    u32             spcr0_lsb;               /* 0x000 */
    u32             spcr0_msb;               /* 0x004 */
    u32             spcr1_lsb;               /* 0x008 */
    u32             spcr1_msb;               /* 0x00c */
    u32             newqp;                   /* 0x010 */
    u32             endqp;                   /* 0x014 */
    u32             spcr2;                   /* 0x018 */
    u32             reserved0;               /* 0x01c */
    u32             mspi_status;             /* 0x020 */
    u32             cptqp;                   /* 0x024 */
    u32             reserved1[6];            /* 0x028 */
    u32             txram[NUM_TXRAM];        /* 0x040 */
    u32             rxram[NUM_RXRAM];        /* 0x0c0 */
    u32             cdram[NUM_CDRAM];        /* 0x140 */
    u32             write_lock;              /* 0x180 */
    u32             disable_flush_gen;       /* 0x184 */
};

/* BSPI registers */
struct bcm_bspi_hw {
    u32             revision_id;             /* 0x000 */
    u32             scratch;                 /* 0x004 */
    u32             mast_n_boot_ctrl;        /* 0x008 */
    u32             busy_status;             /* 0x00c */
    u32             intr_status;             /* 0x010 */
    u32             b0_status;               /* 0x014 */
    u32             b0_ctrl;                 /* 0x018 */
    u32             b1_status;               /* 0x01c */
    u32             b1_ctrl;                 /* 0x020 */
    u32             strap_override_ctrl;     /* 0x024 */
    u32             flex_mode_enable;        /* 0x028 */
    u32             bits_per_cycle;          /* 0x02C */
    u32             bits_per_phase;          /* 0x030 */
    u32             cmd_and_mode_byte;       /* 0x034 */
    u32             flash_upper_addr_byte;   /* 0x038 */
    u32             xor_value;               /* 0x03C */
    u32             xor_enable;              /* 0x040 */
    u32             pio_mode_enable;         /* 0x044 */
    u32             pio_iodir;               /* 0x048 */
    u32             pio_data;                /* 0x04C */
};

/* RAF registers */
struct bcm_bspi_raf {
    u32             start_address;           /* 0x00 */
    u32             num_words;               /* 0x04 */
    u32             ctrl;                    /* 0x08 */
    u32             fullness;                /* 0x0C */
    u32             watermark;               /* 0x10 */
    u32             status;                  /* 0x14 */
    u32             read_data;               /* 0x18 */
    u32             word_cnt;                /* 0x1C */
    u32             curr_addr;               /* 0x20 */
};

/* CRU register */
struct bcm_cru {
    u32             cru_control;             /* 0x00 */
};

/* State */
enum bcm_qspi_state {
    QSPI_STATE_DISABLED,
    QSPI_STATE_MSPI,
    QSPI_STATE_BSPI
};


/* QSPI private data */
struct bcmspi_priv {

    /* Slave entry */
    struct spi_slave                    slave;
   
    /* Specified SPI parameters */
    unsigned int                        max_hz;
    unsigned int                        spi_mode;
    
    /* State */
    enum bcm_qspi_state                 state;
    u8                                  bspi_op;
    u32                                 bspi_addr;
    int                                 mspi_16bit;
    int                                 mode_4byte;
    
    /* Registers */
    volatile struct bcm_mspi_hw         *mspi_hw;
    volatile struct bcm_bspi_hw         *bspi_hw;
    volatile struct bcm_bspi_raf        *bspi_hw_raf;
    volatile struct bcm_cru             *cru_hw;
};

UINT32 configIprocBspiDataLanes = CONFIG_IPROC_BSPI_DATA_LANES;
UINT32 configIprocBspiAddrLanes = CONFIG_IPROC_BSPI_ADDR_LANES;
		
void sysUsDelay(INT32 count);


struct bcmspi_priv *to_qspi_slave(struct spi_slave *slave)
{
	char *p = (char *)slave;
        int  offset = offsetof(struct bcmspi_priv, slave);
        return (struct bcmspi_priv *)(p - offset);
}

int
spi_cs_is_valid(unsigned int bus, unsigned int cs)
{
    if (bus == CONFIG_IPROC_QSPI_BUS && cs == CONFIG_IPROC_QSPI_CS) {
        return 1;
    }
    return 0;
}

struct spi_slave *
spi_setup_slave(unsigned int bus, unsigned int cs, 
                unsigned int max_hz, unsigned int mode)
{
    struct bcmspi_priv *priv;
    unsigned int spbr;

    if (!spi_cs_is_valid(bus, cs)) {
        return NULL;
    }
    
    priv = malloc(sizeof(*priv));
    if (priv == NULL) {
        return NULL;
    }
    
    priv->slave.bus = bus;
    priv->slave.cs = cs;
    priv->max_hz = max_hz;
    priv->spi_mode = mode;
    priv->state = QSPI_STATE_DISABLED;
    priv->mode_4byte = 0;
    priv->bspi_hw = (volatile struct bcm_bspi_hw *)(QSPI_REG_BASE + 0x000);
    priv->bspi_hw_raf = (volatile struct bcm_bspi_raf *)(QSPI_REG_BASE + 0x100);
    priv->mspi_hw = (volatile struct bcm_mspi_hw *)(QSPI_REG_BASE + 0x200);
    priv->cru_hw = (volatile struct bcm_cru *)CRU_CONTROL_REG;
    
    /* BSPI: clock configuration */
    priv->cru_hw->cru_control &= ~0x00000006;
    (void)priv->cru_hw->cru_control; /* Need to read back */
    if (priv->max_hz >= 62500000) {
        priv->cru_hw->cru_control |= 0x00000006;
    } else if (priv->max_hz >= 50000000) {
        priv->cru_hw->cru_control |= 0x00000002;
    } else if (priv->max_hz >= 31250000) {
        priv->cru_hw->cru_control |= 0x00000004;
    }
    (void)priv->cru_hw->cru_control; /* Need to read back */
    
    /* BSPI: configure for dual/quad mode */
    if (configIprocBspiDataLanes != 1 || 
    		configIprocBspiAddrLanes != 1) 
	{
    
        /* Disable flex mode first */
        priv->bspi_hw->flex_mode_enable = 0;
            
        /* Data / Address lanes */
        priv->bspi_hw->bits_per_cycle = 0;
        if (configIprocBspiDataLanes == 4) {
            priv->bspi_hw->bits_per_cycle |= 
                2 << BSPI_BITS_PER_CYCLE_DATA_SHIFT;
        } else {
            priv->bspi_hw->bits_per_cycle |= 
                (configIprocBspiDataLanes - 1) 
                    << BSPI_BITS_PER_CYCLE_DATA_SHIFT;
        }
        if (configIprocBspiAddrLanes == 4) {
            priv->bspi_hw->bits_per_cycle |= 
                2 << BSPI_BITS_PER_CYCLE_ADDR_SHIFT;
        } else {
            priv->bspi_hw->bits_per_cycle |= 
                (configIprocBspiAddrLanes - 1) 
                    << BSPI_BITS_PER_CYCLE_ADDR_SHIFT;
        }
        
        /* Dummy cycles */
        priv->bspi_hw->bits_per_phase &= ~0xFF;
        priv->bspi_hw->bits_per_phase |= CONFIG_IPROC_BSPI_READ_DUMMY_CYCLES;
        
        /* Command byte for BSPI */
        priv->bspi_hw->cmd_and_mode_byte = CONFIG_IPROC_BSPI_READ_CMD;
        
        /* Enable flex mode to take effect */
        priv->bspi_hw->flex_mode_enable = 1;
    }
    
    /* MSPI: Basic hardware initialization */
    priv->mspi_hw->spcr1_lsb = 0;
    priv->mspi_hw->spcr1_msb = 0;
    priv->mspi_hw->newqp = 0;
    priv->mspi_hw->endqp = 0;
    priv->mspi_hw->spcr2 = 0;
    
    {
    extern int32_t iproc_get_axi_clk(uint32_t refclk);

    UINT32 apb_clk = iproc_get_axi_clk(0) / 4;

    /* MSPI: SCK configuration */
    spbr = (apb_clk - 1) / (2 * priv->max_hz) + 1;
    priv->mspi_hw->spcr0_lsb = max(min(spbr, SPBR_MAX), SPBR_MIN);
    }
    /* MSPI: Mode configuration (8 bits by default) */
    priv->mspi_16bit = 0;
    priv->mspi_hw->spcr0_msb = 
        0x80 |                      /* Master */
        (8 << 2) |                  /* 8 bits per word */
        (priv->spi_mode & 3)        /* mode: CPOL / CPHA */
        ;
    
    return &priv->slave;
}

void 
spi_free_slave(struct spi_slave *slave)
{
    struct bcmspi_priv *priv;
    
    if (slave == NULL) {
        return;
    }
    priv = to_qspi_slave(slave);

    free(priv);
}

static void 
bspi_flush_prefetch_buffers(struct bcmspi_priv *priv)
{
    priv->bspi_hw->b0_ctrl = 0;
    priv->bspi_hw->b1_ctrl = 0;
    priv->bspi_hw->b0_ctrl = 1;
    priv->bspi_hw->b1_ctrl = 1;
}

static int 
bcmspi_enable_bspi(struct bcmspi_priv *priv)
{
    if (priv->state == QSPI_STATE_BSPI) {
        return 0;
    }

    /* Disable write lock */
    priv->mspi_hw->write_lock = 0;
    
    /* Flush prefetch buffers */
    bspi_flush_prefetch_buffers(priv);
    
    /* Switch to BSPI */
    priv->bspi_hw->mast_n_boot_ctrl = 0;
    
    /* Update state */
    priv->state = QSPI_STATE_BSPI;
    
    return 0;
}

static int 
bcmspi_disable_bspi(struct bcmspi_priv *priv)
{
    if (priv->state == QSPI_STATE_MSPI) 
	{
        return 0;
	}
    
    /* Switch to MSPI if not yet */
    if ((priv->bspi_hw->mast_n_boot_ctrl & 1) == 0) {
	unsigned int count;
	for(count = 0; count < QSPI_WAIT_TIMEOUT ; count++) {
            if ((priv->bspi_hw->busy_status & 1) == 0) {
                priv->bspi_hw->mast_n_boot_ctrl = 1;
                sysUsDelay(1);
                break;
            }
            sysUsDelay(1);
        }
        if (priv->bspi_hw->mast_n_boot_ctrl != 1) {
            return -1;
        }
    }
    
    /* Update state */
    priv->state = QSPI_STATE_MSPI;
    
    return 0;
}

int
spi_claim_bus(struct spi_slave *slave)
{
    struct bcmspi_priv *priv;
    
    if (slave == NULL) {
        return -1;
    }
    priv = to_qspi_slave(slave);
    if (priv->state != QSPI_STATE_DISABLED) {
        /* Already enabled */
        return  0;
    }

    /* Switch to BSPI by default */
    if (bcmspi_enable_bspi(priv) != 0) {
        return -1;
    }
    
    return 0;
}

void 
spi_release_bus(struct spi_slave *slave)
{
    struct bcmspi_priv *priv;
    
    if (slave == NULL) {
        return;
    }
    priv = to_qspi_slave(slave);
    if (priv->state == QSPI_STATE_DISABLED) {
        return;
    }
    
    /* Make sure no operation is in progress */
    priv->mspi_hw->spcr2 = 0;
    sysUsDelay(1);
    
    /* Switch to BSPI */
    bcmspi_enable_bspi(priv);
    
    /* Update state */
    priv->state = QSPI_STATE_DISABLED;
}

static void
bspi_set_4byte_mode(struct bcmspi_priv *priv, int enable)
{
    
    /* Disable flex mode first */
    priv->bspi_hw->flex_mode_enable = 0;
        
    if (enable) {
        
        /* Enable 32-bit address */
        priv->bspi_hw->bits_per_phase |= BSPI_BITS_PER_PHASE_ADDR_MARK;
        
    } else {

        /* Disable 32-bit address */
        priv->bspi_hw->bits_per_phase &= ~BSPI_BITS_PER_PHASE_ADDR_MARK;
        
        /* Clear upper address byte */
        priv->bspi_hw->flash_upper_addr_byte = 0;
    }
    
    /* Enable flex mode to take effect */
    priv->bspi_hw->flex_mode_enable = 1;
        
    /* Record current mode */
    priv->mode_4byte = enable;
}

#define DWORD_ALIGNED(a) (!(((unsigned long)(a)) & 3))

static void
bspi_read_via_raf(struct bcmspi_priv *priv, u8 *rx, uint bytes)
{
    u32 status;
    uint words;
    int aligned;

    /* Flush data from the previous session (unlikely) */
    for(;;) {
        status = priv->bspi_hw_raf->status;
        if (!(status & BSPI_RAF_STATUS_FIFO_EMPTY_MASK)) {
            (void)priv->bspi_hw_raf->read_data;
            continue;
        }
        if (!(status & BSPI_RAF_STATUS_SESSION_BUSY_MASK)) {
            break;
        }
    }

    /* Transfer is in words */
    words = (bytes + 3) / 4;
    
    /* Setup hardware */
    if (priv->mode_4byte) {
        u32 val = priv->bspi_addr & 0xFF000000;
        if (val != priv->bspi_hw->flash_upper_addr_byte) {
            priv->bspi_hw->flash_upper_addr_byte = val;
            bspi_flush_prefetch_buffers(priv);
        }
    }
    priv->bspi_hw_raf->start_address = priv->bspi_addr & 0x00FFFFFF;
    priv->bspi_hw_raf->num_words = words;
    priv->bspi_hw_raf->watermark = 0;
    
    /* Kick off */
    priv->bspi_hw_raf->ctrl = BSPI_RAF_CONTROL_START_MASK;
    
    /* Reading the data */
    aligned = DWORD_ALIGNED(rx);
    while(bytes) {
        status = priv->bspi_hw_raf->status;
        if (!(status & BSPI_RAF_STATUS_FIFO_EMPTY_MASK)) {
            
            u32 data = le32_to_cpu(priv->bspi_hw_raf->read_data);
            
            /* Check if we can use the whole word */
            if (aligned && bytes >=4) {
            
                /* RAF is LE only, convert data to host endianness */
                *(u32 *)rx = le32_to_cpu(data);
                rx += 4;
                bytes -= 4;
            } else {
                
                uint chunk = min(bytes, 4);
                
                /* Read out bytes one by one */
                while(chunk) {
                    *rx++ = (u8)data;
                    data >>= 8;
                    chunk--;
                    bytes--;
                }
            }
            
            continue;
        }
        if (!(status & BSPI_RAF_STATUS_SESSION_BUSY_MASK)) {
            /* FIFO is empty and the session is done */
            break;
        }
    }
}

static int
bspi_emulate_flash_read(struct bcmspi_priv *priv, 
                        const u8 *tx, u8 *rx, uint bytes, unsigned long flags)
{
    int idx = priv->mode_4byte? 2 : 1;
    
    /* Check the address */
    if (flags & SPI_XFER_BEGIN) {
        
        u32 addr;
        
        /* Drop if the first transfer doesn't contain full address */
        if (bytes < idx + 3 + 1) {
            return -1;
        }
        
        /* Flash offset - lower 24 bits */
        addr = (tx[idx] << 16) | (tx[idx+1] << 8) | tx[idx+2];
        
        /* Flash offset - upper 8 bits */
        if (priv->mode_4byte) {
            addr |= tx[1] << 24;
        }

        /* Remaining length for data (excluding one dummy cycle) */
        bytes -= idx + 3 + 1;
        if (rx) {
            rx += idx + 3 + 1;
        }

        /* non-aligned transfers are handled by MSPI */
        if (!DWORD_ALIGNED(addr)) {
            return -1;
        }
        
        /* Switch to BSPI */
        if (bcmspi_enable_bspi(priv) != 0) {
            return -1;
        }
        
        /* Record BSPI status */
        priv->bspi_op = OPCODE_FAST_READ;
        priv->bspi_addr = addr;
    }
    
    /* Transfer data if any */
    while (bytes && rx) {
    
        /* Special handing since RAF cannot go across 16MB boundary */
        uint trans = bytes;
        
        /* Divide into multiple transfers if it goes across the boundary */
        if (priv->mode_4byte && 
           (priv->bspi_addr >> 24) != ((priv->bspi_addr + bytes) >> 24)) {
           
            /* Limit this transfer to not go beyond 16MB boundary */
            trans = 0x1000000 - (priv->bspi_addr & 0xFFFFFF);
        }
        bspi_read_via_raf(priv, rx, trans);
        priv->bspi_addr += trans;
        rx += trans;
        bytes -= trans;
    }
    
    /* Flush prefetch buffers at the end */
    if (flags & SPI_XFER_END) {
        bspi_flush_prefetch_buffers(priv);
    }
    
    return 0;
}

static int
bspi_emulate_flash_rdsr(struct bcmspi_priv *priv, 
                        const u8 *tx, u8 *rx, uint bytes, unsigned long flags)
{
    /* Only emulate the status register if it was a BSPI read */
    if (priv->state != QSPI_STATE_BSPI) {
        return -1;
    }
    
    /* Handle for the first transfer */
    if (flags & SPI_XFER_BEGIN) {

        /* Record status */
        priv->bspi_op = OPCODE_RDSR;
        
        /* Skip the first byte: command */
        bytes--;
        rx++;
    }
    
    /* Fill the rx data with 0 */
    while (bytes) {
        *rx++ = 0x00;
    }
    
    return 0;
}

static int
mspi_xfer(struct bcmspi_priv *priv, uint bytes, const u8 *tx, u8 *rx, int end)
{
    if (bytes & 1) {
        /* Use 8-bit queue for odd-bytes transfer */
        if (priv->mspi_16bit) {
            priv->mspi_hw->spcr0_msb |= MSPI_SPCR0_MSB_BITS_8;
            priv->mspi_16bit = 0;
        }
    } else {
        /* Use 16-bit queue for even-bytes transfer */
        if (!priv->mspi_16bit) {
            priv->mspi_hw->spcr0_msb &= ~MSPI_SPCR0_MSB_BITS_8;
            priv->mspi_16bit = 1;
        }
    }
    
    while(bytes) {
    
        uint chunk;
        uint queues;
        uint i;
        
        /* Separate code for 16bit and 8bit transfers for performance */
        if (priv->mspi_16bit) {
        
            /* Determine how many bytes to process this time */
            chunk = min(bytes, NUM_CDRAM * (priv->mspi_16bit? 2 : 1));
            queues = (chunk - 1) / 2 + 1;
            bytes -= chunk;
            
            /* Fill CDRAMs */
            for(i=0; i<queues; i++) {
                priv->mspi_hw->cdram[i] = 0xc2;
            }
            
            /* Fill TXRAMs */
            for(i=0; i<chunk; i++) {
                priv->mspi_hw->txram[i] = tx? tx[i] : 0xff;
            }
            
        } else {
        
            /* Determine how many bytes to process this time */
            chunk = min(bytes, NUM_CDRAM * (priv->mspi_16bit? 2 : 1));
            queues = chunk;
            bytes -= chunk;
        
            /* Fill CDRAMs and TXRAMS */
            for(i=0; i<chunk; i++) {
                priv->mspi_hw->cdram[i] = 0x82;
                priv->mspi_hw->txram[i << 1] = tx? tx[i] : 0xff;
            }
        }
        
        /* Setup queue pointers */
        priv->mspi_hw->newqp = 0;
        priv->mspi_hw->endqp = queues - 1;

        /* Deassert CS if requested and it's the last transfer */
        if (bytes == 0 && end ) {
            priv->mspi_hw->cdram[queues - 1] &= ~0x80;
        }
        
        /* Kick off */
        priv->mspi_hw->mspi_status = 0;
        priv->mspi_hw->spcr2 = 0xc0;    /* cont | spe */
        
        /* Wait for completion */
	{
	unsigned int i;
	for(i = 0 ; i < QSPI_WAIT_TIMEOUT; i++)
	    {
            if (priv->mspi_hw->mspi_status & 1) 
		{
                break;
            	}
	    sysUsDelay(1);
            }

        if ((priv->mspi_hw->mspi_status & 1) == 0) 
	    {
            return -1;
            }
	}
        
        /* Read data out */
        if (rx) {
            if (priv->mspi_16bit) {
                for(i=0; i<chunk; i++) {
                    rx[i] = priv->mspi_hw->rxram[i] & 0xff;
                }
            } else {
                for(i=0; i<chunk; i++) {
                    rx[i] = priv->mspi_hw->rxram[(i << 1) + 1] & 0xff;
                }
            }
        }
        
        /* Advance pointers */
        if (tx)
            tx += chunk;
        if (rx) 
            rx += chunk;
    }
    
    return 0;
}

int
spi_xfer(struct spi_slave *slave, unsigned int bitlen, const void *dout,
         void *din, unsigned long flags)
{
    struct bcmspi_priv *priv;
    const u8 *tx = dout;
    u8 *rx = din;
    uint bytes = bitlen / 8;
    int ret = 0;
    
    if (slave == NULL) {
        return -1;
    }
    priv = to_qspi_slave(slave);
    if (priv->state == QSPI_STATE_DISABLED) {
        return -1;
    }
    
    /* we can only do 8 bit transfers */
    if (bitlen % 8) {
        return -1;
    }
    
    /* Check if we can make use of BSPI */
    if (flags & SPI_XFER_BEGIN) {
    
        /* We only use BSPI if the first transfer contains command */
        if (bytes) {
            switch(tx[0]) {
            
            case OPCODE_FAST_READ:
                if (bspi_emulate_flash_read(priv, tx, rx, bytes, flags) == 0) {
                    return 0;
                }
                break;
                
            case OPCODE_RDSR:
                if (bspi_emulate_flash_rdsr(priv, tx, rx, bytes, flags) == 0) {
                    return 0;
                }
                break;
                
            case OPCODE_EN4B:
                bspi_set_4byte_mode(priv, 1);
                break;
                
            case OPCODE_EX4B:
                bspi_set_4byte_mode(priv, 0);
                break;
                
            case OPCODE_BRWR:
                bspi_set_4byte_mode(priv, tx[1]? 1 : 0);
                break;
                
            default:
                break;
            }
        }

    } else if (priv->state == QSPI_STATE_BSPI) {
        /* It's a following BSPI operation */
        switch(priv->bspi_op) {
        
        case OPCODE_FAST_READ:
            if (bspi_emulate_flash_read(priv, tx, rx, bytes, flags) == 0) {
                return 0;
            }
            break;
            
        case OPCODE_RDSR:
            if (bspi_emulate_flash_rdsr(priv, tx, rx, bytes, flags) == 0) {
                return 0;
            }
            break;
            
        default:
            break;
        }
        return -1;
    }
    
    /* MSPI: Enable write lock at the beginning */
    if (flags & SPI_XFER_BEGIN) {
    
        /* Switch to MSPI if not yet */
        if (bcmspi_disable_bspi(priv) != 0) {
            return -1;
        }
        
        priv->mspi_hw->write_lock = 1;
    }

    /* MSPI: Transfer it */
    if (bytes) {
        ret = mspi_xfer(priv, bytes, tx, rx, (flags & SPI_XFER_END));
    }

    /* MSPI: Disable write lock if it's done */
    if (flags & SPI_XFER_END) {
        priv->mspi_hw->write_lock = 0;
    }

    return ret;
}

