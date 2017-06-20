#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <selectLib.h>
#include <errnoLib.h>
#include <ftpLib.h>

#include "config.h"
#include "sysLib.h"
#include "netinet/in.h"
#include "sockLib.h"
#include "ftpXfer2.h"

#define FTP_DATA_CONN_TIMEOUT		20	/* sec */

/* $Id: ftpXfer2.c,v 1.4 2011/07/21 16:14:08 yshtil Exp $
 * ftpXfer2
 *
 * This routine replaces VxWorks ftpXfer and doesn't have a bug that can
 * cause very small file transfers to fail.  In particular, it calls
 * select() on both the data and control sockets, and if they become
 * ready at the SAME TIME (as opposed to just the data socket), vxWorks
 * falsely assumes an error.
 */

BOOL ftpVerbose;

STATUS ftpXfer2(char *host, char *user, char *passwd, char *acct,
		char *cmd, char *dirname, char *filename,
		int *pCtrlSock, int *pDataSock)
{
    int			ctrlSock = ERROR, dataSock = ERROR;
    struct fd_set 	rfd;
    int			result;
    struct timeval	tmo;
    char		*errmsg = NULL;

    if (ftpVerbose)
	printf("ftpXfer2: hookup host=%s file=%s\n", host, filename);

    if ((ctrlSock = ftpHookup(host)) == ERROR) {
	errmsg = "server unreachable";
	goto error;
    }

    *pCtrlSock = ctrlSock;

    if (ftpVerbose)
	printf("ftpXfer2: login user=%s\n", user);

    if (ftpLogin(ctrlSock, user, passwd, acct) != OK) {
	errmsg = "authentication failed";
	goto error;
    }

    if (ftpVerbose)
	printf("ftpXfer2: set binary\n");

    if (ftpCommand(ctrlSock, "TYPE I", 0, 0, 0, 0, 0, 0) != FTP_COMPLETE) {
	errmsg = "set binary mode failed";
	goto error;
    }

    if (dirname[0]) {
	if (ftpVerbose)
	    printf("ftpXfer2: cd %s\n", dirname);

	if (ftpCommand(ctrlSock, "CWD %s",
		       (int) dirname, 0, 0, 0, 0, 0) != FTP_COMPLETE) {
	    errmsg = "change directory failed";
	    goto error;
	}
    }

    /*
     * Retry loop for transient errors, such as the remote host being
     * unable to assign the requested port number.
     */

 retry_transient_error:

    /*
     * If this is a transfer command requiring a data connection,
     * first establish socket for server to connect back to.
     */

    if (pDataSock) {
	int			len;
	short			port;
	struct sockaddr_in 	ownAddr;
	struct sockaddr_in 	dataAddr;

	if (ftpVerbose)
	    printf("ftpXfer2: set binary\n");

	/* Find out our own address */

	len = sizeof (ownAddr);
	if (getsockname(ctrlSock, (struct sockaddr *) &ownAddr, &len) < 0) {
	    errmsg = "FTP could not get own addr";
	    goto error;
	}

	if ((dataSock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
	    errmsg = "FTP could not create data socket";
	    goto error;
	}

	dataAddr = ownAddr;
	dataAddr.sin_port = htons(0);

	if (bind(dataSock,
		 (struct sockaddr *) &dataAddr,
		 sizeof (dataAddr)) != OK) {
	    close(dataSock);
	    errmsg = "FTP could not bind data socket";
	    goto error;
	}

	/* Read back to find out what port was bound */

	len = sizeof (dataAddr);
	if (getsockname(dataSock, (struct sockaddr *) &dataAddr, &len) < 0) {
	    errmsg = "FTP could not get data addr";
	    goto error;
	}

	port = ntohs(dataAddr.sin_port);

	if (listen(dataSock, 1) < 0) {
	    close(dataSock);
	    errmsg = "FTP could not listen on data socket";
	    goto error;
	}

	/* Use PORT command to inform server of data socket address */

	if (ftpCommand(ctrlSock,
		       "PORT %d,%d,%d,%d,%d,%d",
		       (int) ((UINT8 *) &dataAddr.sin_addr)[0],
		       (int) ((UINT8 *) &dataAddr.sin_addr)[1],
		       (int) ((UINT8 *) &dataAddr.sin_addr)[2],
		       (int) ((UINT8 *) &dataAddr.sin_addr)[3],
		       (int) (port >> 8),
		       (int) (port & 0xff)) != FTP_COMPLETE) {
	    close(dataSock);
	    errmsg = "FTP could not send PORT command";
	    goto error;
	}
    }

    /*
     * Send the FTP command.
     */

    if (ftpVerbose)
	printf("ftpXfer2: command %s\n", cmd);
    
    if (strlen(filename) < 128) {
        result = ftpCommand(ctrlSock, cmd, (int) filename, 0, 0, 0, 0, 0);
    } else {
        result = FTP_ERROR;
    }

    if (ftpVerbose)
	printf("ftpXfer2: result %d\n", result);

    switch (result) {
    case FTP_TRANSIENT:
	if (pDataSock)
	    close(dataSock);
	goto retry_transient_error;
    case FTP_COMPLETE:
    case FTP_CONTINUE:
	if (pDataSock) {
	    close(dataSock);
	    errmsg = "server returned COMPLETE or CONTINUE instead of PRELIM";
	    goto error;
	}
	return OK;	/* Non-transfer command succeeded */
    case FTP_PRELIM:
	if (! pDataSock) {
	    errmsg = "server returned PRELIM for non-transfer command";
	    goto error;
	}
	break;		/* Continue below to start transfer */
    default:
	if (pDataSock)
	    close(dataSock);
	errmsg = "command failed";
	goto error;
    }

    /*
     * Wait for server to connect back on data socket.
     * Use select to provide a timeout.
     */

    FD_ZERO(&rfd);
    FD_SET(dataSock, &rfd);

    tmo.tv_sec = FTP_DATA_CONN_TIMEOUT;
    tmo.tv_usec = 0;

    if (ftpVerbose)
	printf("ftpXfer2: wait for data\n");

    if (select(FD_SETSIZE, &rfd, NULL, NULL, &tmo) <= 0) {
	if (pDataSock)
	    close(dataSock);
	errmsg = "data conn failed or timed out";
	goto error;
    }

    if (ftpVerbose)
	printf("ftpXfer2: get data conn\n");

    if ((dataSock = ftpDataConnGet(dataSock)) == ERROR) {
	errmsg = "failed to accept server connection";
	goto error;
    }

    *pDataSock = dataSock;

    if (ftpVerbose)
	printf("ftpXfer2: return OK\n");

    return OK;

 error:

    if (errmsg && ftpVerbose)
	printErr("FTP ERROR: %s (errno=%d)\n", errmsg, errnoGet());

    if (ctrlSock != ERROR) {
	(void) ftpCommand(ctrlSock, "QUIT", 0, 0, 0, 0, 0, 0);
	close(ctrlSock);
    }

    if (ftpVerbose)
	printf("ftpXfer2: return ERROR\n");

    return ERROR;
}
