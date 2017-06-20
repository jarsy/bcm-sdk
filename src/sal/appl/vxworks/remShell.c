/* remShell.c - remote shell service library */

/* Copyright 1997 Wind River Systems, Inc. */
#include "copyright_wrs.h"

/* $Id: remShell.c,v 1.7 2011/07/21 16:13:21 yshtil Exp $
modification history
--------------------
01a,28may97,mjc  written.
*/

/*
DESCRIPTION
This module provides a remote login facility for VxWorks shell via telnet.

The telnet daemon accepts remote telnet login requests and causes the shell's 
input and output to be redirected to the remote user.  The telnet daemon 
supporting this facility is started by calling remShellInit().

Internally, the telnet daemon provides a tty-like interface to the remote user
through the use of the VxWorks pseudo-terminal driver, ptyDrv.

INCLUDE FILES: remShell.h

SEE ALSO:
ptyDrv, rlogLib
*/

#include "vxWorks.h"
#include "telnetLib.h"
#include "shellLib.h"
#include "taskLib.h"
#if VX_VERSION == 69
#include <private/shellLibP.h>
#endif
#include "sysLib.h"
#include "string.h"
#include "logLib.h"
#include "ioLib.h"
#include "stdio.h"
#include "sockLib.h"
#include "usrLib.h"
#include "taskLib.h"

/* defines */

/* Defining BROADCOM_DEBUG will make this library hook remote telnet clients up to
 * the routine copyStreams() instead of to the shell.  This is useful for
 * debugging telnet without losing the shell. */

/* #define BROADCOM_DEBUG */

#define TELNET_SERVICE          23      /* telnet port number */


/* global variables */

IMPORT	int logFdFromRlogin;	/* fd of pty for telnet session */

/* local variables */

LOCAL char *telnetShellName = "tShell";
LOCAL int telnetShellDemonPriority = 2;

LOCAL BOOL activeFlag = FALSE;	/* TRUE if there is an active connection */

LOCAL int shellInFd;		/* original console input */
LOCAL int shellOutFd;		/* original console output */
LOCAL int shellErrFd;		/* original console error output */


/*******************************************************************************
*
* remShellTelnetDisconn - disconnect shell from telnet
*
* This routine is called from the telnetInTask either as a result of the 
* connection terminating from the remote host, or as a result of the logout() 
* command issued to our shell.  It is therefore always run in the context of 
* the telnetInTask.
* 
* The shell standard I/O is redirected back to the console, and the shell is 
* restarted.
*
* RETURNS: N/A
*/
void remShellTelnetDisconn
    (
    int disconnArg
    )
    {
#if VX_VERSION == 69
    shellLogoutInstall ((SHELL_LOGOUT_RTN) NULL, (void *)0); /* uninstall logout function */
#else
    shellLogoutInstall ((FUNCPTR) NULL, 0);     /* uninstall logout function */
#endif
    if (logFdFromRlogin != NONE)
        {
        logFdDelete (logFdFromRlogin);		/* cancel extra log device */
        logFdFromRlogin = NONE;			/* reset fd */
        }
#if VX_VERSION == 55 || VX_VERSION == 542
    shellOrigStdSet (STD_IN,  shellInFd);       /* restore shell's stnd I/O */
    shellOrigStdSet (STD_OUT, shellOutFd);
    shellOrigStdSet (STD_ERR, shellErrFd);
#endif
    shellLock (FALSE);                          /* unlock shell */

    /* For typical remote sessions, there is no need to restart the shell.
     * If we are in shell context, simply restoring the standard I/O
     * descriptors is enough to get the shell back on track upon return
     * from this function.  If we are in telnetInTask context, the closing
     * of the pty device will cause the shell to unblock from its read()
     * and do subsequent I/O from the restored descriptors.
     * However, problems can occur upon logout if the remote user has
     * disabled the line editor and/or put the pty device in raw mode.
     * The problem caused is that the shell does not resume properly.
     * It is therefore deemed prudent to always restart the shell, thereby
     * avoiding any funny business.
     *
     * The previous version attempted to send a ctrl-D up the pty device
     * to wakeup and restart the shell.  Unfortunately, ctrl-D only has
     * special meaning when the device is in line mode, and hence did
     * not work in raw mode.
     *
     * The pty device is closed after the shell is restarted, when called
     * from telnetInTask, to avoid waking the existing shell and causing an
     * additional prompt to appear on the console.
     */

    activeFlag = FALSE;			/* allow new connection */

#if (_WRS_VXWORKS_MAJOR >= 6) 
    excJobAdd((VOIDFUNCPTR)shellRestart,
                  (int)CURRENT_SHELL_SESSION , 0, 0, 0, 0, 0);
#else
    excJobAdd(shellRestart, FALSE, 0, 0, 0, 0, 0);
#endif
    } /* remShellTelnetDisconn */

/*******************************************************************************
*
* remShellTelnetConn - connect shell to telnet
*
* This routine is called by the telnet demon to connect the shell with the 
* remote telnet user.
* Remote telnet requests will cause `stdin', `stdout', and `stderr' to be stolen
* away from the console.  When the remote user disconnects, `stdin', `stdout',
* and `stderr' are restored, and the shell is restarted.
*
* RETURNS: OK or ERROR.
*/
STATUS remShellTelnetConn
    (
    int              interpConnArg,	/* argument for interpConn () */
    int              slaveFd,
    int              exitArg,		/*  */
    int *            disconnArg,
    char *           msg
    )
    {
    /* wait for shell to exist */

    while (taskNameToId (telnetShellName) == ERROR)
        taskDelay (sysClkRateGet ());

    /* check to see if there's already an active connection */

    if (activeFlag)
        {
        strcpy(msg, "\r\nSorry, this system is engaged.\r\n");

        return ERROR;
        }

    if (!shellLock (TRUE))
	{
	strcpy(msg, "\r\nSorry, shell is locked.\r\n");

	printErr ("telnetd: someone tried to login into.\n");
        return ERROR;
	}

    printErr ("\ntelnetd: This system *IN USE* via telnet.\n");
#if VX_VERSION == 69
    shellLogoutInstall ((SHELL_LOGOUT_RTN) telnetdExit, (void *)exitArg);
#else
    shellLogoutInstall ((FUNCPTR) telnetdExit, exitArg);
#endif
    activeFlag    = TRUE;

    /* get the shell's standard I/O fd's so we can restore them later */

    shellInFd  = ioGlobalStdGet (STD_IN);
    shellOutFd = ioGlobalStdGet (STD_OUT);
    shellErrFd = ioGlobalStdGet (STD_ERR);

#ifndef BROADCOM_DEBUG
#if VX_VERSION == 55 || VX_VERSION == 542
    /* set shell's standard I/O to pty device; add extra logging device */

    shellOrigStdSet (STD_IN, slaveFd);
    shellOrigStdSet (STD_OUT, slaveFd);
    shellOrigStdSet (STD_ERR, slaveFd);

    logFdAdd (slaveFd);
    logFdFromRlogin = slaveFd;      /* store pty fd for logFdSet() */
#endif
#endif	/* BROADCOM_DEBUG */

    /* the shell is currently stuck in a read from the console, so we
     * restart it */

#ifndef BROADCOM_DEBUG
#if (_WRS_VXWORKS_MAJOR >= 6) 
    excJobAdd((VOIDFUNCPTR)shellRestart,
                  (int)CURRENT_SHELL_SESSION , 0, 0, 0, 0, 0);
#else
    excJobAdd (shellRestart, TRUE, 0, 0, 0, 0, 0);
#endif
#else
    sp (copyStreams, slaveFd, slaveFd,0,0,0,0,0,0,0); /* !!! */
#endif	/* BROADCOM_DEBUG */

    (void) ioctl (slaveFd, FIOFLUSH, 0 /*XXX*/);

    return OK;
    } /* remShellTelnetConn */

/*******************************************************************************
*
* remShellInit - initialize the remote shell facility
*
* This routine initializes the remote shell  facility, which supports remote 
* login to the VxWorks shell via the telnet protocol.  It spawns the telnet 
* daemon by call to the telnetServiceAdd().  This demon get name "tTnetShelld"
* and accepts connections from the standard telnet service port.
*
* RETURNS: N/A
*/
void remShellInit (void)
    {
    telnetServiceAdd("Shell", TELNET_SERVICE, telnetShellDemonPriority, 
                     remShellTelnetConn, 0, remShellTelnetDisconn);
    } /* remShellInit */
