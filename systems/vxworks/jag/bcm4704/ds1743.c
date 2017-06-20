/* $Id: ds1743.c,v 1.3 2011/07/21 16:14:21 yshtil Exp $
 *
 * DS1743 TOD Driver
 *
 * The DS1743 is an 8K NVRAM starting at offset DS1743_BASE_ADDR and
 * continuing for 8184 bytes. After that starts the Time-Of-Day (TOD)
 * registers which are used to set/get the internal date/time functions.
 *
 * This module implements Y2K compliance by taking full year numbers
 * and translating back and forth from the TOD 2-digit year.
 *
 * NOTE: for proper interaction with an operating system, the TOD should
 * be used to store Universal Coordinated Time (GMT) and timezone
 * conversions should be used.
 *
 * Here is a diagram of the memory layout:
 *
 * +---------------------------------------------+ BASE
 * | Non-volatile memory                         | .
 * |                                             | .
 * | (8184 bytes of Non-volatile memory)         | .
 * |                                             | .
 * +---------------------------------------------+ BASE + 8K - 8
 * | Control                                     |
 * +---------------------------------------------+ BASE + 8K - 7
 * | Seconds                                     |
 * +---------------------------------------------+ BASE + 8K - 6
 * | Minutes                                     |
 * +---------------------------------------------+ BASE + 8K - 5
 * | Hours                                       |
 * +---------------------------------------------+ BASE + 8k - 4
 * | Day                                         |
 * +---------------------------------------------+ BASE + 8K - 3
 * | Date                                        |
 * +---------------------------------------------+ BASE + 8K - 2
 * | Month                                       |
 * +---------------------------------------------+ BASE + 8K - 1
 * | Year                                        |
 * +---------------------------------------------+ 
 */

#include "vxWorks.h"
#include "mbz.h"
#include "ds1743.h"
#ifdef  MIPSEL
#define ADDR_XOR_3 0
#else
#define ADDR_XOR_3 3
#endif

#define YEAR		(0x7 ^ ADDR_XOR_3)
#define MONTH		(0x6 ^ ADDR_XOR_3)
#define DAY		(0x5 ^ ADDR_XOR_3)
#define DAY_OF_WEEK	(0x4 ^ ADDR_XOR_3)
#define HOUR		(0x3 ^ ADDR_XOR_3)
#define MINUTE		(0x2 ^ ADDR_XOR_3)
#define SECOND		(0x1 ^ ADDR_XOR_3)
#define CONTROL		(0x0 ^ ADDR_XOR_3)

static UINT8 *ds1743BaseAddr;

/* FIXME */
#define DS1743_ADDR	((volatile UINT8 *) (ds1743BaseAddr + 8*1024 - 8))

STATUS ds1743_tod_init(UINT8 *addr)
{

  ds1743BaseAddr = addr;

  SYS_TOD_UNPROTECT();
  DS1743_ADDR[CONTROL] = 0;
  SYS_TOD_PROTECT();

  /* Start the clock if the oscillator is not running */
  if(DS1743_ADDR[SECOND] & 0x80)
    return ds1743_tod_set(2003, 1, 1, 0, 0, 0);
  else
    return 0;
}

/*
 * ds1743_tod_set
 */

static int to_bcd(int value)
{
  return value / 10 * 16 + value % 10;
}

static int from_bcd(int value)
{
  return value / 16 * 10 + value % 16;
}

static int day_of_week(int y, int m, int d)	/* 0-6 ==> Sun-Sat */
{       
  static int t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4}; 
  y -= m < 3;
  return (y + y/4 - y/100 + y/400 + t[m-1] + d) % 7;
}       

/*
 * Note: the TOD should store the current GMT
 */

STATUS ds1743_tod_set(int year,		/* 1980-2079 */
		      int month,		/* 01-12 */
		      int day,		/* 01-31 */
		      int hour,		/* 00-23 */
		      int minute,		/* 00-59 */
		      int second)		/* 00-59 */
     
{

  SYS_TOD_UNPROTECT();
  DS1743_ADDR[CONTROL] |= 0x80;	/* Set WRITE bit */

  DS1743_ADDR[YEAR] = to_bcd(year % 100);
  DS1743_ADDR[MONTH] = to_bcd(month);
  DS1743_ADDR[DAY] = to_bcd(day);
  DS1743_ADDR[DAY_OF_WEEK] = day_of_week(year, month, day) + 1;
  DS1743_ADDR[HOUR] = to_bcd(hour);
  DS1743_ADDR[MINUTE] = to_bcd(minute);
  DS1743_ADDR[SECOND] = to_bcd(second);

  DS1743_ADDR[CONTROL] &= ~0x80;	/* Clear WRITE bit */
  SYS_TOD_PROTECT();
  return OK;
}

/*
 * Note: the TOD should store the current GMT
 */

STATUS ds1743_tod_get(int *year,		/* 1980-2079 */
		      int *month,		/* 01-12 */
		      int *day,		        /* 01-31 */
		      int *hour,		/* 00-23 */
		      int *minute,		/* 00-59 */
		      int *second)		/* 00-59 */
{
  int y;
  
  SYS_TOD_UNPROTECT();
  DS1743_ADDR[CONTROL] |= 0x40;	/* Set READ bit */
  SYS_TOD_PROTECT();

  y = from_bcd(DS1743_ADDR[YEAR]);
  *year = y < 80 ? 2000 + y : 1900 + y;
  *month = from_bcd(DS1743_ADDR[MONTH]);
  *day = from_bcd(DS1743_ADDR[DAY]);
  /* 
   * Note: MSB of day-of-week reg contains battery-life bit.  It
   * should always be '1'.  If it's zero, either the battery is dead
   * or we're not really reading the chip.
   */
  /* day_of_week = DS1743_ADDR[DAY_OF_WEEK] & 0xf; */
  *hour = from_bcd(DS1743_ADDR[HOUR]);
  *minute = from_bcd(DS1743_ADDR[MINUTE]);
  *second = from_bcd(DS1743_ADDR[SECOND] & 0x7f);
  
  DS1743_ADDR[CONTROL] &= ~0x40;	/* Clear READ bit */
  
  return OK;
}

int ds1743_tod_get_second(void)
{
    return from_bcd(DS1743_ADDR[SECOND] & 0x7f);
}


STATUS
ds1743_tod_print()
{
  int year, month, day, hour, minute, second;
  ds1743_tod_get(&year,		/* 1980-2079 */
		 &month,		/* 01-12 */
		 &day,		        /* 01-31 */
		 &hour,		/* 00-23 */
		 &minute,		/* 00-59 */
		 &second);		/* 00-59 */  
  
  printf("%d/%d/%d, %d:%.2d:%.2d\n", 
	 month, day, year, 
	 hour, minute, second);
  return 0;
}
