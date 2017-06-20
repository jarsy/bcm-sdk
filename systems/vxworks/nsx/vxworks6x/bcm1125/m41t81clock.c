/*
 * $Id: m41t81clock.c,v 1.2 2006/05/03 23:46:11 sanjayg Exp $
 *
 * M41T81 RTC driver
 *
 * This module contains driver for M41T81 SMBus real-time-clock.
 *
 * Author: jzhao@broadcom.com
 */
#include "vxWorks.h"
#include "taskLib.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "ctype.h"
#include "config.h"
#include "bcm1250Lib.h"
#include "m41t81clock.h"

/*
 * Initialize the SMBus (RTC uses channel 1)
 */
static int
bcm1250_smbus_init(int smb_chan)
{
    /*
     * Assume 100KHz for all devices.  We don't need to go fast
     * ever. Also turn off direct mode and disable interrupts.
     */
    SBWRITECSR(A_SMB_REGISTER(smb_chan, R_SMB_FREQ), K_SMB_FREQ_100KHZ);
    SBWRITECSR(A_SMB_REGISTER(smb_chan, R_SMB_CONTROL), 0);

    return 0;
}

/*
 * Wait for SMBus
 */
static int
bcm1250_smbus_waitready(int smb_chan)
{
    UINT64 status = 0;
    int cnt = 10000000;        /* about 1 second at 1Ghz */

    while (cnt > 0) {
        status = SBREADCSR(A_SMB_REGISTER(smb_chan, R_SMB_STATUS));
        if (status & M_SMB_BUSY) {
            cnt--;
            continue;
        }
        break;
    }

    if (cnt == 0) return -1;

    if (status & M_SMB_ERROR) {
        SBWRITECSR(A_SMB_REGISTER(smb_chan, R_SMB_STATUS), (status & M_SMB_ERROR));
        return -1;
    }

    return 0;
}

/*
 * read from SMBus
 */
static int
bcm1250_smbus_read(int smb_chan, UINT8 slave, UINT8 *buf, int len)
{
    int err;

    while (len > 0) {
        err = bcm1250_smbus_waitready(smb_chan);
        if (err < 0) return err;

        SBWRITECSR(A_SMB_REGISTER(smb_chan, R_SMB_START),
                   V_SMB_TT(K_SMB_TT_RD1BYTE) | ((UINT64)slave));

        err = bcm1250_smbus_waitready(smb_chan);
        if (err < 0) return err;

        *buf++ = (UINT8) SBREADCSR(A_SMB_REGISTER(smb_chan, R_SMB_DATA));
        len--;
    }

    return 0;
}

/*
 * Write to SMBus
 */
static int
bcm1250_smbus_write(int smb_chan, UINT8 slave, UINT8 *buf, int len)
{
    int err;

    /*
     * Make sure the bus is idle (ignore error here)
     */
    bcm1250_smbus_waitready(smb_chan);

    /*
     * Depending on how many bytes we're writing, fill in the various
     * SMB registers and execute the command.
     */

    switch (len) {
        case 1:         /* "command" byte alone */
            SBWRITECSR(A_SMB_REGISTER(smb_chan, R_SMB_CMD), buf[0]);
            SBWRITECSR(A_SMB_REGISTER(smb_chan, R_SMB_START),
                       V_SMB_TT(K_SMB_TT_WR1BYTE) | ((UINT64)slave));
            break;
        case 2:         /* "command" byte plus a data byte */
            SBWRITECSR(A_SMB_REGISTER(smb_chan, R_SMB_CMD), buf[0]);
            SBWRITECSR(A_SMB_REGISTER(smb_chan, R_SMB_DATA), buf[1]);
            SBWRITECSR(A_SMB_REGISTER(smb_chan, R_SMB_START),
                       V_SMB_TT(K_SMB_TT_WR2BYTE) | ((UINT64)slave));
            break;
        case 3:         /* "command" byte plus 2 data bytes */
            SBWRITECSR(A_SMB_REGISTER(smb_chan, R_SMB_CMD), buf[0]);
            SBWRITECSR(A_SMB_REGISTER(smb_chan, R_SMB_DATA),
                       ((UINT64)(buf[1])) | (((UINT64)buf[2]) << 8));

            SBWRITECSR(A_SMB_REGISTER(smb_chan, R_SMB_START),
                       V_SMB_TT(K_SMB_TT_WR3BYTE) | ((UINT64)slave));
            break;
        default:
            return -1;
            break;
    }

    /*
     * Wait for command to complete.
     */
    err = bcm1250_smbus_waitready(smb_chan);
    if (err < 0) return err;

    return 0;
}

/*
 * Read RTC
 */
static int
time_readrtc(int slaveaddr, int devaddr)
{
    UINT8 buf[1];
    int err;

    /*
     * Start the command
     */
    buf[0] = devaddr;
    err = bcm1250_smbus_write(M41T81_SMBUS_CHAN, slaveaddr, buf, 1);
    if (err < 0) return err;

    /*
     * Read the data byte
     */
    err = bcm1250_smbus_read(M41T81_SMBUS_CHAN, slaveaddr, buf, 1);
    if (err < 0) return err;

    return (buf[0]);
}

/*
 * Write to RTC
 */
static int
time_writertc(int slaveaddr, int devaddr, int b)
{
    UINT8 buf[2];
    int err;
    int polls;

    /*
     * Start the command.  Keep pounding on the device until it
     * submits or the timer expires, whichever comes first.  The
     * datasheet says writes can take up to 10ms, so we'll give it 500.
     */

    buf[0] = devaddr;
    buf[1] = b;

    err = bcm1250_smbus_write(M41T81_SMBUS_CHAN, slaveaddr, buf, 2);
    if (err < 0) {
        return err;
    }

    /*
     * Pound on the device with a current address read
     * to poll for the write complete
     */
    err = -1;
    for (polls = 0; polls < 50; polls++) {
        err = bcm1250_smbus_read(M41T81_SMBUS_CHAN, slaveaddr, buf, 1);
        if (err == 0) break;
        taskDelay(1);
    }

    return err;
}

/*
 * Get RTC ready
 */
STATUS
m41t81_tod_init(void)
{
    UINT8 byte;
    int polls;
    int err;

    /* RTC is on SMBus 1 */
    bcm1250_smbus_init(M41T81_SMBUS_CHAN);

    /*
     * Reset HT bit to "0" to update registers with current time.
     */
    byte = time_readrtc(M41T81_CCR_ADDRESS, M41T81REG_AHR);
    byte &= ~M41T81REG_AHR_HT;
    time_writertc(M41T81_CCR_ADDRESS, M41T81REG_AHR, byte);

    /*
     * Try to read from the device.  If it does not
     * respond, fail.  We may need to do this for up to 300ms.
     */
    for (polls = 0; polls < 300; polls++) {
        taskDelay(1);
        err = time_readrtc(M41T81_CCR_ADDRESS, 0);
        if (err >= 0) break;              /* read is ok */
    }

    return 0;
}

STATUS
m41t81_tod_get(int *year,        /* 1980-2079 */
               int *month,       /* 01-12 */
               int *day,         /* 01-31 */
               int *hour,        /* 00-23 */
               int *minute,      /* 00-59 */
               int *second)      /* 00-59 */
{
    UINT8 byte;

    byte = (UINT8) time_readrtc(M41T81_CCR_ADDRESS, M41T81REG_HR);
    byte &= ~(M41T81REG_HR_CB | M41T81REG_HR_CEB);
    *hour = (UINT8) FROM_BCD(byte);

    byte = (UINT8) time_readrtc(M41T81_CCR_ADDRESS, M41T81REG_MN);
    *minute = (UINT8) FROM_BCD(byte);

    byte = (UINT8) time_readrtc(M41T81_CCR_ADDRESS, M41T81REG_SC);
    byte &= ~(M41T81REG_SC_ST);
    *second = (UINT8) FROM_BCD(byte);

    byte =  (UINT8) time_readrtc(M41T81_CCR_ADDRESS, M41T81REG_MO);
    *month = (UINT8) byte;

    byte = (UINT8) time_readrtc(M41T81_CCR_ADDRESS, M41T81REG_DT);
    *day = (UINT8) FROM_BCD(byte);

    byte =  (UINT8) time_readrtc(M41T81_CCR_ADDRESS, M41T81REG_YR);
    *year = (UINT8) byte;

    byte = (UINT8) time_readrtc(M41T81_CCR_ADDRESS, M41T81REG_HR);
    byte &= M41T81REG_HR_CB;
    if (byte) {
        *year += 2000;                   /*Year 20xx*/
    } else {
        *year += 1900;                   /*Year 19xx*/
    }

    return 0;
}

/*
 * Note: the TOD should store the current GMT
 */
STATUS
m41t81_tod_set(int year,            /* 1980-2079 */
               int month,           /* 01-12 */
               int day,             /* 01-31 */
               int hour,            /* 00-23 */
               int minute,          /* 00-59 */
               int second)          /* 00-59 */
{
    UINT8 y2k, temp;

    /* write time */
    temp = time_readrtc(M41T81_CCR_ADDRESS, M41T81REG_HR);
    temp &= (M41T81REG_HR_CB | M41T81REG_HR_CEB);
    hour = TO_BCD(hour);
    hour |= temp;
    time_writertc(M41T81_CCR_ADDRESS, M41T81REG_HR, hour);

    time_writertc(M41T81_CCR_ADDRESS, M41T81REG_MN, TO_BCD(minute));

    second &= ~M41T81REG_SC_ST;
    time_writertc(M41T81_CCR_ADDRESS, M41T81REG_SC, TO_BCD(second));

    /* write date */
    time_writertc(M41T81_CCR_ADDRESS, M41T81REG_MO, TO_BCD(month));

    time_writertc(M41T81_CCR_ADDRESS, M41T81REG_DT, TO_BCD(day));

    y2k = (year >= 2000) ? 0x20 : 0x19;
    year %= 100;
    time_writertc(M41T81_CCR_ADDRESS, M41T81REG_YR, TO_BCD(year));

    /*
     * M41T81 does not have a century byte, so we don't need to write y2k
     * But we should flip the century bit (CB) to "1" for year 20xx and to "0"
     * for year 19xx.
     */
    temp = (UINT8) time_readrtc(M41T81_CCR_ADDRESS, M41T81REG_HR);
    if (y2k == 0x19) {
        temp &= ~M41T81REG_HR_CB;
    } else if (y2k == 0x20) {
        temp |= M41T81REG_HR_CB;
    }
    time_writertc(M41T81_CCR_ADDRESS, M41T81REG_HR, temp);

    return 0;
}

