/*
 * flashutl.c - Flash Read/write/Erase routines
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * $Id: flashutl.c,v 1.3 Broadcom SDK $
 */

#include <typedefs.h>
#include <osl.h>

#define DECLARE_FLASHES
#include <bcmutils.h>
#include <siutils.h>
#include <hndsoc.h>
#include <flash.h>
#include <sflash.h>
#include <flashutl.h>
#include <bcmnvram.h>

#if 0
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define DPRINT(x) printf x
#else
#define DPRINT(x)
#endif /* 0 */

#define ERR2	0x30 /* Mask for err UNUSED */
#define DONE	0x80 /* Mask for done */
#define WBUFSIZE 32  /* Write Buffer size */
#define FLASH_TRIES 4000000 /* retry count */
#define CMD_ADDR ((unsigned long)0xFFFFFFFF)

/* 'which' param for block() */
#define BLOCK_BASE 0  /* Base of block */
#define BLOCK_LIM 1  /* Limit of block */

/* Use 8bit mode for better compatibility */
#define IS_8_BIT 1
#define DELAY() do { int i; for(i=0;i<1048576; i++); } while(0)
#define FLASH_ADDR(off) ((unsigned long)flashutl_base + (off))

static uint nvram_start_offset = 512*1024;

#define RETRY  100 /* retry count for probing flash device */

/* Local vars */
static si_t *sih = NULL;
static chipcregs_t *cc = NULL;

/* Global vars */
uint8              *flashutl_base = NULL;
flash_desc_t   *flashutl_desc = NULL;
flash_cmds_t  *flashutl_cmd = NULL;
uint8 flashutl_wsz = sizeof(uint16);

static void scmd(uint16 cmd, unsigned long off);
static void cmd(uint16 cmd, unsigned long off);
static void flash_reset(void);
static int flash_poll(unsigned long off, uint16 data);
static unsigned long block(unsigned long addr, int which);
static int flash_eraseblk(unsigned long off);
static int flash_write(unsigned long off, uint8 *src, uint nbytes);
static uint16 INLINE flash_readword(unsigned long addr);
static void INLINE flash_writeword(unsigned long addr, uint16 data);

int sysFlashErase(uint off, unsigned int numbytes);

/* Read the flash ID and set the globals */
int
sysFlashInit(char *flash_str)
{
    osl_t *osh;
    uint32 fltype = PFLASH;
    uint16 flash_vendid = 0;
    uint16 flash_devid = 0;
    int idx, i = 0, retry = 0;
    struct sflash *sflash;
    
    /*
     * Check for serial flash.
     */
    sih = si_kattach(SI_OSH);
    ASSERT(sih);
    
    osh = si_osh(sih);
    
    cc = (chipcregs_t *)si_setcoreidx(sih, SI_CC_IDX);
    ASSERT(cc);
    
    flashutl_base = (uint8 *)FLASH_BASE_ADDRESS_ALIAS;

    /* Use 8bit mode for better compatibility for probing flash ID to avoid endian difference */
    if (IS_8_BIT) {
        flashutl_wsz = sizeof(uint8);
    } else {
        flashutl_wsz = (R_REG(osh, &cc->pflash_config) & CC_CFG_DS) ? 
            sizeof(uint16) : sizeof(uint8);
    }

    /* Select SFLASH ? */
    fltype = R_REG(osh, &cc->capabilities) & CC_CAP_FLASH_MASK;
    if (fltype == SFLASH_ST || fltype == SFLASH_AT) {
        if (sih->ccrev == 12)
            flashutl_base = (void *)OSL_UNCACHED((uintptr)SI_FLASH2);
        sflash = sflash_init(sih, cc);
        flashutl_cmd = &sflash_cmd_t;
        flashutl_desc = &sflash_desc;
        flashutl_desc->size = sflash->size;
        if (flash_str)
            sprintf(flash_str, "SFLASH %d kB", sflash->size/1024);
        return (0);
    }
    
    ASSERT(flashutl_wsz == sizeof(uint8) || flashutl_wsz == sizeof(uint16));
    
    /*
     * Parallel flash support
     *  Some flashes have different unlock addresses, try each it turn
     */
    for (idx = 0;
        fltype == PFLASH && idx < ARRAYSIZE(flash_cmds);
        idx ++) {
        flashutl_cmd = &flash_cmds[idx];
        if (flashutl_cmd->type == OLD)
            continue;
        
        if (flashutl_cmd->read_id)
           cmd(flashutl_cmd->read_id, CMD_ADDR);
        
        retry = 0;
        flash_vendid = flash_readword(FLASH_ADDR(0));
        /* Since BCM5300x runs too fast, we need some delay in between for AMD serial flash*/
        if ((flashutl_cmd->type == AMD) && (flash_vendid != 1) ) {
            for (i=1 ; i <= RETRY ; i++) {
                DELAY();
        flash_vendid = flash_readword(FLASH_ADDR(0));
                retry++;
                if (flash_vendid == 0x1) {
                    i = RETRY;
                }
            }
        }
        flash_devid = flash_readword(FLASH_ADDR(2));

        DPRINT(("(flash type = 0x%x )flash_vendid = 0x%x, flash_devid = 0x%x, retry = %d\n", 
            flashutl_cmd->type, flash_vendid, flash_devid, retry));
        
        /* Funky AMD, uses 3 byte device ID so use first byte (4th addr) to
         * identify it is a 3-byte ID and use the next two bytes (5th & 6th addr)
         * to form a word for unique identification of format xxyy, where
         * xx = 5th addr and yy = 6th addr
         */
        if ((flash_vendid == 1) &&
              ((flash_devid == 0x227e && flashutl_wsz == sizeof(uint16)) ||
                (flash_devid == 0x7e && flashutl_wsz == sizeof(uint8)))) {
            /* Get real devid */
            uint16 flash_devid_5th;

            flash_devid_5th = flash_readword(FLASH_ADDR(0x1c)) << 8;
            flash_devid = (flash_readword(FLASH_ADDR(0x1e)) & 0xff) | flash_devid_5th;
        }
        
        flashutl_desc = flashes;
        while (flashutl_desc->mfgid != 0 &&
               !(flashutl_desc->mfgid == flash_vendid &&
                 flashutl_desc->devid == flash_devid)) {
            flashutl_desc++;
        }
        if (flashutl_desc->mfgid != 0)
        break;
    }
    
    /* Restore flashutl_wsz to original CC_CFG_DS value after probing flash device ID */
    if (IS_8_BIT) {
        flashutl_wsz = (R_REG(osh, &cc->pflash_config) & CC_CFG_DS) ? 
            sizeof(uint16) : sizeof(uint8);
    }
    
    if (flashutl_desc->mfgid == 0) {
        flashutl_desc = NULL;
        flashutl_cmd = NULL;
    } else {
        flashutl_cmd = flash_cmds;
        while (flashutl_cmd->type != 0 && flashutl_cmd->type != flashutl_desc->type)
            flashutl_cmd++;
        if (flashutl_cmd->type == 0)
            flashutl_cmd = NULL;
    }
    
    if (flashutl_cmd != NULL) {
        flash_reset();
    }
    
    if (flashutl_desc == NULL) {
        if (flash_str)
            sprintf(flash_str, "UNKNOWN 0x%x 0x%x", flash_vendid, flash_devid);
        DPRINT(("Flash type UNKNOWN\n"));
        return 1;
    }
    
    if (flash_str)
        strcpy(flash_str, flashutl_desc->desc);
    DPRINT(("Flash type \"%s\"\n", flashutl_desc->desc));
    
    return 0;
}

static int
flash_eraseblk(unsigned long addr)
{
    unsigned long a;
    uint16 st;
    
    a = (unsigned long)addr;
    if (a >= flashutl_desc->size)
        return 1;
    
    a = block(a, BLOCK_BASE);
    
    /* Ensure blocks are unlocked (for intel chips) */
    if (flashutl_cmd->type == BSC) {
        scmd((unsigned char)INTEL_UNLOCK1, a);
        scmd((unsigned char)INTEL_UNLOCK2, a);
    }
    
    if (flashutl_cmd->pre_erase)
        cmd(flashutl_cmd->pre_erase, CMD_ADDR);
    if (flashutl_cmd->erase_block)
        cmd(flashutl_cmd->erase_block, a);
    if (flashutl_cmd->confirm)
        scmd(flashutl_cmd->confirm, a);
    
    if (flashutl_wsz == sizeof(uint8))
         st = flash_poll(a, 0xff);
    else
         st = flash_poll(a, 0xffff);
    
    flash_reset();
    
    if (st) {
        DPRINT(("Erase of block 0x%08lx-0x%08lx failed\n",
            a, block((unsigned long)addr, BLOCK_LIM)));
        return st;
    }
    
    DPRINT(("Erase of block 0x%08lx-0x%08lx done\n", 
        a, block((unsigned long)addr, BLOCK_LIM)));
    
    return 0;
}

static int
flash_write(unsigned long off, uint8 *src, uint nbytes)
{
    uint8 *dest;
    uint16 st, data;
    uint i, len;
    
    ASSERT(flashutl_desc != NULL);
    
    if (off >= flashutl_desc->size)
        return 1;
    
    ASSERT(!(off & (flashutl_wsz - 1)));
    
    dest = (uint8*)FLASH_ADDR(off);
    st = 0;
    
    while (nbytes) {
        if ((flashutl_desc->type == SCS) &&
            flashutl_cmd->write_buf &&
            ((off & (WBUFSIZE - 1)) == 0)) {
            /* issue write command */
            if (flashutl_cmd->write_buf)
                cmd(flashutl_cmd->write_buf, off);
            if ((st = flash_poll(off, DONE)))
                continue;
            
            len = MIN(nbytes, WBUFSIZE);
            
#ifndef MIPSEB
            /* write (length - 1) */
            cmd(len / sizeof(uint16) - 1, off);
            
            /* write data */
            for (i = 0; i < len; i += sizeof(uint16),
                dest += sizeof(uint16), src += sizeof(uint16))
               *(uint16 *)dest = *(uint16 *)src;
#else
            /*
             * BCM4710 endianness is word consistent but
             * byte/short scrambled. This write buffer
             * mechanism appears to be sensitive to the
             * order of the addresses hence we need to
             * unscramble them. We may also need to pad
             * the source with two bytes of 0xffff in case
             * an odd number of shorts are presented.
             */
            
            /* write (padded length - 1) */
            cmd((ROUNDUP(len, sizeof(uint32)) / sizeof(uint16)) - 1, off);
            
            /* write data (plus pad if necessary) */
            for (i = 0; i < ROUNDUP(len, sizeof(uint32)); i += sizeof(uint32),
                         dest += sizeof(uint32), src += sizeof(uint32)) {
                *((uint16 *)dest + 1) = ((i + sizeof(uint16)) < len) ?
                        *((uint16 *)src + 1) : 0xffff;
                *(uint16 *)dest = *(uint16 *)src;
            }
#endif /* MIPSEB */
            
            /* write confirm */
            if (flashutl_cmd->confirm)
                cmd(flashutl_cmd->confirm, off);
            
            if ((st = flash_poll(off, DONE)))
                break;
        } else {
            /* issue write command */
            if (flashutl_cmd->write_word)
                cmd(flashutl_cmd->write_word, CMD_ADDR);
            
            /* write data */
            data = flash_readword((unsigned long)src);
            flash_writeword((unsigned long)dest, data);
            
            /* poll for done */
            if ((st = flash_poll(off, data)))
                break;
            
            len = MIN(nbytes, flashutl_wsz);
            dest += len;
            src += len;
        }
        
        nbytes -= len;
        off += len;
    }
    
    flash_reset();
    
    return st;
}

static uint16 INLINE
flash_readword(unsigned long addr)
{
    if (flashutl_wsz == sizeof(uint8))
        return *(uint8*)addr;
    else
        return *(uint16*)addr;
}

static void INLINE
flash_writeword(unsigned long addr, uint16 data)
{
    if (flashutl_wsz == sizeof(uint8))
        *(uint8*)addr = (uint8)data;
    else
        *(uint16*)addr = data;
}

/* Writes a single command to the flash. */
static void
scmd(uint16 cmd, unsigned long off)
{
    /* XXX should the command word be doubled up to 16 bits? */
    /*  cmd |= cmd << 8; */
    
    flash_writeword(FLASH_ADDR(off), cmd);
}

/* Writes a command to flash, performing an unlock if needed. */
static void
cmd(uint16 cmd, unsigned long off)
{
    int i;
    unlock_cmd_t *ul = NULL;
    
    ASSERT(flashutl_cmd != NULL);
    
    switch (flashutl_cmd->type) {
    case AMD:
        ul = &unlock_cmd_amd;
        break;
    case SST:
        ul = &unlock_cmd_sst;
        break;
    default:
        break;
    }
    
    if (flashutl_cmd->need_unlock) {
        ASSERT(ul);
        for (i = 0; i < UNLOCK_CMD_WORDS; i++) {
            /* flash_writeword(FLASH_ADDR(ul->addr[i]), ul->cmd[i]); */
            /* write flash commands by 8-bits  */
            *(uint8*)FLASH_ADDR(ul->addr[i]) = (uint8)ul->cmd[i];
        }
    }
    
    /* XXX should the command word be doubled up to 16 bits? */
    /* cmd |= cmd << 8; */
    
    if (off == CMD_ADDR) {
        switch (flashutl_cmd->type) {
        case AMD:
            off = AMD_CMD;
            break;
        case SST:
            off = SST_CMD;
            break;
        default:
            off = 0;
            break;
        }
    }
    
    /* flash_writeword(FLASH_ADDR(off), cmd); */
    /* write flash commands by 8-bits  */
    *(uint8*)FLASH_ADDR(off) = (uint8)cmd;
}

static void
flash_reset()
{
    ASSERT(flashutl_desc != NULL);
    
    if (flashutl_cmd->clear_csr)
        scmd(flashutl_cmd->clear_csr, 0);
    if (flashutl_cmd->read_array)
        scmd(flashutl_cmd->read_array, 0);
}

static int
flash_poll(unsigned long off, uint16 data)
{
    unsigned long addr;
    int cnt = FLASH_TRIES;
    uint16 st;
    
    ASSERT(flashutl_desc != NULL);
    
    if (flashutl_desc->type == AMD || flashutl_desc->type == SST) {
        /* AMD style poll checkes the address being written */
        addr = FLASH_ADDR(off);
        while ((st = flash_readword(addr)) != data && cnt != 0)
            cnt--;
        if (cnt == 0) {
            DPRINT(("flash_poll: timeout, off %lx, read 0x%x, expected 0x%x\n",
                off, st, data));
            return -1;
        }
    } else {
        /* INTEL style poll is at second word of the block being written */
        addr = FLASH_ADDR(block(off, BLOCK_BASE)+sizeof(uint16));
        while (((st = flash_readword(addr)) & DONE) == 0 && cnt != 0)
            cnt--;
        if (cnt == 0) {
            DPRINT(("flash_poll: timeout, error status = 0x%x\n", st));
            return -1;
        }
    }
    
    return 0;
}

static unsigned long
block(unsigned long addr, int which)
{
    unsigned long b, l, sb;
    uint* sblocks;
    int i;
    
    ASSERT(flashutl_desc != NULL);
    
    ASSERT(addr < (unsigned long)flashutl_desc->size);
    
    b = addr / flashutl_desc->bsize;
    /* check for an address a full size block */
    if (b >= flashutl_desc->ff && b <= flashutl_desc->lf) {
        if (which == BLOCK_LIM) b++;
        return (b * flashutl_desc->bsize);
    }
    
    /* search for the sub-block */
    if (flashutl_desc->ff == 0) {
        /* sub blocks are at the end of the flash */
        sb = flashutl_desc->bsize * (flashutl_desc->lf + 1);
    } else {
        /* sub blocks are at the start of the flash */
        sb = 0;
    }
    
    sblocks = flashutl_desc->subblocks;
    for (i = 0; i < flashutl_desc->nsub; i++) {
        b = sb + sblocks[i];
        l = sb + sblocks[i+1];
        if (addr >= b && addr < l) {
            if (which == BLOCK_BASE)
                return b;
            else
                return l;
        }
    }
    
    return 0;
}

void
nvStartOffset(uint off)
{
    nvram_start_offset = off;
}

void
nvWrite(unsigned short *data, unsigned int len)
{
    uint off = nvram_start_offset;
    sysFlashWrite(off, (uchar*)data, len);
}

void
nvWriteChars(unsigned char *data, unsigned int len)
{
    uint off = nvram_start_offset;
    int err;
    
    if (flashutl_cmd->type == SFLASH)
        err = sflash_commit(sih, cc, off, len, data);
    else /* PFLASH */
        err = flash_write(off, data, len);
    
    if (err)
        DPRINT(("nvWriteChars failed\n"));
    else
        DPRINT(("nvWriteChars succeeded\n"));
}

int
sysFlashErase(uint off, unsigned int numbytes)
{
    unsigned long end = off + numbytes;
    int err = 0;
    
    if (flashutl_cmd->type == SFLASH) {
        err = sflash_commit(sih, cc, off, numbytes, NULL);
    } else {
        while (off < end) {
            err = flash_eraseblk(off);
            if (err)
                break;
            off = block(off, BLOCK_LIM);
        }
    }
    
    if (err)
        DPRINT(("Block erase at 0x%x failed\n", off));
    else
        DPRINT(("Done\n"));
    
    return !err;
}

int
sysFlashWrite(uint off, uchar *src, uint numbytes)
{
    int err;
    
    DPRINT(("Writing 0x%x bytes to flash @0x%x ...\n", (unsigned int)numbytes, off));
    
    if (flashutl_cmd->type == SFLASH)
        err = sflash_commit(sih, cc, off, numbytes, src);
    else {
        if (!sysFlashErase(off, numbytes))
            return 0;
        err = flash_write(off, src, numbytes);
    }
    
    if (err)
        DPRINT(("Flash write failed\n"));
    else
        DPRINT(("Flash write succeeded\n"));
    
    return !err;
}

int
sysFlashRead(uint off, uchar *buf, uint numbytes)
{
    uint read, total_read = 0;
    
    if (flashutl_cmd->type == SFLASH) {
        while (numbytes) {
        read = sflash_read(sih, cc, off, numbytes, buf);
        numbytes -= read;
        buf += read;
        off += read;
        total_read += read;
        }
    } else {
        ASSERT(!(off & (flashutl_wsz - 1)));
        ASSERT(!(numbytes & (flashutl_wsz - 1)));
        
        while (numbytes) {
            flash_writeword((unsigned long)buf, flash_readword(FLASH_ADDR(off)));
            numbytes -= flashutl_wsz;
            buf += flashutl_wsz;
            off += flashutl_wsz;
            total_read += flashutl_wsz;
        }
    }
    
    return (total_read);
}

