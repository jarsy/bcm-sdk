/*
 * $Id: memreg.c,v 1.6 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */
#include "memreg.h"

#include "chips/allfields.h"
#include "chips/fields.c"

#include "chips/allmems.c"
#include "chips/allregs.c"

#include "chips/bcm56218_a0.c"

#include "chips/chip.c"

#include "chips/driver.h"

/*
 * Base driver pointer
 */
soc_driver_t   *drivers[MAX_DEVICES];

/***********************************************
 * FIELD VALUE MANIPULATION FUNCTIONS
 ***********************************************/

/* Adjust justification for uint32 writes to fields */
/* dst is an array name of type uint32 [] */
#define SAL_MAC_ADDR_TO_UINT32(mac, dst) do {\
        dst[0] = (((uint32)mac[2]) << 24 | \
                 ((uint32)mac[3]) << 16 | \
                 ((uint32)mac[4]) << 8 | \
                 ((uint32)mac[5])); \
        dst[1] = (((uint32)mac[0]) << 8 | \
                 ((uint32)mac[1])); \
    } while (0)

/* Adjust justification for uint32 writes to fields */
/* src is an array name of type uint32 [] */
#define SAL_MAC_ADDR_FROM_UINT32(mac, src) do {\
        mac[0] = (uint8) (src[1] >> 8 & 0xff); \
        mac[1] = (uint8) (src[1] & 0xff); \
        mac[2] = (uint8) (src[0] >> 24); \
        mac[3] = (uint8) (src[0] >> 16 & 0xff); \
        mac[4] = (uint8) (src[0] >> 8 & 0xff); \
        mac[5] = (uint8) (src[0] & 0xff); \
    } while (0)

/*
 * Find field _fsrch in the field list _flist (with _fnum entries)
 * Sets _infop to the field_info of _fsrch or NULL if not found
 */
#define SOC_FIND_FIELD(_fsrch, _flist, _fnum, _infop) do { \
        soc_field_info_t *__f, *__e; \
        __f = _flist; \
        __e = &__f[_fnum]; \
        _infop = NULL; \
        while (__f < __e) \
        { \
            if (__f->field == _fsrch) \
            { \
                _infop = __f; \
                break; \
            } \
            __f++; \
        } \
    } while (0)

        /*
         * Macro used by memory accessor functions to fix order
         */
#define FIX_MEM_ORDER_E(v,m) (((m)->flags & SOC_MEM_FLAG_BE) ? \
    BYTES2WORDS((m)->bytes)-1-(v) : (v))

        /*
         * Function:      soc_meminfo_field_get
         * Purpose:       Get a memory field without reference to chip.
         * Parameters:    meminfo   --  direct reference to memory description
         */
        uint32 *
        soc_meminfo_field_get(soc_mem_info_t *meminfo, const uint32 *entbuf,
    soc_field_t field, uint32 *fldbuf, uint32 fldbuf_size) {
        soc_field_info_t    *fieldinfo;
    int                 i, wp, bp, len;

    SOC_FIND_FIELD(field,
        meminfo->fields,
        meminfo->nFields,
        fieldinfo);
    assert(fieldinfo);

    bp = fieldinfo->bp;
    len = fieldinfo->len;
    assert(len / 32 <= fldbuf_size); /* assert we do not write beyond the end of the output buffer */

    if (len == 1) { /* special case single bits */
        wp = bp / 32;
        bp = bp & (32 - 1);
        if (entbuf[FIX_MEM_ORDER_E(wp, meminfo)] & (1<<bp)) {
            fldbuf[0] = 1;
        } else {
            fldbuf[0] = 0;
        }
        return fldbuf;
    }

    if (fieldinfo->flags & SOCF_LE) {
        wp = bp / 32;
        bp = bp & (32 - 1);
        i = 0;

        for (; len > 0; len -= 32) {
            if (bp) {
                fldbuf[i] =
                    entbuf[FIX_MEM_ORDER_E(wp++, meminfo)] >> bp &
                    ((1 << (32 - bp)) - 1);
                fldbuf[i] |= entbuf[FIX_MEM_ORDER_E(wp, meminfo)] << (32 - bp);
            } else {
                fldbuf[i] = entbuf[FIX_MEM_ORDER_E(wp++, meminfo)];
            }

            if (len < 32) {
                fldbuf[i] &= ((1 << len) - 1);
            }

            i++;
        }
    } else {
        i = (len - 1) / 32;

        while (len > 0) {
            assert(i >= 0);

            fldbuf[i] = 0;

            do {
                fldbuf[i] =
                    (fldbuf[i] << 1) |
                    ((entbuf[FIX_MEM_ORDER_E(bp / 32, meminfo)] >>
                    (bp & (32 - 1))) & 1);
                len--;
                bp++;
            } while (len & (32 - 1));

            i--;
        }
    }

    return fldbuf;
}


/*
 * Function:    soc_mem_field_get
 * Purpose:     Get a memory field
 * Parameters:  unit - device
 *              mem - table
 *              entbuf - table entry buffer
 *              field - which field to get
 *              fldbuf - field buffer
 */
uint32 *
soc_mem_field_get(int unit, soc_mem_t mem, const uint32 *entbuf,
                  soc_field_t field, uint32 *fldbuf)
{
    soc_mem_info_t      *meminfo;

    meminfo = &SOC_MEM_INFO(unit, mem);

    assert(entbuf);
    assert(fldbuf);

    return soc_meminfo_field_get(meminfo, entbuf, field, fldbuf, SOC_MAX_MEM_WORDS);
}


/*
 * Function:      soc_meminfo_field_set
 * Purpose:       Set a memory field without reference to chip.
 * Parameters:    meminfo   --  direct reference to memory description
 */

/* Define a macro so the assertion printout is informative. */
#define VALUE_TOO_BIG_FOR_FIELD         ((fldbuf[i] & ~mask) != 0)

void
soc_meminfo_field_set(soc_mem_info_t *meminfo, uint32 *entbuf,
                      soc_field_t field, uint32 *fldbuf)
{
    soc_field_info_t    *fieldinfo;
    uint32              mask;
    int                 i, wp, bp, len;

    SOC_FIND_FIELD(field,
        meminfo->fields,
        meminfo->nFields,
        fieldinfo);
    assert(fieldinfo);

    bp = fieldinfo->bp;

    if (fieldinfo->flags & SOCF_LE) {
        wp = bp / 32;
        bp = bp & (32 - 1);
        i = 0;

        for (len = fieldinfo->len; len > 0; len -= 32) {
            if (bp) {
                if (len < 32) {
                    mask = (1 << len) - 1;
                    assert(!VALUE_TOO_BIG_FOR_FIELD);
                } else {
                    mask = -1;
                }

                entbuf[FIX_MEM_ORDER_E(wp, meminfo)] &= ~(mask << bp);
                entbuf[FIX_MEM_ORDER_E(wp++, meminfo)] |= fldbuf[i] << bp;
                entbuf[FIX_MEM_ORDER_E(wp, meminfo)] &= ~(mask >> (32 - bp));
                entbuf[FIX_MEM_ORDER_E(wp, meminfo)] |=
                    fldbuf[i] >> (32 - bp) & ((1 << bp) - 1);
            } else {
                if (len < 32) {
                    mask = (1 << len) - 1;
                    assert(!VALUE_TOO_BIG_FOR_FIELD);
                    entbuf[FIX_MEM_ORDER_E(wp, meminfo)] &= ~mask;
                    entbuf[FIX_MEM_ORDER_E(wp++, meminfo)] |= fldbuf[i] << bp;
                } else {
                    entbuf[FIX_MEM_ORDER_E(wp++, meminfo)] = fldbuf[i];
                }
            }

            i++;
        }
    } else { /* Big endian: swap bits */
        len = fieldinfo->len;

        while (len > 0) {
            len--;
            entbuf[FIX_MEM_ORDER_E(bp / 32, meminfo)] &= ~(1 << (bp & (32-1)));
            entbuf[FIX_MEM_ORDER_E(bp / 32, meminfo)] |=
                (fldbuf[len / 32] >> (len & (32-1)) & 1) << (bp & (32-1));
            bp++;
        }
    }
}


/*
 * Function:      soc_mem_field_set
 * Purpose:       Set a memory field
 */
void
soc_mem_field_set(int unit, soc_mem_t mem, uint32 *entbuf,
                  soc_field_t field, uint32 *fldbuf)
{
    soc_mem_info_t      *meminfo;

    meminfo = &SOC_MEM_INFO(unit, mem);

    soc_meminfo_field_set(meminfo, entbuf, field, fldbuf);
}


/*
 * Function:     soc_mem_field32_get
 * Purpose:      Get a <=32 bit field out of a memory entry
 * Returns:      The value of the field
 */
uint32
soc_mem_field32_get(int unit, soc_mem_t mem, const void *entbuf,
                    soc_field_t field)
{
    uint32 value;

    assert(entbuf && fldbuf);
    soc_meminfo_field_get(&SOC_MEM_INFO(unit, mem), entbuf, field, &value, 1);

    return value;
}


/*
 * Function:     soc_mem_field32_set
 * Purpose:      Set a <=32 bit field out of a memory entry
 * Returns:      void
 */
void
soc_mem_field32_set(int unit, soc_mem_t mem, void *entbuf,
                    soc_field_t field, uint32 value)
{
    soc_mem_field_set(unit, mem, entbuf, field, &value);
}


/*
 * Function:     soc_mem_mac_addr_set
 * Purpose:      Set a mac address field in a memory from a mac addr type
 * Returns:      void
 */
void
soc_mem_mac_addr_set(int unit, soc_mem_t mem, void *entry,
                     soc_field_t field, const sal_mac_addr_t mac)
{
    uint32              mac_field[2];

    SAL_MAC_ADDR_TO_UINT32(mac, mac_field);

    soc_mem_field_set(unit, mem, entry, field, mac_field);
}


/*
 * Function:     soc_meminfo_mac_addr_set
 * Purpose:      Set a mac address field in a memory from a mac addr type
 * Returns:      void
 */
void
soc_meminfo_mac_addr_set(soc_mem_info_t *meminfo, void *entry,
                         soc_field_t field, const sal_mac_addr_t mac)
{
    uint32              mac_field[2];

    SAL_MAC_ADDR_TO_UINT32(mac, mac_field);

    soc_meminfo_field_set(meminfo, entry, field, mac_field);
}


/*
 * Function:     soc_mem_mac_addr_get
 * Purpose:      Get a mac address field in a memory from a mac addr type
 * Returns:      SOC_E_xxx
 */
void
soc_mem_mac_addr_get(int unit, soc_mem_t mem, const void *entry,
                     soc_field_t field, sal_mac_addr_t mac)
{
    uint32 mac_field[2];

    soc_mem_field_get(unit, mem, entry, field, mac_field);

    SAL_MAC_ADDR_FROM_UINT32(mac, mac_field);
}


/***********************************************
 * S-Chanel operation
 ***********************************************/

/*
 * S-Chanel operation names
 */

static char *
_soc_schan_op_names[] = {
    "UNKNOWN_OPCODE",
    "BP_WARN_STATUS",      /* 0x01 */
    "BP_DISCARD_STATUS",   /* 0x02 */
    "COS_QSTAT_NOTIFY",    /* 0x03 */
    "HOL_STAT_NOTIFY",     /* 0x04 */
    "READ_MEM_CMD",        /* 0x07 */
    "READ_MEM_ACK",        /* 0x08 */
    "WRITE_MEM_CMD",       /* 0x09 */
    "WRITE_MEM_ACK",       /* 0x0a */
    "READ_REG_CMD",        /* 0x0b */
    "READ_REG_ACK",        /* 0x0c */
    "WRITE_REG_CMD",       /* 0x0d */
    "WRITE_REG_ACK",       /* 0x0e */
    "ARL_INSERT_CMD",      /* 0x0f */
    "ARL_INSERT_DONE",     /* 0x10 */
    "ARL_DELETE_CMD",      /* 0x11 */
    "ARL_DELETE_DONE",     /* 0x12 */
    "LINKSTAT_NOTIFY",     /* 0x13 */
    "MEM_FAIL_NOTIFY",     /* 0x14 */
    "INIT_CFAP",           /* 0x15 */
    "ENTER_DEBUG_MODE",    /* 0x17 */
    "EXIT_DEBUG_MODE",     /* 0x18 */
    "ARL_LOOKUP_CMD",      /* 0x19 */
    "L3_INSERT_CMD",       /* 0x1a */
    "L3_INSERT_DONE",      /* 0x1b */
    "L3_DELETE_CMD",       /* 0x1c */
    "L3_DELETE_DONE",      /* 0x1d */
    "L3_LOOKUP_CMD",       /* 0x1e */
    "UNKNOWN_OPCODE",      /* 0x1f */
    "L2_LOOKUP_CMD_MSG",   /* 0x20 */
    "L2_LOOKUP_ACK_MSG",   /* 0x21 */
    "L3X2_LOOKUP_CMD_MSG", /* 0x22 */
    "L3X2_LOOKUP_ACK_MSG", /* 0x23 */
};

char *
soc_schan_op_name(int op)
{
    #define COUNTOF(ary)            ((int) (sizeof (ary) / sizeof ((ary)[0])))
    if (op < 0 || op >= COUNTOF(_soc_schan_op_names)) {
        op = 0;
    }

    return _soc_schan_op_names[op];
}


static void
_soc_schan_reset(int unit)
{
    uint32 cmicConfig = soc_pci_read(unit, CMIC_CONFIG);

    /* Toggle S-Channel abort bit in CMIC_CONFIG register */

    soc_pci_write(unit, CMIC_CONFIG, cmicConfig | CC_SCHAN_ABORT);
    soc_pci_write(unit, CMIC_CONFIG, cmicConfig);

    sal_usleep(10 * MILLISECOND_USEC);
}


void
soc_schan_dump(int unit, schan_msg_t *msg, int dwc)
{
    char                buf[128];
    int                 i, j;

    PRINTF_DEBUG(("  HDR[CPU=%d COS=%d EBIT=%d ECODE=%d "
        "L=%d SRC=%d DST=%d OP=%d=%s ADDR=0x%08x]\n",
        msg->header.cpu, msg->header.cos,
        msg->header.ebit, msg->header.ecode,
        msg->header.datalen, msg->header.srcblk,
        msg->header.dstblk, msg->header.opcode,
        soc_schan_op_name(msg->header.opcode), msg->writecmd.address));

    PRINTF_DEBUG(("  HDR[CPU=%d COS=%d EBIT=%d ECODE=%d "
        "L=%d SRC=%d DST=%d OP=%d=%s]\n",
        msg->readresp.header.cpu, msg->readresp.header.cos,
        msg->readresp.header.ebit, msg->readresp.header.ecode,
        msg->readresp.header.datalen, msg->readresp.header.srcblk,
        msg->readresp.header.dstblk, msg->readresp.header.opcode,
        soc_schan_op_name(msg->readresp.header.opcode)));

    assert(dwc <= CMIC_SCHAN_WORDS(unit));

    for (i = 0; i < dwc; i += 4) {
        buf[0] = 0;

        for (j = i; j < i + 4 && j < dwc; j++) {
            sprintf(buf + strlen(buf), " DW[%2d]=0x%08x", j, msg->dwords[j]);
        }

        PRINTF_DEBUG((" %s\n", buf));
    }
}


int
soc_schan_op(int unit, schan_msg_t *msg, int dwc_write, int dwc_read, int intr)
{
    int i, rv;

    assert(dwc_write <= CMIC_SCHAN_WORDS(unit));
    assert(dwc_read <= CMIC_SCHAN_WORDS(unit));

    do {
        rv = SOC_E_NONE;

        /* Write raw S-Channel Data: dwc_write words */

        for (i = 0; i < dwc_write; i++) {
            soc_pci_write(unit, CMIC_SCHAN_MESSAGE(unit, i), msg->dwords[i]);
        }

        /* Tell CMIC to start */
        soc_pci_write(unit, CMIC_SCHAN_CTRL, SC_MSG_START_SET);

        /* Wait for completion using either the interrupt or polling method */
        {
            soc_timeout_t to;
            uint32 schanCtrl;

            soc_timeout_init(&to, SCHAN_TIMEOUT, 100);

            while (((schanCtrl = soc_pci_read(unit, CMIC_SCHAN_CTRL)) &
            SC_MSG_DONE_TST) == 0) {
                if (soc_timeout_check(&to)) {
                    rv = SOC_E_TIMEOUT;
                    break;
                }
            }

            /* if (rv == SOC_E_NONE) {
                PRINTF_DEBUG(("  Done in %d polls\n", to.polls));
            } */

            if (schanCtrl & SC_MSG_NAK_TST) {
                PRINTF_DEBUG(("  NAK received from SCHAN.\n"));
                rv = SOC_E_FAIL;
            }

            if (schanCtrl & SC_MSG_TIMEOUT_TST) {
                rv = SOC_E_TIMEOUT;
            }
            soc_pci_write(unit, CMIC_SCHAN_CTRL, SC_MSG_DONE_CLR);
        }

        if (rv == SOC_E_TIMEOUT) {
            PRINTF_DEBUG(("soc_schan_op: operation attempt timed out %d\n",
                globalCount));
            _soc_schan_reset(unit);
            break;
        }

        /* Read in data from S-Channel buffer space, if any */

        for (i = 0; i < dwc_read; i++) {
            msg->dwords[i] = soc_pci_read(unit, CMIC_SCHAN_MESSAGE(unit, i));
        }

    } while (0);

    if (rv == SOC_E_TIMEOUT) {
        PRINTF_ERROR(("soc_schan_op: operation timed out\n"));
        soc_schan_dump(unit, msg, dwc_write);
    }

    return rv;
}


/***********************************************
 * MEMORY WRITE/READ FUNCTIONS
 ***********************************************/

/*
 * Function:    soc_mem_addr
 * Purpose:     Turn a memory, block, and index into a memory address
 * Returns:     address to send off in an schannel message
 */

/*
 * Regular:      | Table(8) | Block(4) | Region(4) | Index(16) |
 * Monolithic:   | Table(8) | Index(24)                        |
 */
static uint32
soc_mem_addr(int unit, int mem, int blk, int index)
{
    soc_mem_info_t *mip = &SOC_MEM_INFO(unit, mem);
    uint32              blkoff;

    assert(blk >= 0 && blk < SOC_MAX_NUM_BLKS);
    assert(index >= 0);

    if ((mip->blocks & (1 << blk)) && !(mip->flags & SOC_MEM_FLAG_MONOLITH)) {
        blkoff = SOC_BLOCK2OFFSET(unit, blk) << SOC_BLOCK_BP;
    } else {
        blkoff = 0;
    }

    return mip->base + blkoff + (index * mip->gran);
}


/*
 * Function:
 *      soc_mem_read
 * Purpose:
 *      Read a memory internal to the SOC.
 * Notes:
 *      GBP/CBP memory should only accessed when MMU is in DEBUG mode.
 */

int
soc_mem_read(int unit, soc_mem_t mem, int copyno, int index, void *entry_data)
{
    schan_msg_t schan_msg;
    soc_mem_info_t *meminfo = &SOC_MEM_INFO(unit, mem);
    int entry_dw = BYTES2WORDS(meminfo->bytes);
    int index2, blk;
    uint32 maddr;
    int rv;

    if (copyno == MEM_BLOCK_ANY) {
        SOC_MEM_BLOCK_ITER(unit, meminfo, blk) {
            copyno = blk;
            break;
        }
    }

#ifdef DEBUG
    PRINTF_DEBUG(("soc_mem_read - %s\n", meminfo->name));
    PRINTF_DEBUG(("soc_mem_read copyno = %d\n", copyno));
#endif

    rv = SOC_E_NONE;

    /* Setup S-Channel command packet */
    schan_msg_clear(&schan_msg);
    schan_msg.readcmd.header.opcode = READ_MEMORY_CMD_MSG;
    schan_msg.readcmd.header.srcblk = HE_SOC_BLOCK2SCH_CMIC_BLOCK;
    schan_msg.readcmd.header.datalen = 4;

    index2 = index;

    schan_msg.readcmd.address = soc_mem_addr(unit, mem, copyno, index2);
    maddr = schan_msg.readcmd.address;
    schan_msg.readcmd.header.dstblk = (maddr >> SOC_BLOCK_BP) & 0xf;

    /*
     * Write onto S-Channel "memory read" command packet consisting of header
     * word + address word, and read back header word + entry_dw data words.
     */
    if ((rv = soc_schan_op(unit, &schan_msg, 2, 1 + entry_dw, 0)) < 0) {
        PRINTF_DEBUG(("soc_schan_op error\n"));
        goto done;
    }

    if (schan_msg.readresp.header.opcode != READ_MEMORY_ACK_MSG) {
        PRINTF_DEBUG(("soc_mem_read error 2\n"));
        soc_schan_dump( unit, &schan_msg, 2 + entry_dw);
        rv = SOC_E_INTERNAL;
        goto done;
    }

    sal_memcpy(entry_data,
        schan_msg.readresp.data,
        entry_dw * sizeof (uint32));

done:
    return rv;
}


int
soc_l2_insert(int unit,
              int copyno, uint8 *mac, int port, int vid, int tocpu)
{
    schan_msg_t schan_msg;
    int blk;
    soc_mem_info_t *meminfo = &SOC_MEM_INFO(unit, L2Xm);
    int entry_dw = BYTES2WORDS( meminfo->bytes );
    int rv;
    l2x_entry_t entry_data;

#ifdef DEBUG
    PRINTF_DEBUG(("mem - %s\n", meminfo->name));
#endif

    /*
     * Setup S-Channel command packet
     *
     * NOTE: the datalen field matters only for the Write Memory and
     * Write Register commands, where it is used only by the CMIC to
     * determine how much data to send, and is in units of bytes.
     */
    schan_msg_clear(&schan_msg);
    schan_msg.arlins.header.opcode = ARL_INSERT_CMD_MSG;
    schan_msg.arlins.header.srcblk = HE_SOC_BLOCK2SCH_CMIC_BLOCK;
    schan_msg.arlins.header.datalen = entry_dw * sizeof (uint32);
    schan_msg.arlins.header.dstblk = 0x7; 

    sal_memset(&entry_data, 0, sizeof(entry_data));
    soc_mem_mac_addr_set(unit, L2Xm, &entry_data, MAC_ADDRf, mac);
    soc_mem_field32_set(unit, L2Xm, &entry_data, VLAN_IDf, 1);
    soc_mem_field32_set(unit, L2Xm, &entry_data, CPUf, ((tocpu)?1:0));
    soc_mem_field32_set(unit, L2Xm, &entry_data, PORT_TGIDf, port);
    soc_mem_field32_set(unit, L2Xm, &entry_data, STATIC_BITf, 1);

    memcpy(schan_msg.arlins.data, &entry_data, entry_dw * sizeof (uint32));

    if (copyno != COPYNO_ALL) {
        if (!SOC_MEM_BLOCK_VALID(unit, meminfo, copyno)) {
            PRINTF_ERROR(("soc_mem_write: invalid block %d for memory\n", copyno));
            return SOC_E_PARAM;
        }
    }

    /* Write to one or all copies of the memory */

    rv = SOC_E_NONE;

    SOC_MEM_BLOCK_ITER(unit, meminfo, blk) {

        if (copyno != COPYNO_ALL && copyno != blk) {
            continue;
        }

        if ((rv = soc_schan_op(unit, &schan_msg, 1 + entry_dw, 0, 0)) < 0) {
            printf("l2 insert failed \n");
            goto done;
        }
    }

done:
    return rv;
}

int
soc_mem_write(int unit,
              soc_mem_t mem, /* Use COPYNO_ALL for all */
              int copyno, int index, void *entry_data)
{
    schan_msg_t schan_msg;
    int blk;
    soc_mem_info_t *meminfo = &SOC_MEM_INFO(unit, mem);
    int entry_dw = BYTES2WORDS( meminfo->bytes );
    int index2;
    int rv;

#ifdef DEBUG
    PRINTF_DEBUG(("mem - %s\n", meminfo->name));
#endif

    /*
     * Setup S-Channel command packet
     *
     * NOTE: the datalen field matters only for the Write Memory and
     * Write Register commands, where it is used only by the CMIC to
     * determine how much data to send, and is in units of bytes.
     */

    schan_msg_clear(&schan_msg);
    schan_msg.writecmd.header.opcode = WRITE_MEMORY_CMD_MSG;
    schan_msg.writecmd.header.srcblk = HE_SOC_BLOCK2SCH_CMIC_BLOCK;
    schan_msg.writecmd.header.datalen = entry_dw * sizeof (uint32);

    memcpy(schan_msg.writecmd.data, entry_data, entry_dw * sizeof (uint32));

    if (copyno != COPYNO_ALL) {
        if (!SOC_MEM_BLOCK_VALID(unit, meminfo, copyno)) {
            PRINTF_ERROR(("soc_mem_write: invalid block %d for memory\n", copyno));
            return SOC_E_PARAM;
        }
    }

    /* Write to one or all copies of the memory */

    rv = SOC_E_NONE;

    SOC_MEM_BLOCK_ITER(unit, meminfo, blk) {

        if (copyno != COPYNO_ALL && copyno != blk) {
            continue;
        }

        index2 = index;

        schan_msg.writecmd.address = soc_mem_addr(unit, mem, blk, index2);

        schan_msg.writecmd.header.dstblk =
            (schan_msg.writecmd.address >> SOC_BLOCK_BP) & 0xf;

        /* Write header + address + entry_dw data DWORDs */
        /* Note: The hardware does not send WRITE_MEMORY_ACK_MSG. */
        /* soc_schan_dump( unit, &schan_msg, 2 + entry_dw); */
        if ((rv = soc_schan_op(unit, &schan_msg, 2 + entry_dw, 0, 0)) < 0) {
            goto done;
        }
    }

done:
    return rv;
}


static const uint32 _NULLMEM_ENTRY[128];

int
soc_mem_clear(int unit, int mem, int copyno, int force_all)
{
    int         blk, index;
    soc_mem_info_t *meminfo = &SOC_MEM_INFO(unit, mem);
    int         rv = SOC_E_NONE;

    SOC_MEM_BLOCK_ITER(unit, meminfo, blk) {
        if (copyno != COPYNO_ALL && copyno != blk) {
            continue;
        }

        for (index = meminfo->index_min; index <= meminfo->index_max; index++) {
            if ((rv = soc_mem_write(unit, mem, blk, index,
                                    (void*)_NULLMEM_ENTRY)) < 0) {
                PRINTF_ERROR(("soc_mem_clear: FAIL @ %d", index));
                goto done;
            }
        }
    }

done:
    return rv;
}


/****************************************************************
 * Register field manipulation functions
 ****************************************************************/

#define REG_FIELD_IS_VALID      finfop
#define REG_VALUE_TOO_BIG_FOR_FIELD         ((value & ~mask) != 0)

/*
 * Function:     soc_reg_field_length
 * Purpose:      Return the length of a register field in bits.
 *               Value is 0 if field is not found.
 * Returns:      bits in field
 */
int
soc_reg_field_length(int unit, soc_reg_t reg, soc_field_t field)
{
    soc_field_info_t    *finfop;

    SOC_FIND_FIELD(field,
        SOC_REG_INFO(unit, reg).fields,
        SOC_REG_INFO(unit, reg).nFields,
        finfop);
    if (finfop == NULL) {
        return 0;
    }

    return finfop->len;
}


/*
 * Function:     soc_reg_field_get
 * Purpose:      Get the value of a field from a register
 * Parameters:
 * Returns:      Value of field
 */
uint32
soc_reg_field_get(int unit, soc_reg_t reg, uint32 current, soc_field_t field)
{
    soc_field_info_t *finfop;
    uint32           val;

    SOC_FIND_FIELD(field,
        SOC_REG_INFO(unit, reg).fields,
        SOC_REG_INFO(unit, reg).nFields,
        finfop);
    assert(REG_FIELD_IS_VALID);

    val = current >> finfop->bp;
    if (finfop->len < 32) {
        return val & ((1 << finfop->len) - 1);
    } else {
        return val;
    }
}


/*
 * Function:     soc_reg_field_set
 * Purpose:      Set the value of a register's field.
 * Parameters:
 * Returns:      void
 */
void
soc_reg_field_set(int unit, soc_reg_t reg, uint32 *current,
                  soc_field_t field, uint32 value)
{
    soc_field_info_t    *finfop;
    uint32              mask;

    SOC_FIND_FIELD(field,
        SOC_REG_INFO(unit, reg).fields,
        SOC_REG_INFO(unit, reg).nFields,
        finfop);
    assert(REG_FIELD_IS_VALID);

    if (finfop->len < 32) {
        mask = (1 << finfop->len) - 1;
        assert(!REG_VALUE_TOO_BIG_FOR_FIELD);
    } else {
        mask = -1;
    }

    *current = (*current & ~(mask << finfop->bp)) | value << finfop->bp;
}


/***********************************************
 * Regsiter Read/Write Functions
 ***********************************************/

/*
 * Function:    soc_reg_addr
 * Purpose:     calculate the address of a register
 * Parameters:
 *              unit    switch unit
 *              reg     register number
 *              port    port number or REG_PORT_ANY
 *              index   array index (or cos number)
 * Returns:     register address suitable for soc_reg_read and friends
 * Notes:       the block number to access is determined by the register
 *              and the port number
 *
 * cpureg       00SSSSSS 00000000 0000RRRR RRRRRRRR
 * genreg       00SSSSSS BBBB1000 0000RRRR RRRRRRRR
 * portreg      00SSSSSS BBBB00PP PPPPRRRR RRRRRRRR
 * cosreg       00SSSSSS BBBB01CC CCCCRRRR RRRRRRRR
 *
 * where        B+ is the 4 bit block number
 *              P+ is the 6 bit port number (within a block or chip wide)
 *              C+ is the 6 bit class of service
 *              R+ is the 12 bit register number
 *              S+ is the 6 bit Pipe stage
 */
uint32
soc_reg_addr(int unit, soc_reg_t reg, int port, int index)
{
    uint32       base;        /* base address from reg_info */
    int          block, blk;  /* block number */
    int          pindex;      /* register port/cos field */
    int          gransh;      /* index granularity shift */
    soc_block_t  regblktype;

    #define SOC_REG_IS_VALID(unit, reg) \
        (reg >= 0 && reg < NUM_SOC_REG && (SOC_REG_PTR(unit, reg) != NULL))

    assert(SOC_REG_IS_VALID(unit, reg));

#define SOC_REG_ADDR_INVALID_PORT 0 /* for asserts */

    regblktype = SOC_REG_INFO(unit, reg).block;
    if (port >= 0) {
        block = SOC_PORT_INFO(unit, port).blk;
        pindex = SOC_PORT_INFO(unit, port).bindex;
    } else if (port == REG_PORT_ANY) {
        port = 0;
        block = SOC_PORT_INFO(unit, port).blk;
        pindex = SOC_PORT_INFO(unit, port).bindex;
    } else {
        assert(SOC_REG_ADDR_INVALID_PORT); /* invalid port */
        block = pindex = -1;
    }

    if (!(regblktype & SOC_BLK_PORT)) {
        for (blk = 0; SOC_BLOCK_INFO(unit, blk).type >= 0; blk++) {
            if (SOC_BLOCK_INFO(unit, blk).type == regblktype) {
                block = blk;
            }
        }
    }

    assert(block >= 0); /* block must be valid */

    /* determine final block, pindex, and index */
    gransh = 0;
    switch (SOC_REG_INFO(unit, reg).regtype) {
        case soc_cpureg:
            block = -1;
            pindex = 0;
            gransh = 2; /* 4 byte granularity */
            break;
        case soc_portreg:
            if (!(regblktype & SOC_BLK_PORT)) {
                pindex = port;
            }
            break;
        case soc_cosreg:
            pindex = index;
            index = 0;
            break;
        case soc_genreg:
            pindex = 0;
            break;
        default:
            assert(0); /* unknown register type */
            break;
    }

    /* put together address: base|block|pindex + index */
    base = SOC_REG_INFO(unit, reg).offset;

    if (block >= 0) {
        base |= SOC_BLOCK2OFFSET(unit, block) << SOC_BLOCK_BP;
    }

    if (pindex) {
        base |= pindex << SOC_REGIDX_BP;
    }

    if (index && SOC_REG_ARRAY(unit, reg)) {
        assert(index >= 0 && index < SOC_REG_NUMELS(unit, reg));
        base += (index << gransh);
    }

    return base;
}


int
soc_reg32_write(int unit, uint32 addr, uint32 data)
{
    schan_msg_t schan_msg;

    /*
     * Setup S-Channel command packet
     *
     * NOTE: the datalen field matters only for the Write Memory and
     * Write Register commands, where it is used only by the CMIC to
     * determine how much data to send, and is in units of bytes.
     */

    schan_msg_clear(&schan_msg);
    schan_msg.writecmd.header.opcode = WRITE_REGISTER_CMD_MSG;
    schan_msg.writecmd.header.srcblk = HE_SOC_BLOCK2SCH_CMIC_BLOCK;
    schan_msg.writecmd.header.datalen =  4;
    schan_msg.writecmd.header.dstblk = (addr >> SOC_BLOCK_BP) & 0xf;
    schan_msg.writecmd.address = addr;
    schan_msg.writecmd.data[0] = data;

    /* Write header word + address + data DWORD */
    /* Note: The hardware does not send WRITE_REGISTER_ACK_MSG. */

    return soc_schan_op(unit, &schan_msg, 3, 0, 0);
}


/*
 * Read an internal SOC register through S-Channel messaging buffer.
 */

int
soc_reg32_read(int unit, uint32 addr, uint32 *data)
{
    schan_msg_t schan_msg;

    /*
     * Write message to S-Channel.
     */
    schan_msg_clear(&schan_msg);
    schan_msg.readcmd.header.opcode = READ_REGISTER_CMD_MSG;
    schan_msg.readcmd.header.datalen = 4;
    schan_msg.writecmd.header.srcblk = HE_SOC_BLOCK2SCH_CMIC_BLOCK;
    schan_msg.readcmd.header.dstblk = (addr >> SOC_BLOCK_BP) & 0xf;
    schan_msg.readcmd.address = addr;

    /* Write header word + address DWORD, read header word + data DWORD */
    SOC_IF_ERROR_RETURN(soc_schan_op(unit, &schan_msg, 2, 2, 0));

    /* Check result */

    if (schan_msg.readresp.header.opcode != READ_REGISTER_ACK_MSG) {
        PRINTF_ERROR((
            "soc_reg32_read: "
            "invalid S-Channel reply, expected READ_REG_ACK:\n"));
        soc_schan_dump(unit, &schan_msg, 2);
        return SOC_E_INTERNAL;
    }

    *data = schan_msg.readresp.data[0];

    return SOC_E_NONE;
}
