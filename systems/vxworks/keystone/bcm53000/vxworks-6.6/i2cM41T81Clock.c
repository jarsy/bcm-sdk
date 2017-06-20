/*
 * $Id: i2cM41T81Clock.c,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#include <typedefs.h>
#include <osl.h>
#include <sbchipc.h>
#include <hndsoc.h>
#include <siutils.h>
#include <pcicfg.h>
#include <nicpci.h>
#include <bcmutils.h>
#include <bcmdevs.h>

#include "vxWorks.h"
#include "taskLib.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "ctype.h"
#include "config.h"
#include "chipc_i2c.h"
#include "i2cM41T81Clock.h"

static int m41t81_exist = 0;

static int time_readrtc(int chan,int slaveaddr, int devaddr, char *b)
{
    uint8_t buf[1];
    int err = 0;

    /*
     * Start the command
     */

    buf[0] = devaddr;
    err = ksi2c_write(chan,slaveaddr,buf,1);
    if (err < 0) {
    	return err;
    }

    /*
     * Read the data byte
     */

    err = ksi2c_read(chan,slaveaddr,buf,1);
    if (err < 0) {
    	return err;
    }

    *b = buf[0];
    
    return err;
}

static int time_writertc(int chan,int slaveaddr,int devaddr, int b)
{
    uint8_t buf[2];
    int err;
    int polls;

    /*
     * Start the command.  Keep pounding on the device until it
     * submits or the timer expires, whichever comes first.  The
     * datasheet says writes can take up to 10ms, so we'll give it 500.
     */

    buf[0] = devaddr;
    buf[1] = b;

    err = ksi2c_write(chan,slaveaddr,buf,2);
    if (err < 0) {
    	return err;
    }

    /*
     * Pound on the device with a current address read
     * to poll for the write complete
     */
    err = -1; 

    for (polls = 0; polls < 300; polls++) {
        taskDelay(1);
        err = ksi2c_read(chan,slaveaddr,buf,1);
        if (err == 0) break;
    }

    return err;
}

/*
 * Get RTC ready
 */
STATUS
m41t81_tod_init(void)
{
#ifndef BUILD_VXBOOT
    char *name=NULL;

    if ((name = getvar(NULL,"boardtype")) == NULL) {
        return -1;
    }

    if(!strncmp(name,"bcm953003c",strlen("bcm953003c"))){
        m41t81_exist = 1;
    } else {
        m41t81_exist = 0;
    }
#endif

    return 0;
}

int m41t81_tod_kick_start(void)
{
    char byte;
    int polls;
    int status;

    /*
     * Reset ST bit to "0" .
     */
    status = time_readrtc(M41T81_SMBUS_CHAN, M41T81_CCR_ADDRESS, 
                                       M41T81REG_SC, &byte);
    if (byte & 0x80) {
        byte &= 0x7f;
        time_writertc(M41T81_SMBUS_CHAN,M41T81_CCR_ADDRESS,
                                M41T81REG_SC, byte);
    }

    /*
     * Reset HT bit to "0" to update registers with current time.
     */
    status = time_readrtc(M41T81_SMBUS_CHAN, M41T81_CCR_ADDRESS, 
                                       M41T81REG_AHR, &byte);
    if (byte & M41T81REG_AHR_HT) {
        byte &= ~M41T81REG_AHR_HT;
        time_writertc(M41T81_SMBUS_CHAN,M41T81_CCR_ADDRESS,
                               M41T81REG_AHR, byte);
    }

    /*
     * Try to read from the device.  If it does not
     * respond, fail.  We may need to do this for up to 300ms.
     */
    for (polls = 0; polls < 300; polls++) {
        taskDelay(1);
        status = time_readrtc(M41T81_SMBUS_CHAN, M41T81_CCR_ADDRESS, 0, &byte);
        if (status == OK) break;              /* read is ok */
    }
    return (status == OK) ? 0 : -1;
}

int
m41t81_tod_get_second(void)
{
    int second;
    char  byte;

    second = 0;
    if (time_readrtc(M41T81_SMBUS_CHAN,
                     M41T81_CCR_ADDRESS, M41T81REG_SC, &byte) == 0) {
        byte &= ~(M41T81REG_SC_ST);
        second = (UINT8) FROM_BCD(byte);
    }

    return second;
}

STATUS
m41t81_tod_get(int *year,        /* 1980-2079 */
               int *month,       /* 01-12 */
               int *date,        /* 01-31 */
               int *hour,        /* 00-23 */
               int *minute,      /* 00-59 */
               int *second)      /* 00-59 */
{
    int   status;
    unsigned char  byte;
    int   y2k;

    if (!m41t81_exist) {
        return -1;
    }

    status = time_readrtc(M41T81_SMBUS_CHAN, M41T81_CCR_ADDRESS, 
                                       M41T81REG_HR, &byte);
    y2k = ((byte & M41T81REG_HR_CB) == M41T81REG_HR_CB);
    byte &= ~(M41T81REG_HR_CB | M41T81REG_HR_CEB);
    *hour = (UINT8) FROM_BCD(byte);

    status = time_readrtc(M41T81_SMBUS_CHAN, M41T81_CCR_ADDRESS, 
                                       M41T81REG_MN, &byte);
    *minute = (UINT8) FROM_BCD(byte);

    status = time_readrtc(M41T81_SMBUS_CHAN, M41T81_CCR_ADDRESS, 
                                       M41T81REG_SC, &byte);
   byte &= ~(M41T81REG_SC_ST);
    *second = (UINT8) FROM_BCD(byte);

    status = time_readrtc(M41T81_SMBUS_CHAN, M41T81_CCR_ADDRESS, 
                                       M41T81REG_MO, &byte);
    *month = (UINT8) FROM_BCD(byte);

    status = time_readrtc(M41T81_SMBUS_CHAN, M41T81_CCR_ADDRESS, 
                                       M41T81REG_DT, &byte);
    *date = (UINT8) FROM_BCD(byte);

    status = time_readrtc(M41T81_SMBUS_CHAN, M41T81_CCR_ADDRESS, 
                                       M41T81REG_YR, &byte);
    *year = (UINT8) FROM_BCD(byte);
    if (y2k) {
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
    UINT8 y2k;
    unsigned char  buffer;

    if (!m41t81_exist) {
        return -1;
    }

    /* write time */
    /*
     * M41T81 does not have a century byte, so we don't need to write y2k
     * But we should flip the century bit (CB) to "1" for year 20xx and to "0"
     * for year 19xx.
     */
    y2k = (year >= 2000) ? 0x20 : 0x19;
    buffer = 0;
    if (y2k == 0x20) {
        buffer = M41T81REG_HR_CB;
    }
    buffer |= (unsigned char) (TO_BCD(hour) & 0xff);
    time_writertc(M41T81_SMBUS_CHAN,M41T81_CCR_ADDRESS,
                           M41T81REG_HR, buffer);

    buffer = (char) (TO_BCD(minute) & 0xff);
    time_writertc(M41T81_SMBUS_CHAN,M41T81_CCR_ADDRESS,
                           M41T81REG_MN, buffer);

    buffer  = (unsigned char) (TO_BCD(second) & 0xff);
    buffer &= ~M41T81REG_SC_ST;
    time_writertc(M41T81_SMBUS_CHAN,M41T81_CCR_ADDRESS,
                           M41T81REG_SC, buffer);

    /* write date */
    buffer = (unsigned char) (TO_BCD(month) & 0xff);
    time_writertc(M41T81_SMBUS_CHAN,M41T81_CCR_ADDRESS,
                           M41T81REG_MO, buffer);

    buffer = (unsigned char) (TO_BCD(day) & 0xff);
    time_writertc(M41T81_SMBUS_CHAN,M41T81_CCR_ADDRESS,
                           M41T81REG_DT, buffer);

    year %= 100;
    buffer = (unsigned char) (TO_BCD(year) & 0xff);
    time_writertc(M41T81_SMBUS_CHAN,M41T81_CCR_ADDRESS,
                           M41T81REG_YR, buffer);

    return 0;
}

#ifdef INCLUDE_I2C_DEBUG 
void
m41t81_tod_show() {
    int    year, month, date, hour, minute, second;

    m41t81_tod_init();

    if (m41t81_tod_get(&year, &month, &date, &hour, &minute, &second) == 0) {
        printf("%d/%d/%d %d:%d:%d\n", 
               year, month, date, hour, minute, second);
    } else {
        printf("m41t81_tod_get() failed\n");
    }
}
#endif /* INCLUDE_I2C_DEBUG */ 
