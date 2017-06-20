/*
 * $Id: i2cM41T81Clock.c,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#include "vxWorks.h"
#include "taskLib.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "ctype.h"
#include "config.h"
#include "sysMotI2c.h"
#include "i2cM41T81Clock.h"

/*
 * Get RTC ready
 */
STATUS
m41t81_tod_init(void)
{
    char byte;
    int polls;
    int status;

    /*
     * Reset HT bit to "0" to update registers with current time.
     */
    status = i2cRead(M41T81_SMBUS_CHAN,
                     M41T81_CCR_ADDRESS,
                     I2C_DEVICE_TYPE_RTC_M41T48,
                     M41T81REG_AHR,
                     1, 
                     &byte);
    byte &= ~M41T81REG_AHR_HT;
    status = i2cWrite(M41T81_SMBUS_CHAN,
                     M41T81_CCR_ADDRESS,
                     I2C_DEVICE_TYPE_RTC_M41T48,
                     M41T81REG_AHR,
                     1, 
                     &byte);

    /*
     * Try to read from the device.  If it does not
     * respond, fail.  We may need to do this for up to 300ms.
     */
    for (polls = 0; polls < 300; polls++) {
        taskDelay(1);
        status = i2cRead(M41T81_SMBUS_CHAN,
                     M41T81_CCR_ADDRESS,
                     I2C_DEVICE_TYPE_RTC_M41T48,
                     0,
                     1, 
                     &byte);
        if (status == OK) break;              /* read is ok */
    }

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
    status = i2cRead(M41T81_SMBUS_CHAN,
                     M41T81_CCR_ADDRESS,
                     I2C_DEVICE_TYPE_RTC_M41T48,
                     M41T81REG_SC,
                     1, 
                     &byte);
    if (byte & 0x80) {
        byte &= 0x7f;
        status = i2cWrite(M41T81_SMBUS_CHAN,
                          M41T81_CCR_ADDRESS,
                          I2C_DEVICE_TYPE_RTC_M41T48,
                          M41T81REG_SC,
                          1, 
                          &byte);
    }

    /*
     * Reset HT bit to "0" to update registers with current time.
     */
    status = i2cRead(M41T81_SMBUS_CHAN,
                     M41T81_CCR_ADDRESS,
                     I2C_DEVICE_TYPE_RTC_M41T48,
                     M41T81REG_AHR,
                     1, 
                     &byte);
    if (byte & M41T81REG_AHR_HT) {
        byte &= ~M41T81REG_AHR_HT;
        status = i2cWrite(M41T81_SMBUS_CHAN,
                          M41T81_CCR_ADDRESS,
                          I2C_DEVICE_TYPE_RTC_M41T48,
                          M41T81REG_AHR,
                          1, 
                          &byte);
    }

    /*
     * Try to read from the device.  If it does not
     * respond, fail.  We may need to do this for up to 300ms.
     */
    for (polls = 0; polls < 300; polls++) {
        taskDelay(1);
        status = i2cRead(M41T81_SMBUS_CHAN,
                     M41T81_CCR_ADDRESS,
                     I2C_DEVICE_TYPE_RTC_M41T48,
                     0,
                     1, 
                     &byte);
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
    if (i2cRead(M41T81_SMBUS_CHAN,
                     M41T81_CCR_ADDRESS,
                     I2C_DEVICE_TYPE_RTC_M41T48,
                     M41T81REG_SC,
                     1,
                     &byte) == 0) {
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
    char  byte;
    int   y2k;

    status = i2cRead(M41T81_SMBUS_CHAN,
                     M41T81_CCR_ADDRESS,
                     I2C_DEVICE_TYPE_RTC_M41T48,
                     M41T81REG_HR,
                     1, 
                     &byte);
    y2k = ((byte & M41T81REG_HR_CB) == M41T81REG_HR_CB);
    byte &= ~(M41T81REG_HR_CB | M41T81REG_HR_CEB);
    *hour = (UINT8) FROM_BCD(byte);

    status = i2cRead(M41T81_SMBUS_CHAN,
                     M41T81_CCR_ADDRESS,
                     I2C_DEVICE_TYPE_RTC_M41T48,
                     M41T81REG_MN,
                     1,
                     &byte);
    *minute = (UINT8) FROM_BCD(byte);

    status = i2cRead(M41T81_SMBUS_CHAN,
                     M41T81_CCR_ADDRESS,
                     I2C_DEVICE_TYPE_RTC_M41T48,
                     M41T81REG_SC,
                     1,
                     &byte);
    byte &= ~(M41T81REG_SC_ST);
    *second = (UINT8) FROM_BCD(byte);

    status = i2cRead(M41T81_SMBUS_CHAN,
                     M41T81_CCR_ADDRESS,
                     I2C_DEVICE_TYPE_RTC_M41T48,
                     M41T81REG_MO,
                     1,
                     &byte);
    *month = (UINT8) byte;

    status = i2cRead(M41T81_SMBUS_CHAN,
                     M41T81_CCR_ADDRESS,
                     I2C_DEVICE_TYPE_RTC_M41T48,
                     M41T81REG_DT,
                     1,
                     &byte);
    *date = (UINT8) FROM_BCD(byte);
#if 0
    status = i2cRead(M41T81_SMBUS_CHAN,
                     M41T81_CCR_ADDRESS,
                     I2C_DEVICE_TYPE_RTC_M41T48,
                     M41T81REG_DY,
                     1,
                     &byte);
    *day = (UINT8) byte;
#endif
    status = i2cRead(M41T81_SMBUS_CHAN,
                     M41T81_CCR_ADDRESS,
                     I2C_DEVICE_TYPE_RTC_M41T48,
                     M41T81REG_YR,
                     1,
                     &byte);
    *year = (UINT8) byte;
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
    int   status;
    char  buffer;

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
    buffer |= (char) (TO_BCD(hour) & 0xff);
    status = i2cWrite(M41T81_SMBUS_CHAN,
                   M41T81_CCR_ADDRESS,
                   I2C_DEVICE_TYPE_RTC_M41T48,
                   M41T81REG_HR, 
                   1,
                   &buffer);

    buffer = (char) (TO_BCD(minute) & 0xff);
    status = i2cWrite(M41T81_SMBUS_CHAN,
                   M41T81_CCR_ADDRESS,
                   I2C_DEVICE_TYPE_RTC_M41T48,
                   M41T81REG_MN, 
                   1,
                   &buffer);

    buffer  = (char) (TO_BCD(second) & 0xff);
    buffer &= ~M41T81REG_SC_ST;
    status = i2cWrite(M41T81_SMBUS_CHAN,
                   M41T81_CCR_ADDRESS,
                   I2C_DEVICE_TYPE_RTC_M41T48,
                   M41T81REG_SC,
                   1,
                   &buffer);

    /* write date */
    buffer = (char) (TO_BCD(month) & 0xff);
    status = i2cWrite(M41T81_SMBUS_CHAN,
                   M41T81_CCR_ADDRESS,
                   I2C_DEVICE_TYPE_RTC_M41T48,
                   M41T81REG_MO,
                   1,
                   &buffer);

    buffer = (char) (TO_BCD(day) & 0xff);
    status = i2cWrite(M41T81_SMBUS_CHAN,
                   M41T81_CCR_ADDRESS,
                   I2C_DEVICE_TYPE_RTC_M41T48,
                   M41T81REG_DT,
                   1,
                   &buffer);

    year %= 100;
    buffer = (char) (TO_BCD(year) & 0xff);
    status = i2cWrite(M41T81_SMBUS_CHAN,
                   M41T81_CCR_ADDRESS,
                   I2C_DEVICE_TYPE_RTC_M41T48,
                   M41T81REG_YR,
                   1,
                   &buffer);                  

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
