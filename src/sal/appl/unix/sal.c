/*
 * $Id: sal.c,v 1.18 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File: 	sal.c
 * Purpose:	Defines sal routines for user-mode Unix targets.
 */

#include <sys/types.h>
#include <sys/time.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <assert.h>

#include <sal/core/time.h>
#include <sal/core/thread.h>
#include <sal/core/sync.h>
#include <sal/core/boot.h>
#include <sal/core/spl.h>
#include <sal/core/alloc.h>

#include <sal/appl/sal.h>
#include <sal/appl/io.h>
#include <sal/appl/config.h>

#ifdef BROADCOM_DEBUG
/* { */
#ifdef INCLUDE_BCM_SAL_PROFILE
/* { */
#if AGGRESSIVE_ALLOC_DEBUG_TESTING
/* { */
#include <sal/core/alloc.h>
/* } */
#endif
/* } */
#endif
/* } */
#endif /* BROADCOM_DEBUG */

#if defined (__STRICT_ANSI__)
#define RANDOM  rand
#define SRANDOM srand
#else
#define RANDOM  random
#define SRANDOM srandom
#endif

/*
 * Function:
 *	sal_flash_sync
 * Purpose:
 *	For compatibility only.
 * Parameters:
 *	None
 * Returns:
 *	0 - success
 *	-1 - failed
 * Notes:
 *	Not supported for Unix.
 */

int
sal_flash_sync(void)
{
    return(-1);
}

/*
 * Function:
 *	sal_flash_init
 * Purpose:
 *	For compatibility only.
 * Parameters:
 *	None
 * Returns:
 *	0 - success
 *	-1 - failed
 * Notes:
 *	Not supported for Unix.
 */

int
sal_flash_init(int format)
{
    return(-1);
}

/*
 * Function:
 *	sal_flash_boot
 * Purpose:
 *	For compatibility only.
 * Parameters:
 *	None
 * Returns:
 *	0 - success
 *	-1 - failed
 * Notes:
 *	Not supported for Unix.
 */

int
sal_flash_boot(char *f)
{
    return(-1);
}

/*
 * Function:
 *	sal_appl_init
 * Purpose:
 *	Initialize the SAL abstraction layer for Unix
 * Parameters:
 *	None
 * Returns:
 *	0 - success
 *	-1 - failed
 */

int
sal_appl_init(void)
{
#ifndef NO_FILEIO
    char		start_cwd[256];

    sal_getcwd(start_cwd, sizeof (start_cwd));
    sal_homedir_set(start_cwd);
#endif

    sal_console_init();
    sal_config_refresh();

    DISPLAY_MEM ;
    return(0);
}

/*
 * Function:
 *	sal_reboot
 * Purpose:
 *	For compatibility only.
 * Parameters:
 *	None
 * Notes:
 *	Causes the Unix application to exit.
 */

void
sal_reboot(void)
{
    exit(0);
}

/*
 * Function:
 *	sal_shell
 * Purpose:
 *	Fork a standard O/S shell.
 * Parameters:
 *	None
 */

void
sal_shell(void)
{
    char *s = getenv("SHELL");
    /*    coverity[tainted_string]    */
    system(s ? s : "/bin/sh");
}

/*
 * Function:
 *	sal_led
 * Purpose:
 *	For compatibility only.
 * Parameters:
 *	v - pattern to show on LED display
 * Returns:
 *	Previous state of LEDs.
 * Notes:
 *	Not supported for Unix.
 */

uint32
sal_led(uint32 v)
{
    static uint32	led = 0;
    uint32		led_prev;

    led_prev = led;
    led = v;

    return led_prev;
}

/*
 * Function:
 *	sal_led_string
 * Purpose:
 *	For compatibility only.
 * Parameters:
 *	s - string to show on LED display
 * Notes:
 *	Not supported for Unix.
 */

void
sal_led_string(const char *s)
{
    COMPILER_REFERENCE(s);
}

/*
 * Function:
 *	sal_date_set
 * Purpose:
 *	For compatibility only.
 * Parameters:
 *	val - new system time
 * Returns:
 *	0 - success
 *	-1 - failure
 * Notes:
 *	Not supported for Unix.
 */

int
sal_date_set(sal_time_t *val)
{
    sal_printf("Don't know how to set date on this platform (%lu)\n", *val);
    return -1;
}

/*
 * Function:
 *	sal_date_get
 * Purpose:
 *	For compatibility only.
 * Parameters:
 *	None
 * Returns:
 *	0 - success
 *	-1 - failure
 */

int
sal_date_get(sal_time_t *val)
{
    time((time_t *) val);
    return 0;
}

#if !(defined(UNIX) && !defined(__STRICT_ANSI__))
/*
 * Function:
 *	sal_strcasecmp
 * Purpose:
 *	Compare two strings ignoring the case of the characters.
 * Parameters:
 *	s1 - first string to compare
 *	s2 - second string to compare
 * Returns:
 *	0 if s1 and s2 are identical.
 *	negative integer if s1 < s2.
 *	positive integer if s1 > s2.
 * Notes
 *	See man page of strcasecmp for more info.
 */

int
sal_strcasecmp(const char *s1, const char *s2)
{
    unsigned char c1, c2;

    do {
        c1 = tolower(*s1++);
        c2 = tolower(*s2++);
    } while (c1 == c2 && c1 != 0);

    return c1 - c2;
}

/*
 * Function:
 *	sal_strncasecmp
 * Purpose:
 *	Compare two strings ignoring the case of the characters.
 * Parameters:
 *	s1 - first string to compare
 *	s2 - second string to compare
 *	n - maximum number of characters to compare
 * Returns:
 *	0 if s1 and s2 are identical up to n characters.
 *	negative integer if s1 < s2 up to n characters.
 *	positive integer if s1 > s2 up to n characters.
 * Notes
 *	See man page of strncasecmp for more info.
 */

int
sal_strncasecmp(const char *s1, const char *s2, size_t n)
{
    unsigned char c1, c2;

    if (n == 0) {
        return 0;
    }
    do {
        c1 = tolower(*s1++);
        c2 = tolower(*s2++);
    } while (--n && c1 == c2 && c1 != 0);

    return c1 - c2;
}

#endif /* !(defined(UNIX) && !defined(__STRICT_ANSI__)) */
/*
 * Function:
 *  sal_strcasestr
 * Purpose:
 *  Finds the first occurrence of the substring needle in the string haystack
 * Parameters:
 *  haystack - string to be searched in
 *  needle   - substring to be searched for in haystack
 * Returns:
 *  These functions return a pointer to the beginning of the substring
 *  or NULL if the substring is not found.
 * Notes
 *  See man page of strncasestr for more info.
 */

char *
sal_strcasestr(const char *haystack, const char *needle)
{
    char c, sc;
    size_t len;

    if ((c = *needle++) != 0) {
        c = tolower((unsigned char)c);
        len = strlen(needle);
        do {
            do {
                if ((sc = *haystack++) == 0)
                    return (NULL);
            } while ((char)tolower((unsigned char)sc) != c);
        } while (sal_strncasecmp(haystack, needle, len) != 0);
        haystack--;
    }
    return ((char *)haystack);
}

/* 
 * Function     : sal_i2c_read
 * Purpose      : Provide i2c bus read functionality using CPU i2c controller
 * Parameters   :   unit        - I2C controller 
 *                  slave       - slave address on the I2C bus
 *                  addr        - internal address on the slave. 
 *                  addr_len    - length of internal address 
 *                  buf         - buffer to hold the read data
 *                  buf_len     - length of data to read
 * Returns      : 0 - success, -1 - failure
 */
int
sal_i2c_read(int unit, uint16 slave, uint32 addr, uint8 addr_len, uint8 *buf, 
             uint8 buf_len)
{
    return(-1); /* Not supported */
}

/* 
 * Function     : sal_i2c_write
 * Purpose      : Provide i2c bus write functionality using CPU i2c controller    
 * Parameters   :   unit    - I2C controller 
 *                  slave   - slave address on the I2C bus
 *                  addr    - internal address on the slave. 
 *                  addr_len    - length of internal address 
 *                  buf         - buffer to hold the write data
 *                  buf_len     - length of data to write
 * Returns      : 0 - success, -1 - failure
 */
int
sal_i2c_write(int unit, uint16 slave, uint32 addr, uint8 addr_len, uint8 *buf, 
              uint8 buf_len)
{
    return(-1); /* Not supported */
}

/* 
 * Function     : sal_i2c_config_get 
 * Purpose      : Retrieve configurable i2c bus properties 
 * unit         : I2C controller 
 * flags        : SAL_I2C_* flags 
 */
int
sal_i2c_config_get(int unit, uint32 *flags)
{
    return(-1); /* Not supported */
}

/* 
 * Function     : sal_i2c_config_set 
 * Purpose      : Configure i2c bus properties 
 * unit         : I2C controller 
 * flags        : SAL_I2C_* flags 
 */
int
sal_i2c_config_set(int unit, uint32 flags)
{
    return(-1); /* Not supported */
}
