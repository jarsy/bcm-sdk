#if defined(INCLUDE_TELNET)
/* telnetLib.c - telnet server library */

/* Copyright 1984-1997 Wind River Systems, Inc. */
#include "copyright_wrs.h"

/* $Id: telnetLib.c,v 1.8 2011/07/21 16:13:21 yshtil Exp $
modification history
--------------------
04a,28may97,mjc  redesigned to allow user specified command interpreter and  
                 concurrent telnet connection support
03b,09aug94,dzb  fixed activeFlag race with cleanupFlag (SPR #2050).
                 made telnetdSocket global (SPR #1941).
		 added logFdFromRlogin (SPR #2212).
03a,02may94,ms   increased stack size for SIMHPPA.
02z,11aug93,jmm  Changed ioctl.h and socket.h to sys/ioctl.h and sys/socket.h
02y,01feb93,jdi  documentation cleanup for 5.1.
02x,18jul92,smb  Changed errno.h to errnoLib.h.
02w,26may92,rrr  the tree shuffle
		  -changed includes to have absolute path from h/
02v,24apr92,rfs  Fixed flaky shell restart upon connection termination.
                 The functions telnetInTask() and telnetdExit() were changed.
                 This is fixing SPR #1427.  Also misc ANSI noise.
02u,13dec91,gae  ANSI cleanup.
02t,14nov91,jpb  moved remCurIdSet to shellLogout (shellLib.c).
02s,04oct91,rrr  passed through the ansification filter
                  -changed functions to ansi style
		  -changed includes to have absolute path from h/
		  -fixed #else and #endif
		  -changed READ, WRITE and UPDATE to O_RDONLY O_WRONLY and ...
		  -changed VOID to void
		  -changed copyright notice
02r,01aug91,yao  fixed to pass 6 args to excJobAdd() call.
02q,13may91,shl  undo'ed 02o.
02p,30apr91,jdi	 documentation tweaks.
02o,29apr91,shl  added call to restore original machine name, user and
		 group ids (spr 916).
02n,05apr91,jdi	 documentation -- removed header parens and x-ref numbers;
		 doc review by dnw.
02m,24mar91,jdi  documentation cleanup.
02l,05oct90,shl  fixed telnetExit() to restore original user and password.
02k,02oct90,hjb  added a call to htons() where needed.
02j,08aug90,dnw  changed declaration of tnInput from void to int.
		 added forward declaration of setMode().
02i,07may90,shl  changed entry point of tTelnetd back to telnetd.
02h,18apr90,shl  added shell security.
		 changed telnetd name to tTelnetd.
02g,20aug89,gae  bumped telnetTaskStackSize from 1500 to 5000 for SPARC.
02f,29sep88,gae  documentation.
02e,06jun88,dnw  changed taskSpawn/taskCreate args.
02d,30may88,dnw  changed to v4 names.
02c,28may88,dnw  changed to use shellOrigStdSet (...) instead of shellSetOrig...
		 changed not to use shellTaskId global variable.
02b,01apr88,gae  made it work with I/O system revision.
02a,27jan88,jcf  made kernel independent.
01g,14dec87,dnw  fixed bug in telnetdIn() that caused system crashes.
01f,19nov87,dnw  changed telnetd to wait for shell to exist before accepting
		   remote connections.
01e,17nov87,ecs  lint.
01d,04nov87,ecs  documentation.
	     &   fixed bug in use of setsockopt().
	    dnw  changed to call shellLogoutInstall() instead of logoutInstall.
01c,24oct87,gae  changed setOrig{In,Out,Err}Fd() to shellSetOrig{In,Out,Err}().
		 made telnetdOut() exit on EOF from master pty fd.
		 made telnetInit() not sweat being called more than once.
		 added shellLock() to telnetd to get exclusive use of shell.
01g,20oct87,gae  added logging device for telnet shell; made telnetInit()
		   create pty device.
01f,05oct87,gae  made telnetdExit() from telnetdIn() - used by logout().
		 removed gratuitous standard I/O ioctl's.
		 made "disconnection" cleaner by having shell do restart.
01e,26jul87,dnw  changed default priority of telnet tasks from 100 to 2.
		 changed task priority and ids to be global variables so
		   they can be accessed from outside this module.
01d,04apr87,dnw  de-linted.
01c,27mar87,dnw  documentation
		 fixed bug causing control sequences from remote to be
		   misinterpreted.
		 added flushing of pty in case anything was left before
		   remote login.
01b,27feb87,dnw  changed to spawn telnet tasks UNBREAKABLE.
01a,20oct86,dnw  written.
*/

/*
DESCRIPTION
This library provides a remote login facility for VxWorks.  It uses the 
telnet protocol to enable users on remote systems to log in to VxWorks.
This telnet facility supports remote login to the VxWorks shell and the user 
specified command interpreters.  It also support concurrent telnet connections.

The telnet daemon provides a tty-like interface to the remote user through the 
use of the VxWorks pseudo-terminal driver, ptyDrv.

The telnet facility is initialized by calling telnetInit(), which is called
automatically when INCLUDE_TELNET is defined in configAll.h.

The trivial VxWorks telnet service that connect remote telnet user with the 
built-in VxWorks shell via standard telnet port is provided by module 
remShell.c.

This module have 2 levels of the API.

.SH "LOW LEVEL API:"
The telnet daemon  accepts remote telnet login requests and connect the command 
interpreter with the remote user by calling the command interpreter specific 
connection routine which gets a file descriptor of the pty device as one of its 
arguments.

The command interpreter connection routine should be declared as follows:

.CS
STATUS interpConn 
    (
    int     interpConnArg,
    int     ptySFd,
    int     exitArg,
    int *   disconnArg,
    char *  msgBuf
    )
.CE
.IP "<interpConnArg>"
an argument specified by user in the telnetServiceAdd() call;
.IP "<ptySFd>"
file descriptor of the pty device across which the command interpreter connects 
with the telnet;
.IP "<telnetdExit>"
a pointer to the routine that the command interpreter must call in response user
logout;
.IP "<exitArg>"
an argument to pass to the telnet logout service routine (telnetdExit());
.IP "<disconnArg>"
a pointer to a variable where argument for interpDisconn() may be returned, 
initialized to zero before interpConn() call;
.IP "<msgBuf>"
a pointer to a buffer where the error message may be returned when interpConn() 
terminates with ERROR, the telnet demon sends this message to the telnet client.
.LP
Each time a connection terminates telnet calls the command interpreter specific 
disconnection routine providing it with value returned by the corresponding 
interpConn() call.  This command interpreter disconnection routine is called 
independently of the reason of the connection termination: the connection 
closure by the remote host, or the logout command issued to a command 
interpreter by the remote user.

The command interpreter disconnection routine should be declared as follows:

.CS
void interpDisconn
    (
    int disconnArg
    )
.CE
.IP "<disconnArg>"
value returned by the corresponding interpConn() call
.LP
When the logout command is issued to a command interpreter, it must call the 
telnetdExit() routine to inform the telnet that the user is wishing to close 
connection.

To add such kind of the telnet service use telnetServiceAdd() call.

.SH "HIGH LEVEL API (telnet call):"
All that the user of the module must provide this is the routine that read data 
from the stdin and write data to the stdout.  When the telnet accepts remote 
telnet login requests it spawns the new task, causes the task's standard input 
and output to be redirected to the remote user, and calls the specified routine.
When the remote user disconnects this routine must just exit.  

If a telnet connection is broken by a remote host, standard I/O of the task 
that calls the routine provided by the module user, is redirected to an invalid 
file descriptor equal to the maximum number of file descriptors in the sistem 
(NUM_FILES defined in configAll.h).  Thus subsequent reads from and writes to 
standard I/O of that task will return error.

To add such kind of the telnet service use telnetCallAdd() call.

INCLUDE FILES: telnetLib.h

SEE ALSO:
ptyDrv, rlogLib
*/

#include "vxWorks.h"
#include "sys/types.h"
#include "sys/socket.h"
#include "ptyDrv.h"
#include "netinet/in.h"
#include "ioLib.h"
#include "taskLib.h"
#include "telnetLib.h"
#include "stdlib.h"
#include "unistd.h"
#include "errnoLib.h"
#include "string.h"
#include "stdio.h"
#include "fcntl.h"
#include "sockLib.h"
#include "logLib.h"
#include "shellLib.h"
#include "remLib.h"
#include "sysLib.h"
#include "iosLib.h"
#include "taskVarLib.h"
#include "config.h"
#include <sockLib.h>
#include <sys/socket.h>
#include <inetLib.h>

#define STDIN_BUF_SIZE		512
#define STDOUT_BUF_SIZE		512

#define sendToPty(fd,buf,len)	write(fd,buf,len)
#define sendToRem(fd,buf,len)	write(fd,buf,len)

/* telnet input states */

#define TS_DATA		0
#define TS_IAC		1
#define TS_CR		2
#define TS_BEGINNEG	3
#define TS_ENDNEG	4
#define TS_WILL		5
#define TS_WONT		6
#define TS_DO		7
#define TS_DONT		8

/* telnet pty device state */

#define TELNET_PTY_FREE	1
#define BUSY	0

#define ERROR_MSG_LEN	260
#define PTY_NAME_LEN	20


/* typedefs */

typedef struct telnetCallCtrl
    {
    int     priority;
    int     options;
    int     stackSize;
    FUNCPTR func;
    int     arg;
    char    name [8];
    } TELNET_CALL_CTRL;


/* global variables */

#ifndef BROADCOM_DEBUG
int telnetTaskOptions   = VX_SUPERVISOR_MODE | VX_UNBREAKABLE;
#else
int telnetTaskOptions   = VX_SUPERVISOR_MODE;
#endif  /* BROADCOM_DEBUG */

#if    (CPU_FAMILY == SIMHPPA) || (CPU_FAMILY == SIMSPARCSUNOS)
int telnetTaskStackSize = 10000;
#else  /* CPU_FAMILY == SIMHPPA */
int telnetTaskStackSize = 5000;
#endif /* CPU_FAMILY == SIMHPPA */


/* local variables */

LOCAL char ptyTelnetName [] = "/pty/telnet";
LOCAL BOOL * telnetPtyDevs;	/* array that indicates using of pty devices */
LOCAL int telnetMaxConnNum;	/* Maximum number of telnet connections. */
LOCAL BOOL telnetCallCloseConn;	/*  */


/*******************************************************************************
*
* telnetCallMain - telnet call task main routine
*
* This routine redirects its standard I/O to the slave pty device provided by 
* the telnet demon and calls the routine provided by the corresponding 
* telnetCallAdd() call.  If the telnet connection has yet not been closed by 
* the telnetInTask() when telnetCallFunc() is exiting this routine call 
* telnetdExit().
*
* RETURNS: N/A
*
* NOMANUAL - but not LOCAL for i()
*/

void telnetCallMain 
    (
    FUNCPTR          telnetCallFunc, /* externally provided routine */
    int              arg,             /* telnetCallFunc() argument */
    int              slaveFd,         /* slave pty device fd */
    int              exitArg          /* telnetdExit() argument */
    )
    {
    int buf;

    if (taskVarAdd (0, &telnetCallCloseConn) != OK)
        {
        logMsg ("can't add task variable\n", 0, 0, 0, 0, 0, 0);
#if VX_VERSION == 69
        telnetdExit ((void *)exitArg);
#else
        telnetdExit (exitArg);
#endif
        read (slaveFd, (char *)&buf, sizeof(buf));
        return;
        }

    telnetCallCloseConn = TRUE;

    ioTaskStdSet (0, STD_IN,  slaveFd);
    ioTaskStdSet (0, STD_OUT, slaveFd);
    ioTaskStdSet (0, STD_ERR, slaveFd);

    telnetCallFunc (arg);

    /* Force telnetInTask() to close connection */

    if (telnetCallCloseConn)
        {
#if VX_VERSION == 69
        telnetdExit ((void *)exitArg);
#else
        telnetdExit (exitArg);
#endif
        read (slaveFd, (char *)&buf, sizeof(buf));
        }
    } /* telnetCallMain */

/*******************************************************************************
*
* telnetCallDisconn - telnet call disconnect routine
*
* This routine redirect standard I/O of the task, that call the externally 
* provided I/O processing function, to the invalid file descriptor (equal to 
* the maximum number of file descriptors in the sistem).  That force subsequent 
* reads from and writes to standard I/O of that task to return error.
*
* RETURNS: N/A
*/

LOCAL void telnetCallDisconn
    (
    int tId  /* task Id of the corresponding telnet call task */
    )
    {
    /* In form telnetCallMain() that it have not call telnetdExit() */

    taskVarSet (tId, &telnetCallCloseConn, FALSE);

    /* Redirect telnetCallMain() standard I/O */

    ioTaskStdSet (tId, STD_IN,  NUM_FILES);
    ioTaskStdSet (tId, STD_OUT, NUM_FILES);
    ioTaskStdSet (tId, STD_ERR, NUM_FILES);
    } /* telnetCallDisconn */

/*******************************************************************************
*
* telnetCallConn - telnet call connect routine 
*
* This routine spawns the new task that redirects its standard I/O to the slave 
* pty device provided by the telnet demon and calls the routine provided by the 
* corresponding telnetCallAdd() call.
*
* RETURNS: 
*/

LOCAL STATUS telnetCallConn
    (
    TELNET_CALL_CTRL * pTelnetCallCtrl, /*  */
    int                slaveFd,         /*  */
    int                exitArg,         /*  */
    int *              disconnArg,      /*  */
    char *             msg              /*  */
    )
    {
    int tId;
    int connNum;
    char nameBuf[20];

    ioctl (slaveFd, FIOGETNAME, (int)&nameBuf);
    connNum = atoi(nameBuf + sizeof(ptyTelnetName) - 1);
    sprintf(nameBuf, "t%s%d", pTelnetCallCtrl->name, connNum);
    tId = taskSpawn (nameBuf, pTelnetCallCtrl->priority, 
                    pTelnetCallCtrl->options, pTelnetCallCtrl->stackSize, 
                    (FUNCPTR)telnetCallMain, (int)pTelnetCallCtrl->func, 
                    pTelnetCallCtrl->arg, slaveFd, exitArg, 
                    0, 0, 0, 0, 0, 0);
    if(tId == ERROR)
        {
        printErr("telnetCallConn: task %s spawn error\n", nameBuf);
        return ERROR;
        }

    *disconnArg = tId;

    return OK;
    } /* telnetCallConn */

/*******************************************************************************
*
* telnetCallAdd - add telnet call service
*
* This routine spawn the new telnet demon to service telnet connection requests
* arrives to the port. It takes the following arguments:
* .IP <telnetCallName>
* name of the service (20 characters maximum);
* .IP <telnetCallPortNum>
* port number for the service;
* .IP <telnetCallFunc>
* function to be called by telnet to process a remote user I/O;
* .IP <telnetCallArg>
* argument to pass to the function;
* .IP <telnetCallPriority>
* priority of the task that will be spawned to call the function;
* .IP <telnetCallOptions>
* options of the task that will be spawned to call the function;
* .IP <telnetCallStackSize> 
* stack size of the task that will be spawned to call the function;
*
* RETURNS: N/A
*/

void telnetCallAdd 
    (
    char *  telnetCallName,     /* name of the service */
    int     telnetCallPortNum,  /* port number for the service */
    FUNCPTR telnetCallFunc,     /* function to process a remote user I/O */
    int     telnetCallArg,      /* argument to pass to the function */
    int     telnetCallPriority, /* priority of the I/O processing task */
    int     telnetCallOptions,  /* options of the I/O processing task */
    int     telnetCallStackSize /* stack size of the I/O processing task */
    )
    {
    TELNET_CALL_CTRL * pTelnetCallCtrl;

    pTelnetCallCtrl = malloc (sizeof (*pTelnetCallCtrl));

    pTelnetCallCtrl->priority  = telnetCallPriority;
    pTelnetCallCtrl->options   = telnetCallOptions;
    pTelnetCallCtrl->stackSize = telnetCallStackSize;
    pTelnetCallCtrl->func      = telnetCallFunc;
    pTelnetCallCtrl->arg       = telnetCallArg;
    strcpy (pTelnetCallCtrl->name, telnetCallName);

    telnetServiceAdd (pTelnetCallCtrl->name, telnetCallPortNum, 
                      (telnetCallPriority - 1), (INTERP_CON_PROC)telnetCallConn,
                      (int) pTelnetCallCtrl, telnetCallDisconn);
    } /* telnetCallAdd */

/*******************************************************************************
*
* telnetPtyAlloc - allocate a pty device
*
* RETURNS: OK if a pty device was found, otherwise ERROR.
*/
LOCAL STATUS telnetPtyAlloc 
    (
    char *       ptyName,
    const char * serviceName
    )
    {
    int i;

    taskLock ();

    for (i = 0; i < telnetMaxConnNum; i++)
        if (telnetPtyDevs[i] == TELNET_PTY_FREE)
            {
            telnetPtyDevs[i] = BUSY;
            taskUnlock ();

            sprintf (ptyName, "%s%d.", ptyTelnetName, i);
            return OK;
            }

    taskUnlock ();
    return ERROR;
    } /* telnetPtyAlloc */

/*******************************************************************************
*
* telnetPtyFree - free a pty device
*
* RETURNS: N/A
*/
LOCAL void telnetPtyFree
    (
    char * ptyName
    )
    {
    telnetPtyDevs[ atoi(ptyName + sizeof(ptyTelnetName) - 1) ] = TELNET_PTY_FREE;
    } /* telnetPtyFree */

/*******************************************************************************
*
* setMode -
*
* RETURNS: N/A.
*/

LOCAL void setMode
    (
    int    ptyFd,		/* fd to local pseudo-terminal */
    BOOL * pRaw,		/* TRUE = raw mode enabled */
    BOOL * pEcho,		/* TRUE = echo enabled */
    int    telnetOption,
    BOOL   enable
    )
    {
    FAST int ioOptions;

    switch (telnetOption)
	{
	case TELOPT_BINARY: *pRaw  = enable; break;
	case TELOPT_ECHO:   *pEcho = enable; break;
	}

    if (*pRaw)
	ioOptions = 0;
    else
	{
	ioOptions = OPT_7_BIT | OPT_ABORT | OPT_TANDEM | OPT_LINE;
	if (*pEcho)
	    ioOptions |= OPT_ECHO | OPT_CRMOD;
	}

    (void) ioctl (ptyFd, FIOOPTIONS, ioOptions);
    } /* setMode */

/*******************************************************************************
*
* localDoOpt - offer/acknowledge local support of option
*
* This routine will try to enable or disable local support for the specified
* option.  If local support of the option is already in the desired mode, no
* action is taken.  If the request is to disable the option, the option will
* always be disabled.  If the request is to enable the option, the option
* will be enabled, IF we are capable of supporting it.  The remote is
* notified that we WILL/WONT support the option.
*
* RETURNS: OK or ERROR.
*/

LOCAL STATUS localDoOpt
    (
    TBOOL    myOpts[],	/* current option settings - this side */
    BOOL *   pRaw,	/* TRUE = raw mode enabled */
    BOOL *   pEcho,	/* TRUE = echo enabled */
    FAST int option,	/* option to be enabled/disabled */
    BOOL     enable,	/* TRUE = enable option, FALSE = disable */
    int      remFd,	/* fd to remote */
    int      ptyFd,	/* fd to local pseudo-terminal ??? */
    BOOL     reqFromRem	/* TRUE = request is from remote */
    )
    {
    BOOL will = enable;

    if (myOpts [option] == enable)
	return (OK);

    switch (option)
	{
	case TELOPT_BINARY:
	case TELOPT_ECHO:
	    setMode (ptyFd, pRaw, pEcho, option, enable);
	    break;

	case TELOPT_SGA:
	    break;

	default:
	    will = FALSE;
	    break;
	}

    if ((myOpts [option] != will) || reqFromRem)
	{
	unsigned char msg[3];

	msg[0] = IAC;
	msg[1] = will ? WILL : WONT;
	msg[2] = option;

	sendToRem (remFd, (char*) msg, 3);

	myOpts [option] = will;
	}

    return ((will == enable) ? OK : ERROR);
    } /* localDoOpt */

/*******************************************************************************
*
* remDoOpt - request/acknowledge remote enable/disable of option
*
* This routine will try to accept the remote's enable or disable,
* as specified by "will", of the remote's support for the specified option.
* If the request is to disable the option, the option will always be disabled.
* If the request is to enable the option, the option will be enabled, IF we
* are capable of supporting it.  The remote is notified to DO/DONT support
* the option.
*
* RETURNS: OK or ERROR.
*/

LOCAL STATUS remDoOpt
    (
    TBOOL    remOpts[],	/* current option settings - other side */
    BOOL *   pRaw,	/* TRUE = raw mode enabled */
    BOOL *   pEcho,	/* TRUE = echo enabled */
    FAST int option,    /* option to be enabled/disabled */
    BOOL     enable,	/* TRUE = enable option, FALSE = disable */
    int      remFd,	/* fd to remote */
    int      ptyFd,	/* fd to local pseudo-terminal ??? */
    BOOL     reqFromRem	/* TRUE = request is from remote */
    )
    {
    BOOL doOpt = enable;

    if (remOpts [option] == enable)
	return (OK);

    switch (option)
	{
	case TELOPT_BINARY:
	case TELOPT_ECHO:
	    setMode (ptyFd, pRaw, pEcho, option, enable);
	    break;

	case TELOPT_SGA:
	    break;

	default:
	    doOpt = FALSE;
	    break;
	}

    if ((remOpts [option] != doOpt) || reqFromRem)
	{
	unsigned char msg[3];

	msg[0] = IAC;
	msg[1] = doOpt ? DO : DONT;
	msg[2] = option;

	sendToRem (remFd, (char*) msg, 3);

	remOpts [option] = doOpt;
	}

    return ((enable == doOpt) ? OK : ERROR);
    } /* remDoOpt */

/*******************************************************************************
*
* tnInput - process input from remote
*
* RETURNS: state
*/

LOCAL int tnInput
    (
    TBOOL      myOpts[],	/* current option settings - this side */
    TBOOL      remOpts[],	/* current option settings - other side */
    BOOL *     pRaw,		/* TRUE = raw mode enabled */
    BOOL *     pEcho,		/* TRUE = echo enabled */
    FAST int   state,		/* state of input stream */
    FAST int   remFd,		/* fd of socket to otherside */
    FAST int   ptyFd,		/* fd of pty to this side */
    FAST char *buf,		/* ptr to input chars */
    FAST int   n		/* number of chars input */
    )
    {
    char cc;
    int ci;

    while (--n >= 0)
	{
	cc = *buf++;			/* get next character */
	ci = (unsigned char) cc;	/* convert to int since many values
					 * are negative characters */
	switch (state)
	    {
	    case TS_CR:		/* doing crmod; ignore add'l linefeed */
		state = TS_DATA;
		if ((cc != EOS) && (cc != '\n'))
		    sendToPty (ptyFd, &cc, 1);	/* forward char */
		break;

	    case TS_DATA:	/* just pass data */
		if (ci == IAC)
		    state = TS_IAC;
		else
		    {
		    sendToPty (ptyFd, &cc, 1);	/* forward char */
		    if (!myOpts [TELOPT_BINARY] && (cc == '\r'))
			state = TS_CR;
		    }
		break;

	    case TS_IAC:

		switch (ci)
		    {
		    case BREAK:		/* interrupt from remote */
		    case IP:
			/* XXX interrupt (); */
			logMsg ("telnetInTask: interrupt\n", 0, 0, 0, 0, 0, 0);
			state = TS_DATA;
			break;

		    case AYT:		/* Are You There? */
			{
			static char aytAnswer [] = "\r\n[yes]\r\n";

			sendToRem (remFd, aytAnswer, sizeof (aytAnswer) - 1);
			state = TS_DATA;
			break;
			}

		    case EC:		/* erase character */
			sendToPty (ptyFd, "\b", 1);
			state = TS_DATA;
			break;

		    case EL:		/* erase line */
			sendToPty (ptyFd, "\025", 1);
			state = TS_DATA;
			break;

		    case DM:		/* data mark */
			state = TS_DATA;
			break;

		    case SB:		/* sub-option negotiation begin */
			state = TS_BEGINNEG;
			break;

		    case WILL: state = TS_WILL;	break;	/* remote will do opt */
		    case WONT: state = TS_WONT;	break;	/* remote wont do opt */
		    case DO:   state = TS_DO;	break;	/* req we do opt */
		    case DONT: state = TS_DONT;	break;	/* req we dont do opt */

		    case IAC:
			sendToPty (ptyFd, &cc, 1);	/* forward char */
			state = TS_DATA;
			break;
		    }
		break;

	    case TS_BEGINNEG:
		/* ignore sub-option stuff for now */
		if (ci == IAC)
		    state = TS_ENDNEG;
		break;

	    case TS_ENDNEG:
		state = (ci == SE) ? TS_DATA : TS_BEGINNEG;
		break;

	    case TS_WILL:		/* remote side said it will do opt */
		(void)remDoOpt (remOpts, pRaw, pEcho, ci, TRUE, remFd, ptyFd, 
                                TRUE);
		state = TS_DATA;
		break;

	    case TS_WONT:		/* remote side said it wont do opt */
		(void)remDoOpt (remOpts, pRaw, pEcho, ci, FALSE, remFd, ptyFd, 
                                TRUE);
		state = TS_DATA;
		break;

	    case TS_DO:			/* remote wants us to do opt */
		(void)localDoOpt (myOpts, pRaw, pEcho, ci, TRUE, remFd, ptyFd, 
                                          TRUE);
		state = TS_DATA;
		break;

	    case TS_DONT:		/* remote wants us to not do opt */
		(void)localDoOpt (myOpts, pRaw, pEcho, ci, FALSE, remFd, ptyFd, 
                                  TRUE);
		state = TS_DATA;
		break;

	    default:
		printErr ("telnetd: invalid state = %d\n", state);
		break;
	    }
	}

    return (state);
    } /* tnInput */

/*******************************************************************************
*
* telnetOutTask - stdout to socket process
*
* This routine gets spawned by the telnet daemon to move data between the
* client socket and the pseudo-terminal.  The task exits when the telnetdExit() 
* is called, or the client disconnects.
*
* NOMANUAL - but not LOCAL for i()
*/

void telnetOutTask
    (
    FAST int sock,      /* socket to copy output to */
    FAST int ptyMfd     /* pty Master fd */
    )
    {
    FAST int n;
    char buf [STDOUT_BUF_SIZE];

    while ((n = read (ptyMfd, buf, sizeof (buf))) > 0)
	{
	/* XXX should scan for IAC and double 'em to escape 'em */
	write (sock, buf, n);
	}
    } /* telnetOutTask */

/*******************************************************************************
*
* telnetInTask - socket to stdin process
*
* This routine gets spawned by the telnet daemon to move data between the
* pseudo-terminal and the client socket.  The task exits when the telnetdExit() 
* is called, or the client disconnects.
*
* RETURNS: N/A.
*
* NOMANUAL - but not LOCAL for i()
*/

void telnetInTask
    (
    INTERP_DIS_PROC interpDisconn, /* disconnects comm. interp. from telnet */
    int             disconnArg,	   /* value returned by the interpConn() call */
    int             ptySfd,	   /* pty Slave fd */

    FAST int        sock,	   /* socket to copy input from */
    int             ptyMfd	   /* pty Master fd */
    )
    {
    int   n = 0;
    int   state = TS_DATA;
    char  buf [STDIN_BUF_SIZE];
    char  ptyName[PTY_NAME_LEN];
    TBOOL myOpts [256];		/* current option settings - this side */
    TBOOL remOpts [256];	/* current option settings - other side */
    BOOL  raw;			/* TRUE = raw mode enabled */
    BOOL  echo;			/* TRUE = echo enabled */
    int   optval;

    /* turn on KEEPALIVE so if the client crashes, we'll know about it */

    optval = 1;
    setsockopt (sock, SOL_SOCKET, SO_KEEPALIVE, 
                (char *) &optval, sizeof (optval));

    /* initialize modes and options and offer to do remote echo */

    raw  = FALSE;
    echo = TRUE;
    bzero ((char *) myOpts, sizeof (myOpts));
    bzero ((char *) remOpts, sizeof (remOpts));

    (void)localDoOpt (&myOpts[0], &raw, &echo, TELOPT_ECHO, TRUE, sock, ptyMfd, 
                      FALSE);

    write (ptyMfd, "\n", 1);

    /* Loop, reading from the socket and writing to the pty. */

    while ((n = read (sock, buf, sizeof (buf))) > 0)
        state = tnInput (&myOpts[0], &remOpts[0], &raw, &echo, state, sock, 
                         ptyMfd, buf, n);

    /* Exit and cleanup.  The above loop will exit when the socket is shut 
     * down.  The socket can be shut down as a result of the connection
     * terminating from the remote host, or as a result of the logout
     * command issued to a command interpreter serviced by this connection.  
     * When the logout command is used, the telnetdExit() routine below is 
     * called.  
     */

    interpDisconn (disconnArg);

    if (ioctl (ptySfd, FIOGETNAME, (int) &ptyName) == OK)
        {
        ptyName[strlen(ptyName) - 1] = '\0';
        telnetPtyFree(ptyName);
        }
    else
        logMsg ("Can't get pty name\n", 0, 0, 0, 0, 0, 0);

    close (sock);
    close (ptyMfd);
    close (ptySfd);

    } /* telnetInTask */

/*******************************************************************************
*
* telnetdExit - exit routine for telnet
*
* This is the support routine for logout.  It must be called from the command  
* interpreter as a result of the logout command being issued to it to inform 
* the telnet that the user wish to terminate connection.
*
* RETURNS: N/A.
*/
#if VX_VERSION == 69
void telnetdExit
    (
    void * logoutParam
    )
    {
    /* In the current implementation the <exitArg> is the fd of the socket used 
     * to establish the telnet connection.
     * Shuting down the socket coses the telnetInTask() above to exit. 
     */

    write ((int)logoutParam, "\n", 1);
    shutdown ((int)logoutParam, 2);
    } /* telnetdExit */

#else
void telnetdExit
    (
    FAST int exitArg
    )
    {
    /* In the current implementation the <exitArg> is the fd of the socket used 
     * to establish the telnet connection.
     * Shuting down the socket coses the telnetInTask() above to exit. 
     */

    write (exitArg, "\n", 1);
    shutdown (exitArg, 2);
    } /* telnetdExit */
#endif

void
logClient(int client, char* msg)
{
#if VX_VERSION == 65
    /* Workaround for inet_ntoa() bug in vx 6.5 */
    extern char * ipcom_inet_ntoa(void *);
#endif    
    int addr_len;
    struct sockaddr_in cli_addr;
    char * cptr;
    addr_len = sizeof(cli_addr);	    
    getpeername(client, (struct sockaddr*)&cli_addr, &addr_len);	    

#if VX_VERSION == 65
    /* Workaround for inet_ntoa() bug in vx 6.5 */
    cptr = ipcom_inet_ntoa(&cli_addr.sin_addr);
#else    
    cptr = inet_ntoa(cli_addr.sin_addr);
#endif    

    printf("NOTICE: %s [%s]\n", msg,cptr);
}


/*******************************************************************************
*
* telnetd - VxWorks telnet daemon
*
* This routine enables remote users to log in to the VxWorks over the network 
* via the telnet protocol.  It is spawned by telnetSeviceAdd(), which should be
* called to add a telnet service.
*
* The telnet daemon requires the existence of pseudo-terminal devices, which 
* must be created by telnetInit() before telnetd() is spawned.  The telnetd()
* routine creates two additional processes, `telnetInTask' and `telnetOutTask',
* whenever a remote user is logged in.  These processes exit when the remote
* connection is terminated.
*
* RETURNS: N/A
*/

void telnetd 
    (
    char *          serviceName,   /* name of the service */
    int             serviceNum,    /* port number of the service */
    INTERP_CON_PROC interpConn,	   /* connects comm. interp. with telnet */
    int             interpConnArg, /* argument for interpConn () */
    INTERP_DIS_PROC interpDisconn  /* disconnects comm. interp. from telnet */
    )
    {
    int telnetTaskPriority;	/* priority of telnet tasks */
    int telnetOutTaskId;	/* task ID of telnetOutTask task */
    int telnetInTaskId;		/* task ID of telnetInTask task */
    struct sockaddr_in myAddress;
    struct sockaddr_in clientAddress;
    int clientAddrLen;
    int client;
    int masterFd;
    int slaveFd;
    int sd;

    int disconnArg;

    int  connNum = 0;		/* number of telnet connection */
    char ptyName [PTY_NAME_LEN];
    char charBuf [ERROR_MSG_LEN];
    char *msg = "\r\nConnection refused - all pty's in use.\r\n";

    taskPriorityGet (0, &telnetTaskPriority); /* for In and Out tasks */

    /* open a socket and wait for a client */
 
    sd = socket (AF_INET, SOCK_STREAM, 0);

    bzero ((char *) &myAddress, sizeof (myAddress));
    myAddress.sin_family = AF_INET;
    myAddress.sin_port   = htons (serviceNum);

    if (bind (sd, (struct sockaddr *) &myAddress, sizeof (myAddress)) == ERROR)
	{
	printErr ("telnetd: port %d bind failed.\n", serviceNum);
	return;
	}

    listen (sd, 1);

    FOREVER
	{
	errnoSet (OK);		/* clear errno for pretty i() display */

        disconnArg = 0;
        client     = 0;
        masterFd   = 0;
        slaveFd    = 0;

	/* now accept connection */

	clientAddrLen = sizeof (clientAddress);
	client = accept (sd, (struct sockaddr *)&clientAddress, &clientAddrLen);

	/* A connection was received.
	 * Don't service it unless you can read something from
	 * the console within a given time-period.
	 */

	
	if (client == ERROR)
	    {
	    printErr ("telnetd: port %d accept failed - status = 0x%x\n",
		      serviceNum, errnoGet ());
	    continue;
	    }
	
	/* create the pseudo terminal:
	 * the master side is connected to the socket to the
	 * remote machine - two processes telnetInTask & telnetOutTask
	 * handle input and output.
	 */

        if (telnetPtyAlloc (ptyName, serviceName) != OK)
            {
		logClient(client, " console in use, "
			  "connection refused for client");
		/* printErr ("telnetd: no more pty devices.\n"); */
	    write (client, msg, strlen (msg));
	    close (client);
            continue;
            }

        connNum = atoi(ptyName + sizeof(ptyTelnetName ) - 1);

        strcpy (charBuf, ptyName);
        strcat(charBuf,"M");
	if ((masterFd = open (charBuf, O_RDWR, 0)) == ERROR)
	    {
	    printErr ("telnetd: error opening %s\n", charBuf);
	    write (client, msg, strlen (msg));
	    goto telnetd_error;
	    }

        charBuf[strlen(charBuf) - 1] = 'S';
	if ((slaveFd = open (charBuf, O_RDWR, 0)) == ERROR)
	    {
	    printErr ("telnetd: error opening %s\n", charBuf);
	    write (client, msg, strlen (msg));
	    goto telnetd_error;
	    }

        /* setup the slave device to act like a terminal */

        (void) ioctl (slaveFd, FIOOPTIONS, OPT_TERMINAL);

        /* flush out pty device */

        (void) ioctl (slaveFd, FIOFLUSH, 0 /*XXX*/);

	/* Log connection */
	logClient(client, "remote login from");
	
        /* Connect the slave pty device with a command interpreter */

        *charBuf = '\0';
        if (interpConn (interpConnArg, slaveFd, client, &disconnArg, charBuf) !=
            OK)
            {
	    printErr ("telnetd: shell connect fault: %s\n", charBuf);
	    write (client, charBuf, strlen (charBuf));
	    goto telnetd_error;
            }

	/* spawn the output process which transfers data from the master pty
	 * to the socket. */

        sprintf(charBuf, "tTnetO%s%d", serviceName, connNum);
	if ((telnetOutTaskId = taskSpawn ( charBuf, telnetTaskPriority,
					   telnetTaskOptions,
					   telnetTaskStackSize,
					   (FUNCPTR)telnetOutTask, 
                        		   client, masterFd, 
					   0, 0, 0, 0, 0, 0, 0, 0)) == ERROR)
            {
            printErr ("telnetd: error spawning %s child - status = 0x%x\n",
		      charBuf, errnoGet ());
	    interpDisconn (disconnArg);
            goto telnetd_error;
            }

	/* spawn the input process which transfers data from the client socket
	 * to the master pty. */

        sprintf(charBuf, "tTnetI%s%d", serviceName, connNum);
	if ((telnetInTaskId =  taskSpawn ( charBuf, telnetTaskPriority,
					   telnetTaskOptions,
					   telnetTaskStackSize,
					   (FUNCPTR)telnetInTask, 
                        		   (int)interpDisconn, disconnArg, 
                        		   slaveFd, client, masterFd, 
					   0, 0, 0, 0, 0)) != ERROR)
            continue;

	printErr ("telnetd: error spawning %s child - status = 0x%x\n",
		  charBuf, errnoGet ());
	taskDelete (telnetOutTaskId);
	interpDisconn (disconnArg);

telnetd_error:		/* try to do a tidy clean-up */
        telnetPtyFree(ptyName);
        if (client != 0)
	    close (client);
        if (masterFd != 0)
	    close (masterFd);
        if (slaveFd != 0)
	    close (slaveFd);
	}
    } /* telnetd */

/*******************************************************************************
*
* telnetServiceAdd - add telnet service
*
* This routine spawn the new telnet demon to service telnet connection requests
* arrives to the port. It takes the following arguments:
* .IP <serviceName>
* a string constant that specifies the name of the desired service (don't use a 
* variable value);
* .IP <serviceNum>
* the protocol port number assigned to the service;
* .IP <servicePriority>
* priority of the telnet demon ;
* .IP <interpConn>
* a pointer to the user supplied routine which connect the command interpreter 
* with the pty device created by the telent demon to service the telent 
* connection.
* .IP <interpConnArg>
* an argument that telnet demon passes to the interpConn();
* .IP <interpDisconn>
* a pointer to the user supplied routine which disconnect the command 
* interpreter from the pty device.
*
* RETURNS: OK or ERROR, if the service can't be added.
*/
STATUS telnetServiceAdd
    (
    char *          serviceName,     /* name of the service */
    int             serviceNum,	     /* port number of the service */
    int             servicePriority, /* priority of telnet demon */
    INTERP_CON_PROC interpConn,	     /* connects comm. interp. with telnet */
    int             interpConnArg,   /* argument for interpConn () */
    INTERP_DIS_PROC interpDisconn    /* disconnects comm. interp. from telnet */
    )
    {
    int  telnetdId;	/* task ID of telnetd task */
    char nameBuf [PTY_NAME_LEN];

    sprintf(nameBuf, "tTnet%sd", serviceName);
    telnetdId = taskSpawn (nameBuf, servicePriority,
			   telnetTaskOptions, telnetTaskStackSize,
			   (FUNCPTR)telnetd, (int)serviceName, serviceNum, 
                           (int)interpConn, interpConnArg, (int)interpDisconn, 
                           0, 0, 0, 0, 0);

    if (telnetdId == ERROR)
        {
	printErr ("telnetServiceAdd: unable to spawn demon to service port "
                  "%d.\n", serviceNum);
        return ERROR;
        }

    return OK;
    } /* telnetSeviceAdd */

/*******************************************************************************
*
* telnetInit - initialize the telnet support
*
* This routine initializes the telnet facility, which supports remote login
* to the VxWorks shell and the user specified command interpreters via the 
* telnet protocol.  It creates so many pty devices as necessary to support 
* number of telnet connection defined by its variable <maxConnNum>.  
*
* It is called automatically when INCLUDE_TELNET is defined in configAll.h.
*
* RETURNS: N/A
*/

void telnetInit 
    (
    int maxConnNum
    )
    {
    static BOOL done;	/* FALSE = not done */
    char nameBuf[PTY_NAME_LEN];
    int i;

    if (done)
	{
	printErr ("telnetInit: already initialized.\n");
	return;
	}

    if (ptyDrv () == ERROR)
	{
	printErr ("telnetInit: unable to initialize pty driver.\n");
	return;
	}

    telnetMaxConnNum = maxConnNum;
    telnetPtyDevs = malloc (sizeof (*telnetPtyDevs) * telnetMaxConnNum);
    for (i = 0; i < telnetMaxConnNum; i++)
        {
        sprintf (nameBuf, "%s%d.", ptyTelnetName, i);

        if (ptyDevCreate (nameBuf, 1024, 1024) == ERROR)
	    {
	    printErr ("telnetInit: unable to create pty device %d.\n", i);
	    return;
	    }
        telnetPtyDevs[i] = TELNET_PTY_FREE;
        }

    done = TRUE;
    } /* telnetInit */

#endif	/* defined(INCLUDE_TELNET) */

int _sal_telnet_not_empty;
