/*
 * $Id: sal.c,v 1.63 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File: 	sal.c
 * Purpose:	Defines sal routines for VxWorks targets.
 */

#include <shared/bsl.h>

#include <sys/types.h>
#if VX_VERSION == 69
#include <types/vxWind.h>
#endif
#include <signal.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <assert.h>

#define __PROTOTYPE_5_0		/* Get stdarg prototypes for logMsg */
#include <vxWorks.h>
#include <cacheLib.h>
#include <taskLib.h>
#include <bootLib.h>
#include <intLib.h>
#include <netLib.h>
#include <logLib.h>
#include <sysLib.h>
#include <usrLib.h>
#include <vmLib.h>
#include <vxLib.h>
#include <ioLib.h>
#include <sockLib.h>
#include <tyLib.h>
#include <shellLib.h>
#if (_WRS_VXWORKS_MAJOR >= 6)
#include <shellInterpLib.h>
#include <excLib.h>
#endif /* _WRS_VXWORKS_MAJOR >= 6 */
#include <selectLib.h>
#include <dosFsLib.h>


#include <sal/core/spl.h>
#include <sal/core/boot.h>
#include <sal/core/time.h>
#include <sal/core/thread.h>
#include <sal/core/sync.h>
#include <sal/core/alloc.h>

#include <sal/appl/sal.h>
#include <sal/appl/io.h>
#include <sal/appl/config.h>
#include <sal/appl/i2c.h>
#include <sal/appl/vxworks/hal.h>

#ifdef SAL_NO_FLASH_FS
#else
#include "srecLoad.h"
#endif

#if defined(DMA_BUFFER_ALIGNMENT)
#define SAL_DMA_BUF_ALIGNMENT DMA_BUFFER_ALIGNMENT
#else
#define SAL_DMA_BUF_ALIGNMENT   8
#endif /* DMA_BUFFER_ALIGNMENT */

#define SAL_ALIGN(p, a)     ((((uint32)(p)) + (a) - 1) & ~((a) - 1))

platform_hal_t *platform_info;

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
	c1 = *s1++;
        c1 = tolower(c1);
	c2 = *s2++;
        c2 = tolower(c2);
    } while (c1 == c2 && c1 != 0);

    return (int)c1 - (int)c2;
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

    do {
	if (n-- < 1) {
	    return 0;
	}
	c1 = *s1++;
        c1 = tolower(c1);
	c2 = *s2++;
        c2 = tolower(c2);
    } while (c1 == c2 && c1 != 0);

    return (int)c1 - (int)c2;
}

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
 * Function:
 *	sal_date_set
 * Purpose:
 *	Set system time/date
 * Parameters:
 *	val - new system time/date
 * Returns:
 *	0 - success
 *	-1 - failure
 */

int
sal_date_set(time_t *val)
{
    struct tm		*tm;
    struct timespec 	tp;
    int			sts;

    if (!SAL_IS_PLATFORM_INFO_VALID) {
        cli_out("Platform not attached\n");
        return -1;
    }

    if (platform_info->f_tod_set == NULL) {
        cli_out("Don't know how to set date on this platform\n");
        return -1;
    }

    tp.tv_sec  = *val;
    tp.tv_nsec = 0;

    clock_settime(CLOCK_REALTIME, &tp);

    tm = gmtime(val);

    tm->tm_year += 1900;
    tm->tm_mon += 1;

    sts = platform_info->f_tod_set(tm->tm_year,	/* 1980-2079 */
                                   tm->tm_mon,	/* 01-12 */
                                   tm->tm_mday,	/* 01-31 */
                                   tm->tm_hour,	/* 00-23 */
                                   tm->tm_min,	/* 00-59 */
                                   tm->tm_sec);	/* 00-59 */

    return (sts == ERROR) ? -1 : 0;
}

/*
 * Function:
 *	sal_date_get
 * Purpose:
 *	Get system date/time
 * Parameters:
 *	None
 * Returns:
 *	0 - success
 *	-1 - failure
 */

int
sal_date_get(time_t *val)
{
    time(val);
    return OK;
}

/*
 * Internal functions for VxWorks Control-C handling
 */

static void
abortFuncJob(WIND_TCB *taskId)
{
    int		tid;

    /*
     * This runs in the context of some task manager thread and sends a
     * signal back to the shell.
     */

    taskId->excInfo.valid = 0;

    tid = PTR_TO_INT(sal_thread_main_get());

    if (tid != 0) {
	kill(tid, SIGINT);
    }
}

static int
abortFuncHandler(void)
{
    /*
     * This runs at INTERRUPT level where we can't do much at all except
     * call the undocumented excJobAdd to schedule a function to be
     * called by some task manager thread.
     */

    excJobAdd(abortFuncJob, PTR_TO_INT(taskIdCurrent), 0, 0, 0, 0, 0);

    return 0;
}

int
abortFuncDefault(void)
{
    /*
     * There is no official way to get the default abort function from
     * vxWorks, so we'll try this:
     */

    ioctl(ioTaskStdGet(0,STD_IN), FIORFLUSH, 0);

#if (_WRS_VXWORKS_MAJOR >= 6)
    excJobAdd((VOIDFUNCPTR)shellRestart, 
              (int)ALL_SHELL_SESSIONS, 0, 0, 0, 0, 0);
#else
    excJobAdd((VOIDFUNCPTR)shellRestart, 
              TRUE, 0, 0, 0, 0, 0);
#endif

    return 0;
}

static void
ttyRawModeSet(int raw)
{
#ifdef INCLUDE_EDITLINE
    static int old, new;

    if (raw) {
	old = ioctl(ioTaskStdGet(0,STD_IN), FIOGETOPTIONS, 0);
	new = old & ~(OPT_LINE | OPT_ECHO);
	ioctl(ioTaskStdGet(0,STD_IN), FIOSETOPTIONS, new);
    } else {
	ioctl(ioTaskStdGet(0,STD_IN), FIOSETOPTIONS, old);
    }
#endif /* INCLUDE_EDITLINE */
}

void
sal_readline_init(void)
{
    ttyRawModeSet(1);
}

void
sal_readline_term(void)
{
    ttyRawModeSet(0);
}


/*
 * MIPS cache check (MIPS only)
 *
 *   May be useful while bringing up a MIPS BSP.
 *   Define it to 1 to get diagnostic output during boot-up.
 */

#define MIPS_CACHE_SANITY	0

#if MIPS_CACHE_SANITY

STATIC int
_sal_mips_cache_test(char *pfx, uint32 *buf, char *allocator)
{
    volatile uint32	*buf_c, *buf_u;
    int			t, rv = 0;

    cli_out("%sTesting %s\n", pfx, allocator);

    if (buf == NULL) {
	cli_out("%s==> %s returns NULL\n", pfx, allocator);
	return -1;
    }

    if (((uint32) buf & 0xe0000000) == 0xa0000000) {
	cli_out("%s==> %s returns uncached memory\n", pfx, allocator);
	return 0;
    }

    if (((uint32) buf & 0xe0000000) != 0x80000000) {
	cli_out("%s==> ERROR: %s returns unknown memory\n", pfx, allocator);
	return -1;
    }

    buf_c = (uint32 *) buf;
    buf_u = (uint32 *) (((uint32) buf & ~0xe0000000) | 0xa0000000);

    cli_out("%s==> buffer: cached=0x%08x uncached=0x%08x\n",
	   pfx, (uint32) buf_c, (uint32) buf_u);

    buf_c[0] = 0x44444444;
    buf_u[0] = 0xcccccccc;

    if (buf_c[0] != 0x44444444) {
	cli_out("%s==> ERROR: cache does not retain data\n", pfx);
	rv = -1;
    }

    buf_u[0] = 0x55555555;
    buf_c[0] = 0xaaaaaaaa;

    if (buf_u[0] != 0x55555555) {
	cli_out("%s==> write-through cache\n", pfx);
    } else {
	sal_dma_flush((void *) buf_c, 40);

	t = (buf_u[0] == 0x55555555);

	cli_out("%s==> write-back cache\n", pfx);

	if (t) {
	    cli_out("%s==> ERROR: cache flush ineffective\n", pfx);
	    rv = -1;
	}
    }

    buf_c[0] = 0x77777777;
    sal_dma_flush((void *) buf_c, 40);
    buf_u[0] = 0x88888888;

    if (buf_c[0] != 0x77777777) {
	cli_out("%s==> cache flush also invalidates\n", pfx);
    } else {
	cli_out("%s==> cache flush does not invalidate\n", pfx);
    }

    buf_c[0] = 0x77777777;
    sal_dma_inval((void *) buf_c, 40);
    buf_u[0] = 0x88888888;

    if (buf_c[0] != 0x88888888) {
	cli_out("%s==> ERROR: invalidation ineffective\n", pfx);
	rv = -1;
    }

    buf_c[0] = 0x77777777;
    buf_u[0] = 0x88888888;
    sal_dma_inval((void *) buf_c, 40);

    if (buf_u[0] == 0x77777777) {
	cli_out("%s==> ERROR: invalidation writes back\n", pfx);
	rv = -1;
    }

    return rv;
}

STATIC int
_sal_mips_cache_sanity(char *pfx)
{
    uint32		*buf;
    int			rv = 0;

    buf = sal_alloc(256, "ctest1");

    if (_sal_mips_cache_test(pfx, buf, "sal_alloc") < 0) {
	rv = -1;
    }

    if (buf != NULL) {
	sal_free(buf);
    }

    buf = sal_dma_alloc(256, "ctest2");

    if (_sal_mips_cache_test(pfx, buf, "sal_dma_alloc") < 0) {
	rv = -1;
    }

    if (buf != NULL) {
	sal_dma_free(buf);
    }

    return rv;
}

#endif /* MIPS_CACHE_SANITY */

/*
 * Function:
 *	sal_dma_alloc_init (internal)
 * Purpose:
 *	Initialize the VxWorks cacheFuncs
 * Parameters:
 *	None
 */

static CACHE_FUNCS sal_cacheFuncs;

static int
sal_dma_alloc_init(void)
{
#if MIPS_CACHE_SANITY
    cli_out("Running cache sanity with cacheNullFuncs\n");
    sal_cacheFuncs = cacheNullFuncs;
    _sal_mips_cache_sanity("    ");
    cli_out("\nRunning cache sanity with cacheDmaFuncs\n");
    sal_cacheFuncs = cacheDmaFuncs;
    _sal_mips_cache_sanity("    ");
#endif /* MIPS_CACHE_SANITY */

    if (platform_info->caps & PLATFORM_CAP_DMA_MEM_UNCACHABLE) {
        sal_cacheFuncs = cacheNullFuncs;
    } else {
        sal_cacheFuncs = cacheDmaFuncs;
    }
    return 0;
}

/*
 * Function:
 *	sal_dma_flush
 * Purpose:
 *	Flush/writeback a region of memory from the cache
 * Parameters:
 *	addr - memory region address
 *	len - memory region length
 * Parameters:
 *	None
 */

void
sal_dma_flush(void *addr, int len)
{
    CACHE_DRV_FLUSH(&sal_cacheFuncs, addr, len);
}

/*
 * Function:
 *	sal_dma_inval
 * Purpose:
 *	Flush/writeback/invalidate a region of memory from the cache
 * Parameters:
 *	addr - memory region address
 *	len - memory region length
 * Parameters:
 *	None
 */

void 
sal_dma_inval(void *addr, int len)
{
    CACHE_DRV_INVALIDATE(&sal_cacheFuncs, addr, len);
}

/*
 * Function:
 *	sal_dma_vtop
 * Purpose:
 *	Convert an address from virtual to physical
 * Parameters:
 *	addr - Virtual memory address
 * Returns:
 *	Physical memory address
 */

void *
sal_dma_vtop(void *addr)
{    
    return CACHE_DRV_VIRT_TO_PHYS(&sal_cacheFuncs, addr);
}

/*
 * Function:
 *	sal_dma_ptov
 * Purpose:
 *	Convert an address from physical to virtual
 * Parameters:
 *	addr - Physical memory address
 * Returns:
 *	Virtual memory address
 */

void *
sal_dma_ptov(void *addr)
{
    return CACHE_DRV_PHYS_TO_VIRT(&sal_cacheFuncs, addr);
}

/*
 * Function:
 *	sal_appl_init
 * Purpose:
 *	Initialize the SAL abstraction layer for VxWorks.
 * Parameters:
 *	None
 * Returns:
 *	0 - success
 *	-1 - failed
 */

extern void (*sysToMonitorHook)(void);
extern int sysToMonitorExcMessage;
extern int sysToMonitorBacktrace;
extern int sysToMonitorReboot;
int
sal_appl_init(void)
{
    static	int	inited = FALSE;

    if (inited) {
	return(0);
    }
    inited = TRUE;

    sal_console_init();

    platform_attach(&platform_info);

    if (!SAL_IS_PLATFORM_INFO_VALID) {
        cli_out("Platform not attached\n");
        return -1;
    }

    if (platform_info->f_dump_info) {
        platform_info->f_dump_info();
    }

    sal_dma_alloc_init();


    if (sal_boot_flags_get() & BOOT_F_SHELL_ON_TRAP) {
        sysToMonitorHook = sal_shell;
    } else {
        sysToMonitorExcMessage = 0;
        sysToMonitorBacktrace = 0;
        sysToMonitorReboot = 1;
    }

#ifndef NO_CONTROL_C
    tyAbortFuncSet(abortFuncHandler);
#endif

    sal_readline_init();

    /*
     * Initialize flash device if present, ignore errors since it may
     * have been initialized if we booted from it, and/or may not be present.
     */
    (void)sal_flash_init(FALSE);

#ifndef NO_FILEIO
    /*
     * Set home directory to default.
     * Change directory to default.
     */
    (void)sal_homedir_set(NULL);
    (void)sal_cd(NULL);
#endif

#ifdef SAL_CONFIG_FILE_FLASH
    /*
     * Initialize simple configuration database from current directory.
     */
    if (sal_config_refresh() < 0) {
        sal_config_file_set(SAL_CONFIG_FILE_FLASH, SAL_CONFIG_TEMP_FLASH);
        /*
         * Initialize simple configuration database after flash has 
         * been initialized.
         */
        (void)sal_config_refresh();
    }
#else

    /*
     * Initialize simple configuration database after flash has 
     * been initialized.
     */
    (void)sal_config_refresh();
#endif

    return(0);
}

/*
 * Function:
 *	sal_reboot
 * Purpose:
 *	Reboot the system.
 * Parameters:
 *	None
 */

void
sal_reboot(void)
{
    cli_out("Resetting...");
    sal_usleep(SECOND_USEC);	/* Time for messages to flush on serial */
    cli_out("\n");
    sal_usleep(SECOND_USEC / 10);	/* Time for CRLF to flush */

    /* Close open network connections */
    close(ioTaskStdGet(0,STD_IN));
    close(ioTaskStdGet(0,STD_OUT));
    close(ioTaskStdGet(0,STD_ERR));

    sysReboot();
}

/*
 * Function:
 *	sal_shell
 * Purpose:
 *	For compatibility only.
 * Parameters:
 *	None
 * Notes:
 *	Not supported for Linux kernel mode.
 */

int sal_telnet_active;		

extern STATUS dbgShellCmdInit (void);
extern STATUS taskShellCmdInit (void);
extern void usrShell (void);
    
void
sal_shell(void)
{
    /*
     * A shell task is forked.  Wait until shell exits before returning.
     * Otherwise, socdiag and the shell fight over TTY input.
     */
#ifndef NO_CONTROL_C
    tyAbortFuncSet(abortFuncDefault);
#endif
    sal_readline_term();			/* Leave raw mode */

#if (_WRS_VXWORKS_MAJOR >= 6)
    shellGenericInit(NULL,     /* configuration string or NULL */
                     0,        /* shell stack (0 = default value) */
                     "tShell", /* shell task name or NULL for def. base name */
                     NULL,     /* pointer on the shell task name or NULL */
                     TRUE,     /* interactive mode if TRUE */
                     FALSE,    /* login access */
                     ioTaskStdGet(0, STD_IN),  /* input file descriptor */
                     ioTaskStdGet(0, STD_OUT), /* output file descriptor */
                     ioTaskStdGet(0, STD_ERR)); /* error file descriptor */
#else 
    shellInit(10000 /* Stack size */, TRUE);	/* create the shell */    
#endif 

    if (sal_telnet_active) {
	/* Redirect shell stdio to task specific descriptors */
    #if VX_VERSION == 55 || VX_VERSION == 542
	shellOrigStdSet (STD_IN, ioTaskStdGet(0,STD_IN));
	shellOrigStdSet (STD_OUT, ioTaskStdGet(0,STD_OUT));
	shellOrigStdSet (STD_ERR, ioTaskStdGet(0,STD_ERR));
    #endif
	shellLock(FALSE); /* Unlock shell */
#if (_WRS_VXWORKS_MAJOR >= 6)
        excJobAdd((VOIDFUNCPTR)shellRestart,
                  (int)CURRENT_SHELL_SESSION , 0, 0, 0, 0, 0);
#else
        excJobAdd(shellRestart, FALSE, 0, 0, 0, 0, 0);
#endif
    }

    do {
	sal_usleep(100000);
    } while (taskNameToId("tShell") != ERROR);

    sal_readline_init();			/* Back into raw mode */

#ifndef NO_CONTROL_C
    tyAbortFuncSet(abortFuncHandler);
#endif
}

/*
 * Function:
 *	sal_flash_init
 * Purpose:
 *	Mounts the VxWorks flash filesystem.
 * Parameters:
 *	format - If TRUE, also erases and formats the flash disk.
 * Returns:
 *	0 - success
 *	-1 - failed
 */

int
sal_flash_init(int format)
{

    if (!platform_info || (platform_info->f_format_fs == NULL)) {
        cli_out("Platform not attached Or Flash Format Not allowed.\n");
        return -1;
    }

    return platform_info->f_format_fs(format, FLASH_FORMAT_DOS);
}

static int sal_image_loader(char *fname, char *buf, 
                            int bufSize, int *entry)
{
#ifdef SAL_NO_FLASH_FS
    return -1;
#else
    FILE		*fp = 0;
    int			rv = -1;
    int			i;
    int			sRecords;

    if (strlen(fname) < 4) {
    bad_fname:
	cli_out("flashBoot: Illegal file %s, must end in .bin .img or .hex\n",
                fname);
	goto done;
    }

    if (!strcmp(fname + strlen(fname) - 4, ".img") ||
        !strcmp(fname + strlen(fname) - 4, ".bin")) {
	sRecords = 0;
    } else if (!strcmp(fname + strlen(fname) - 4, ".hex")) {
	sRecords = 1;
    } else {
	goto bad_fname;		/* Above */
    }

    printf("Loading %s ... ", fname);

    if ((fp = sal_fopen(fname, "r")) == 0) {
	cli_out("\nflashBoot: could not open file %s\n", fname);
	goto done;
    }

    if (sRecords) {
	if ((i = srecLoad(fp, buf, bufSize, entry)) < 0) {
	    cli_out("\nflashBoot: failed reading S-record file %s: %s\n",
                    fname, srecErrmsg(i));
	    goto done;
	}
    } else if ((i = fread(buf, 1, bufSize, fp)) != bufSize) {
	cli_out("\nflashBoot: failed reading 1024 kB from binary file %s\n",
                fname);
	goto done;
    }

done:

    if (fp) {
	fclose(fp);
    }

    return rv;
#endif /* SAL_NO_FLASH_FS */
}

/*
 * Function:
 *	sal_flash_boot
 * Purpose:
 *	Flash the boot area of the flash with an image from a file.
 * Parameters:
 *	file - name of file with image.
 * Returns:
 *	0 - success
 *	-1 - failed
 */

int
sal_flash_boot(char *file)
{
    if (!SAL_IS_PLATFORM_INFO_VALID) {
        cli_out("Platform not attached\n");
        return -1;
    }
    if (platform_info->f_upgrade_image) {
        return platform_info->f_upgrade_image(file, 
                            IMAGE_F_BOOT, sal_image_loader);
    }
    return(-1);
}

/*
 * Function:
 *	sal_led
 * Purpose:
 *	Display a pattern on the system LEDs.
 * Parameters:
 *	v - pattern to show on system LEDs.
 * Returns:
 *	Previous state of LEDs.
 */

uint32
sal_led(uint32 v)
{
#if defined(SYS_LED_ON) && defined(SYS_LED_OFF)
    static uint32	led = 0;
    uint32		led_prev;
    int			lv;

    led_prev = led;
    led = v;

    lv = intLock();

    led ? SYS_LED_ON() : SYS_LED_OFF();

    intUnlock(lv);

    return led_prev;
#else
    return 0;
#endif /* SYS_LED_ON && SYS_LED_OFF */
}

/*
 * Function:
 *	sal_led_string
 * Purpose:
 *	For compatibility only.
 * Parameters:
 *	s - string to show on LED display
 * Notes:
 *	Not supported for Linux kernel mode.
 */

void
sal_led_string(const char *s)
{
    COMPILER_REFERENCE(s);


    if (platform_info->f_led_write_string) {
        platform_info->f_led_write_string(s);
    }
}

/*
 * Function:
 *	sal_watchdog_arm
 * Purpose:
 *	Set up watch dog reset function.
 * Parameters:
 *	usec - 0 to disarm watchdog.
 *		non-zero to set up watchdog time in microseconds.
 * Returns:
 *	0 - success
 *	-1 - failure
 * Notes:
 *	If usec is non-zero, the watchdog timer is armed (or re-armed)
 *	for approximately usec microseconds (if the exact requested
 *	usec is not supported, the next higher available value is used).
 *
 *	If the timer is armed and expires, the system is reset.
 */

int
sal_watchdog_arm(uint32 usec)
{
    COMPILER_REFERENCE(usec);

    if (!platform_info->f_start_wdog) {
        cli_out("No Watchdog support on platform. \n");
        return -1;
    }
    return platform_info->f_start_wdog(usec);
}

int 
sal_memory_check(uint32 addr)
{
    

#if defined(SAL_SKIP_MEM_CHECK)
#else

#ifndef VX_NO_MEM_PROBE
    STATUS probeStatus;
#endif

    /* Sanity check that we can do DMA ... */

    if (!CACHE_DMA_IS_WRITE_COHERENT()) {
	cli_out("sal_memory_check: device requires cache coherent memory\n");
	return -1;
    }

    LOG_VERBOSE(BSL_LS_SOC_PCI,
                (BSL_META("System Memory Top: 0x%x\n"),
                 PTR_TO_INT(sysMemTop())));

#ifndef VX_NO_MEM_PROBE		/* Make.local option */
    /*
     * Test physical memory for sanity's sake. Just see if we can read
     * below the end of DRAM.
     */

    if (vxMemProbe((char *)sysMemTop() - 24,
		   VX_READ, 4, (char*)&probeStatus) != OK) {
	cli_out("sal_memory_check: can't read last word in memory!\n");
	return -1;
    }

    /*
     * Now, test that you can read/write the memory and that it does not
     * generate a bus error.
     */

    if ((vxMemProbe((char *)addr,
		    VX_READ, 4, (char*)&probeStatus) != OK) ||
	(vxMemProbe((char *)addr,
		    VX_WRITE, 4, (char*)&probeStatus) != OK)) {
	cli_out("sal_memory_check: vxMemProbe R/W error (addr=0x%x)\n",
                addr);
	return -1;
    }

#endif /* VX_NO_MEM_PROBE */

#endif /* SAL_SKIP_MEM_CHECK */

    return 0;
}

#ifdef BROADCOM_DEBUG

typedef struct mblock_s {
    uint32              start_sentinel;		/* value: 0xaaaaaaaa */
    char                *description;
    int                 size;
    struct mblock_s	*prev;
    struct mblock_s	*next;
    uint32              *user_data_start;
    sal_vaddr_t       	_int_pad; /* Do NOT assign anything to this variable */
    /* Variable user data; size S = (size + ALIGNMENT - 1) / 4 words. */
    uint32		user_data[1];
} mblock_t;

static mblock_t *head = NULL;

#ifdef INCLUDE_BCM_SAL_PROFILE
static unsigned int _sal_dma_alloc_max;
static unsigned int _sal_dma_alloc_curr;

#define SAL_DMA_ALLOC_RESOURCE_USAGE_INCR(a_curr, a_max, a_size, ilock) \
        ilock = intLock();                                              \
        a_curr += (a_size);                                             \
        a_max = ((a_curr) > (a_max)) ? (a_curr) : (a_max);              \
        intUnlock(ilock)

#define SAL_DMA_ALLOC_RESOURCE_USAGE_DECR(a_curr, a_size, ilock)        \
        ilock = intLock();                                              \
        a_curr -= (a_size);                                             \
        intUnlock(ilock)

/*
 * Function:
 *      sal_dma_alloc_resource_usage_get
 * Purpose:
 *      Provides Current/Maximum memory allocation.
 * Parameters:
 *      alloc_curr - Current memory usage.
 *      alloc_max - Memory usage high water mark
 */

void 
sal_dma_alloc_resource_usage_get(uint32 *alloc_curr, uint32 *alloc_max)
{
    if (alloc_curr != NULL) {
        *alloc_curr = _sal_dma_alloc_curr;
    }
    if (alloc_max != NULL) {
        *alloc_max = _sal_dma_alloc_max;
    }
}
#endif
#endif /* BROADCOM_DEBUG */

/*
 * Function:
 *	sal_dma_alloc
 * Purpose:
 *	Allocate memory that can be DMAd into/out-of.
 * Parameters:
 *	sx - number of bytes to allocate
 *	s - string associated with allocate
 * Returns:
 *	Pointer to allocated memory or NULL.
 */
void *
sal_dma_alloc(unsigned int sz, char *s)
{
#if defined(BROADCOM_DEBUG) && !defined(SAL_NO_ALLOC_DEBUG)
#ifdef INCLUDE_BCM_SAL_PROFILE
    int		ilock;
#endif
    mblock_t	*p;
    int		size_words = ((sz + SAL_DMA_BUF_ALIGNMENT - 1 + 3) / 4);
    int		il;
    uint32      *user_data;

    /*
     * Allocate space for block structure, user data, and the sentinel
     * at the end of the block (accounted for by user_data[1]).
     */

    if (platform_info->caps & PLATFORM_CAP_DMA_MEM_UNCACHABLE) {
        p = cacheDmaMalloc(sizeof (mblock_t) + size_words * 4);
    } else {
        p = malloc(sizeof (mblock_t) + size_words * 4);
    }

    if (p == NULL) {
	return NULL;
    }

    p->start_sentinel = 0xaaaaaaaa;
    p->description = s;
    p->size = sz;
    user_data = (uint32*) SAL_ALIGN(&p->user_data[0], SAL_DMA_BUF_ALIGNMENT);
    p->user_data_start = user_data;
    *(((sal_vaddr_t*) user_data) - 1) = (sal_vaddr_t) p;
    *(user_data + (sz + 3)/4) = 0xbbbbbbbb;

    il = intLock();
    if (head != NULL) {
	head->prev = p;
        sal_dma_flush(head, sizeof(mblock_t));
    }
    p->prev = NULL;
    p->next = head;
    head = p;
    sal_dma_flush(p, sizeof(mblock_t));
    intUnlock(il);

#ifdef INCLUDE_BCM_SAL_PROFILE
    SAL_DMA_ALLOC_RESOURCE_USAGE_INCR(
        _sal_dma_alloc_curr,
        _sal_dma_alloc_max,
        (size_words * 4),
        ilock);
#endif

    return (void *)user_data;

#else /* !BROADCOM_DEBUG */

    if (platform_info->caps & PLATFORM_CAP_DMA_MEM_UNCACHABLE) {
        return cacheDmaMalloc(sz); 
    } else {
        return malloc(sz);
    }

#endif /* !BROADCOM_DEBUG */
}


#ifdef BROADCOM_DEBUG

/* To do: use real data segment limits for bad pointer detection */

#define GOOD_PTR(p)						\
	(PTR_TO_INT(p) >= 0x00001000U &&			\
	 PTR_TO_INT(p) < 0xfffff000U)

#define GOOD_START(p)						\
	(p->start_sentinel == 0xaaaaaaaa)

#define GOOD_END(addr, size)						\
	(*(((uint32*)(addr)) + ((size + 3)/4)) == 0xbbbbbbbb)

#define GOOD_FIELD		0

#endif /* BROADCOM_DEBUG */

/*
 * Function:
 *	sal_dma_free
 * Purpose:
 *	Free memory allocated by sal_dma_alloc
 * Parameters:
 *	addr - pointer to memory to free.
 * Returns:
 *	Nothing.
 */

void
sal_dma_free(void *addr)
{
#if defined(BROADCOM_DEBUG) && !defined(SAL_NO_ALLOC_DEBUG)
#ifdef INCLUDE_BCM_SAL_PROFILE
    int		ilock;
#endif

    mblock_t	*p;
    int		il;
    int		size_words;

    assert(GOOD_PTR(addr));	/* Use macro to beautify assert message */

    p = (mblock_t *) *(((sal_vaddr_t *) addr) - 1);

    assert(p->user_data_start == addr);

    assert(GOOD_START(p));	/* Use macro to beautify assert message */

    assert(GOOD_END(addr, p->size));

    il = intLock();
    if (p == head) {
	if (p->prev != NULL) {
	    intUnlock(il);
	    assert(GOOD_FIELD);
	} else {
	    head = p->next;
	    if (head != NULL) {
		head->prev = NULL;
                sal_dma_flush(head, sizeof(mblock_t));
	    }
	    intUnlock(il);
	}
    } else {
	if (p->prev == NULL) {
	    intUnlock(il);
	    assert(GOOD_FIELD);
	} else {
	    p->prev->next = p->next;
	    if (p->next != NULL) {
		p->next->prev = p->prev;
                sal_dma_flush(p->next, sizeof(mblock_t));
	    }
            sal_dma_flush(p->prev, sizeof(mblock_t));
	    intUnlock(il);
	}
    }

    /*
     * Detect redundant frees and memory being used after freeing
     * by filling entire block with 0xcc bytes.
     * (Similar to the way VxWorks fills unused memory with 0xee bytes).
     */
    size_words = ((p->size +  SAL_DMA_BUF_ALIGNMENT - 1 + 3) / 4);

    memset(p, 0xcc, sizeof (mblock_t) + size_words * 4);

    if (platform_info->caps & PLATFORM_CAP_DMA_MEM_UNCACHABLE) {
        cacheDmaFree(p);
    } else {
        free(p);
    }

#ifdef INCLUDE_BCM_SAL_PROFILE
    SAL_DMA_ALLOC_RESOURCE_USAGE_DECR(
        _sal_dma_alloc_curr,
        (size_words * 4),
        ilock);
#endif

#else /* !BROADCOM_DEBUG */

    if (platform_info->caps & PLATFORM_CAP_DMA_MEM_UNCACHABLE) {
        cacheDmaFree(addr);
    } else {
        free(addr);
    }

#endif /* !BROADCOM_DEBUG */
}


#ifdef BROADCOM_DEBUG
void 
sal_dma_alloc_stat(void *param)
{
    mblock_t *p;
    char *last_desc = "";
    int repeat;
    int rep_count = 0;
    int tot_size = 0;
    int grand_tot = 0;

    repeat = PTR_TO_INT(param);
    for (p = head; p != NULL; p = p->next) {
        grand_tot += p->size;
        if (repeat) { /* Don't show repetitions */
            if (!strcmp(p->description, last_desc)) {
                rep_count++;
                tot_size += p->size;
            } else {
                if (rep_count) {
                    printf("...to %8p, repeats %4d times.  "
                           "Total size 0x%08x\n",
                           (void *)&p->user_data[0], rep_count+1, tot_size);
                    rep_count = 0;
                    tot_size = 0;
                }
                printf("%8p: 0x%08x %s\n",
                       (void *)&p->user_data[0],
                       p->size,
                       p->description != NULL ? p->description : "???");
                tot_size += p->size;
                last_desc = p->description;
            }
        } else { /* Show every entry */
            printf("%8p: 0x%08x %s\n",
                   (void *)&p->user_data[0],
                   p->size,
                   p->description != NULL ? p->description : "???");
        }
    }
    printf("Grand total of %d bytes allocated\n", grand_tot);
}
#endif /* BROADCOM_DEBUG */

/* 
 * Function:    _sal_i2c_op
 * Purpose:     Maps sal i2c options to hal options and calls hal i2c handler
 * Returns:     0 - success, -1 - failure
 */
int
_sal_i2c_op(int unit, uint32 op_flags, uint16 slave, uint32 addr, 
            uint8 addr_len, uint8 *buf, uint8 buf_len)
{
    int     rv;

    if (platform_info == NULL) {
        cli_out("Platform HAL not initialized \n");
        return -1;
    }
    if (platform_info->f_i2c_op == NULL) {
        cli_out("Platform HAL does not support i2c operations \n");
        return -1;
    }

    if (buf == NULL) {
        cli_out("Input param - buf is invalid (NULL) \n");
        return -1;
    }
    switch (addr_len) {
    case 0:
    case 1:
    case 2:
    case 4:
        /* supported address lengths */
        break; 
    default:
        cli_out("Address length:%d not supported \n", addr_len);
        return -1;
    }

    switch (buf_len) {
    case 0:
    case 1:
    case 2:
    case 4:
        /* supported data lengths */
        break; 
    default:
        cli_out("Data buffer length:%d not supported \n", buf_len);
        return -1;
    }

    if (platform_info->flags & SAL_I2C_FAST_ACCESS) {
        op_flags |= HAL_I2C_FAST_ACCESS;
    }

    rv = platform_info->f_i2c_op(unit, op_flags, slave, addr, addr_len, buf, 
                                 buf_len);
    if (rv < 0) {
        cli_out("HAL i2c op failed. \n");
    }

    return rv;
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
    int rv;

    rv = _sal_i2c_op(unit, HAL_I2C_RD, slave, addr, addr_len, buf, buf_len);
    if (rv < 0) {
        cli_out("%s: Failed. \n", FUNCTION_NAME());
        rv = -1; /* override rv to -1 */
    }

    return rv;
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
    int rv;

    rv = _sal_i2c_op(unit, HAL_I2C_WR, slave, addr, addr_len, buf, buf_len);
    if (rv < 0) {
        cli_out("%s: Failed. \n", FUNCTION_NAME());
        rv = -1; /* override rv to -1 */
    }

    return rv;
}

/* 
 * unit     - I2C controller 
 * flags    - SAL_I2C_* flags 
 */
int
sal_i2c_config_set(int unit, uint32 flags)
{
    if (platform_info == NULL) {
        cli_out("Platform HAL not initialized \n");
        return -1;
    }

    /* Only SAL_I2C_FAST_ACCESS is supported for now */
    if (flags & (~SAL_I2C_FAST_ACCESS)) {
        cli_out("%s: Unsupported flags specified: %x", FUNCTION_NAME(),
               (flags & (~SAL_I2C_FAST_ACCESS)));
        return -1;
    }

    /* SAL_I2C_FAST_ACCESS flag applies to all units */
    platform_info->flags = flags;

    return 0;
}

/* 
 * unit     - I2C controller 
 * flags    - SAL_I2C_* flags 
 */
int
sal_i2c_config_get(int unit, uint32 *flags)
{
    if (platform_info == NULL) {
        cli_out("Platform HAL not initialized \n");
        return -1;
    }

    if (flags == NULL) {
        cli_out("Invalid flags (NULL) param. \n");
        return -1;
    }

    *flags = platform_info->flags;
    return 0;
}
