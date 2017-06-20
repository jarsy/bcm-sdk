/*
 * $Id: i2cM41T81Clock.h,v 1.6 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#ifndef _GTO_M41_T81_Y_H
#define _GTO_M41_T81_Y_H

/*
 * The clock is connected to SMBus channel 1
 */
#define M41T81_SMBUS_CHAN       0
#define M41T81_CCR_ADDRESS      0x68

/*
 * Register bits
 */
#define M41T81REG_SC_ST     0x80  /* stop bit */
#define M41T81REG_HR_CB     0x40  /* century bit */
#define M41T81REG_HR_CEB    0x80  /* century enable bit */
#define M41T81REG_CTL_S     0x20  /* sign bit */
#define M41T81REG_CTL_FT    0x40  /* frequency test bit */
#define M41T81REG_CTL_OUT   0x80  /* output level */
#define M41T81REG_WD_RB0    0x01  /* watchdog resolution bit 0 */
#define M41T81REG_WD_RB1    0x02  /* watchdog resolution bit 1 */
#define M41T81REG_WD_BMB0   0x04  /* watchdog multiplier bit 0 */
#define M41T81REG_WD_BMB1   0x08  /* watchdog multiplier bit 1 */
#define M41T81REG_WD_BMB2   0x10  /* watchdog multiplier bit 2 */
#define M41T81REG_WD_BMB3   0x20  /* watchdog multiplier bit 3 */
#define M41T81REG_WD_BMB4   0x40  /* watchdog multiplier bit 4 */
#define M41T81REG_AMO_ABE   0x20  /* alarm in "battery back-up mode" enable bit */
#define M41T81REG_AMO_SQWE  0x40  /* square wave enable */
#define M41T81REG_AMO_AFE   0x80  /* alarm flag enable flag */
#define M41T81REG_ADT_RPT5  0x40  /* alarm repeat mode bit 5 */
#define M41T81REG_ADT_RPT4  0x80  /* alarm repeat mode bit 4 */
#define M41T81REG_AHR_RPT3  0x80  /* alarm repeat mode bit 3 */
#define M41T81REG_AHR_HT    0x40  /* halt update bit */
#define M41T81REG_AMN_RPT2  0x80  /* alarm repeat mode bit 2 */
#define M41T81REG_ASC_RPT1  0x80  /* alarm repeat mode bit 1 */
#define M41T81REG_FLG_AF    0x40  /* alarm flag (read only) */
#define M41T81REG_FLG_WDF   0x80  /* watchdog flag (read only) */
#define M41T81REG_SQW_RS0   0x10  /* sqw frequency bit 0 */
#define M41T81REG_SQW_RS1   0x20  /* sqw frequency bit 1 */
#define M41T81REG_SQW_RS2   0x40  /* sqw frequency bit 2 */
#define M41T81REG_SQW_RS3   0x80  /* sqw frequency bit 3 */

/*
 * Register numbers
 */
#define M41T81REG_TSC       0x00  /* tenths/hundredths of second */
#define M41T81REG_SC        0x01  /* seconds */
#define M41T81REG_MN        0x02  /* minute */
#define M41T81REG_HR        0x03  /* hour/century */
#define M41T81REG_DY        0x04  /* day of week */
#define M41T81REG_DT        0x05  /* date of month */
#define M41T81REG_MO        0x06  /* month */
#define M41T81REG_YR        0x07  /* year */
#define M41T81REG_CTL       0x08  /* control */
#define M41T81REG_WD        0x09  /* watchdog */
#define M41T81REG_AMO       0x0A  /* alarm: month */
#define M41T81REG_ADT       0x0B  /* alarm: date */
#define M41T81REG_AHR       0x0C  /* alarm: hour */
#define M41T81REG_AMN       0x0D  /* alarm: minute */
#define M41T81REG_ASC       0x0E  /* alarm: second */
#define M41T81REG_FLG       0x0F  /* flags */
#define M41T81REG_SQW       0x13  /* square wave register */

#define TO_BCD(x) (((x) % 10) + (((x) / 10) << 4))
#define FROM_BCD(x) ((x) / 16 * 10 + (x) % 16)

STATUS m41t81_tod_init(void);
int m41t81_tod_kick_start(void);
STATUS m41t81_tod_set(int year, int month, int day,
		   int hour, int minute, int second);
STATUS m41t81_tod_get(int *year, int *month, int *day, 
		   int *hour, int *minute, int *second);
STATUS m41t81_tod_get_second(void);

#endif /* _GTO_M41_T81_Y_H */

