/*
 * $Id: console.c,v 1.9 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File: 	console.c
 * Purpose:	User TTY I/O
 */

#include <stdarg.h>
#include <sal/core/sync.h>
#include <sal/core/thread.h>
#include <sal/appl/io.h>

#ifdef SAL_NO_TTY
#define	SAL_THREAD_NAME_PRINT_DISABLE
#else
/* These routines are provided by the TTY module */
extern int tty_printf(const char* fmt, ...);
extern int tty_vprintf(const char* fmt, va_list vargs);
#endif


/*
 * sal_console_init
 */

int
sal_console_init(void)
{
    return 0;
}

/*
 * Function:
 *	sal_console_info
 * Purpose:
 *	Get console information
 * Parameters:
 *	info - console info
 * Returns:
 *      0 on success
 *      -1 if stdin is not a tty
 */

int
sal_console_info_get(sal_console_info_t *info)
{
    return -1;
}

/*
 * Function:
 *	sal_console_write
 * Purpose:
 *	Write characters to console
 * Parameters:
 *	buf - buffer to write
 *	count - number of characters in buffer
 */

int
sal_console_write(const void *buf, int count)
{
    return 0;
}

/*
 * Function:
 *	sal_console_read
 * Purpose:
 *	Read characters from console
 * Parameters:
 *	buf - buffer to fill
 *	count - number of characters to read
 */

int
sal_console_read(void *buf, int count)
{
    return 0;
}

/*
 * Function:
 *	sal_console_gets
 * Purpose:
 *	Read line from console
 * Parameters:
 *	buf - input buffer
 *	size - size of input buffer
 */

extern char *tty_gets(char *, int);

char *
sal_console_gets(char *buf, int size)
{
    return tty_gets(buf, size);
}

#ifndef SDK_CONFIG_SAL_VPRINTF
/*
 * sal_vprintf
 *
 *   This is the base routine upon which all standard printing is built.
 */

int sal_vprintf(const char *fmt, va_list varg)
{
    int			retv = 0;
#ifndef	SAL_THREAD_NAME_PRINT_DISABLE
    char		thread_name[SAL_THREAD_NAME_MAX_LEN];
    sal_thread_t	thread;

    thread = sal_thread_self();
    thread_name[0] = 0;

    if (thread != sal_thread_main_get()) {
	sal_thread_name(thread, thread_name, sizeof (thread_name));
    }
#endif	/* !SAL_THREAD_NAME_PRINT_DISABLE */

#ifndef	SAL_THREAD_NAME_PRINT_DISABLE
    if (thread_name[0] != 0) {
	(void) tty_printf("[%s]", thread_name);
    }
#endif	/* !SAL_THREAD_NAME_PRINT_DISABLE */

#ifndef SAL_NO_TTY
    retv = tty_vprintf(fmt, varg);
#endif

    return retv;
}
#endif /* !SDK_CONFIG_SAL_VPRINTF */

int sal_printf(const char *fmt, ...)
{
    int retv;
    va_list varg;
    va_start(varg, fmt);
    retv = sal_vprintf(fmt, varg);
    va_end(varg);
    return retv;
}
