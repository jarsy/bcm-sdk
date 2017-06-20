/* $Id: ftpXfer2.h,v 1.3 2011/07/21 16:14:55 yshtil Exp $ */
#ifndef FTPXFER2_H
#define FTPXFER2_H

STATUS ftpXfer2(char *host, char *user, char *passwd, char *acct,
		char *cmd, char *dirname, char *filename,
		int *pCtrlSock, int *pDataSock);

#endif	/* FTPXFER2_H */
