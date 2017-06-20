/*
 * $Id: netio.c,v 1.16 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * FTP/RSH Module
 *
 * Abstracts VxWorks FTP and RSH modules to look like ordinary file I/O.
 *
 * Permissible filename syntaxes are:
 *
 *	user%pass@host:file
 *	user@host:file
 *	host:file
 *	file
 *
 * If user or host is missing, the defaults are used and are required.
 *
 * If the user given is "tftp" and the password is blank, then tftp is used.
 * If no password is given and there is no default password, RSH is
 * used; otherwise, FTP is used.
 *
 * Bug: For RSH, errors cannot be detected easily.  If an error occurs
 * while opening a file for reading or writing, the open succeeds.
 */

#include <shared/bsl.h>

#include <vxWorks.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>

#include <ftpLib.h>
#include <tftpLib.h>
#include <remLib.h>
#include <bootLib.h>
#include "config.h"

#include <assert.h>
#include <sal/appl/io.h>

#define DPRINT		if (0) bsl_printf

#include <soc/debug.h>

#if defined(USE_FTP2)
/*
 * Use enhanced version of ftpXfer that actually works.
 * The include file comes from the BSP directory.
 */
#include "ftpXfer2.h"
#define ftpXfer ftpXfer2
#endif /* USE_FTP2 */

#include "netio.h"

/*
 * Structure to keep track of transfers in progress
 */

#define MAX_NETIO_OPEN	5

typedef struct NETIO_FILE {
    int		entry_used;
    int		method;
#define	METHOD_RSH	1
#define	METHOD_TFTP	2
#define	METHOD_FTP	3
    FILE	*data_fp;
    int		ctrl_fd;
    int		data_fd;
} NETIO_FILE;

static NETIO_FILE netio_iob[MAX_NETIO_OPEN];

/*
 * netio_defaults
 *
 *   Routine to set default user, password, and host.  This routine is
 *   optional.  If used, allows any of these three fields to be omitted
 *   from file specifications.
 */

char *netio_dfl_user = NULL;
char *netio_dfl_pass = "";
char *netio_dfl_host = NULL;

void netio_defaults(char *user, char *pass, char *host)
{
    if (user)
	netio_dfl_user = strcpy(malloc(strlen(user) + 1), user);
    if (pass)
	netio_dfl_pass = strcpy(malloc(strlen(pass) + 1), pass);
    if (host)
	netio_dfl_host = strcpy(malloc(strlen(host) + 1), host);
}

/*
 * netio_split
 *
 *   Utility routine to break a file name into directory and file
 *   portions.  Works well on weird paths.  Modifies input string.
 */

void netio_split(char *path, char **dir, char **file)
{
    char		*last_slash;

    if ((last_slash = strrchr(path, '/')) == 0) {
	*dir = ".";
	*file = path;
    } else if (last_slash == path) {
	*dir = "/";
	*file = (path[1] ? &path[1] : ".");
    } else {
	*last_slash = 0;
	*dir = path;
	*file = (last_slash[1] ? &last_slash[1] : ".");
    }
}

/*
 * netio_breakline
 *
 *   Utility routine to parse filename syntax and supply defaults,
 *   if any.
 */

int netio_breakline(char *line,
		    char **host,
		    char **user,
		    char **file,
		    char **pass,
		    int *methodp)
{
    char		*percent, *atsign, *colon;
    char		*methods;

    if (line == NULL) {
	DPRINT("netio: Null filename\n");
	errno = ENOENT;
	return -1;
    }

    /*
     * Syntaxes supported (repeated from comment at top of file):
     *
     *	user%pass@host:file
     *	user@host:file
     *	host:file
     *	file
     */

    *user = netio_dfl_user;
    *pass = netio_dfl_pass;
    *host = netio_dfl_host;

    if ((colon = strchr(line, ':')) != 0)
	*colon++ = 0;

    if ((atsign = strchr(line, '@')) != 0)
	*atsign++ = 0;

    if ((percent = strchr(line, '%')) != 0)
	*percent++ = 0;

    if (percent && atsign && colon) {
	*user = line;
	*pass = percent;
	*host = atsign;
	*file = colon;
    } else if (! percent && atsign && colon) {
	*user = line;
	*host = atsign;
	*file = colon;
    } else if (! percent && ! atsign && colon) {
	*host = line;
	*file = colon;
    } else if (! percent && ! atsign && ! colon) {
	*file = line;
    } else {
	DPRINT("netio: Filename syntax error (user[%%pass]@host:file)\n");
	errno = ENOENT;
	return -1;
    }

    if (!*user || !*pass || !*host || !*file) {
	DPRINT("netio: Insufficient filename spec "
	       "(user[%%pass]@host:file)\n");
	errno = ENOENT;
	return -1;
    }

    if (**file == 0)
	*file = ".";

    if (**pass == 0) {
	if (strcmp(*user, "tftp") == 0) {
	    *methodp = METHOD_TFTP;
	    methods = "tftp";
	} else {
	    *methodp = METHOD_RSH;
	    methods = "rsh";
	}
    } else {
	*methodp = METHOD_FTP;
	methods = "ftp";
    }

    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META("netio_breakline:\n")));
    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META("  File:   %s\n"), *file));
    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META("  User:   %s\n"), *user));
    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META("  Pass:   %s\n"), *pass));
    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META("  Host:   %s\n"), *host));
    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META("  Method: %s\n"), methods));

    return 0;
}

/*
 * netio_open
 *
 *   This routine is functionally similar to open.  The resulting file
 *   pointer can be used with the read and write routines.  It must be
 *   closed with netio_close, not close.  It is not legal to seek.
 */

int netio_open(char *filespec, int oflag, ...)
{
    char		*pass, *host, *file, *user;
    char		cmd[256];
    NETIO_FILE		*ff;
    int			i;

    DPRINT("netio_open: spec=%s oflag=%d\n", filespec, oflag);

    if ((oflag & 3) != O_RDONLY && (oflag & 3) != O_WRONLY) {
	errno = EINVAL;
	return -1;		/* Not supported */
    }

    /*
     * Find a free slot in the table of outstanding transfers
     */

    for (i = 0; i < MAX_NETIO_OPEN; i++)
	if (! netio_iob[i].entry_used)
	    break;

    if (i == MAX_NETIO_OPEN) {		/* Too many outstanding */
	errno = EMFILE;
	return -1;
    }

    ff = &netio_iob[i];

    if (netio_breakline(filespec, &host, &user, &file, &pass, &ff->method) < 0)
	return -1;

    /*
     * If password is non-empty, use ftpLib to open a connection for
     * reading, writing, or appending to the file.  Otherwise, use
     * remLib.
     */

    switch (ff->method) {
    case METHOD_RSH:
	if ((oflag & 3) == O_RDONLY)
	    sprintf(cmd, "cat %s" , file);
	else if (oflag & O_APPEND)
	    sprintf(cmd, "cat >> %s", file);
	else
	    sprintf(cmd, "cat > %s" , file);

	if ((ff->data_fd = rcmd(host, 514, user, user,
				cmd, &ff->ctrl_fd)) == ERROR) {
	    DPRINT("netio_open: RSH transfer failed for host %s\n", host);
	    errno = EIO;
	    return -1;
	}
	break;
    case METHOD_TFTP:
	if ((oflag & 3) == O_RDONLY)
	    strcpy(cmd, "get");
	else if (oflag & O_APPEND)
	    strcpy(cmd, "put");		
	else
	    strcpy(cmd, "put");

	if (tftpXfer(host, 0, file, cmd, "binary",
		    &ff->data_fd, &ff->ctrl_fd) == ERROR) {
	    DPRINT("netio_open: TFTP transfer failed for host %s\n", host);
	    errno = EIO;
	    return -1;
	}
	break;
    case METHOD_FTP:
	if ((oflag & 3) == O_RDONLY)
	    strcpy(cmd, "RETR %s");
	else if (oflag & O_APPEND)
	    strcpy(cmd, "APPE %s");
	else
	    strcpy(cmd, "STOR %s");

	if (ftpXfer(host, user, pass, "", cmd, "", file,
		    &ff->ctrl_fd, &ff->data_fd) == ERROR) {
	    DPRINT("netio_open: FTP transfer failed for host %s\n", host);
	    errno = EIO;
	    return -1;
	}
	break;
    }

    ff->entry_used = 1;

    DPRINT("netio_open: fd=%d\n", ff->data_fd);

    return ff->data_fd;
}

/*
 * netio_close
 *
 *   Closes a file handle returned from netio_open.  Do not use close
 *   directly.
 */

int netio_close(int fd)
{
    NETIO_FILE		*ff;
    int			i, rc = 0;

    DPRINT("netio_close: fd=%d\n", fd);

    /* Search for entry in table of outstanding transfers */

    for (i = 0; i < MAX_NETIO_OPEN; i++)
	if (netio_iob[i].data_fd == fd)
	    break;

    if (i == MAX_NETIO_OPEN) {
	errno = EBADF;
	return -1;
    }

    ff = &netio_iob[i];

    /* Data connection must be closed in order to receive FTP reply */

    if (close(ff->data_fd) < 0)
	rc = -1;

    switch (ff->method) {
    case METHOD_RSH:
	break;
    case METHOD_TFTP:
	break;
    case METHOD_FTP:
	/*
	 * Don't check ftpReplyGet for FTP_COMPLETE.  That would cause
	 * an error if an FTP connection is closed prematurely, which is
	 * not illegal and does happen.
	 */

	(void) ftpReplyGet(ff->ctrl_fd, TRUE);

	if (ftpCommand(ff->ctrl_fd, "QUIT",
		       0, 0, 0, 0, 0, 0) != FTP_COMPLETE) {
	    DPRINT("netio_fclose: Error receiving FTP QUIT completion");
	    rc = -1;
	}
	break;
    }

    close(ff->ctrl_fd);

    ff->entry_used = 0;

    errno = EIO;	/* In case rc < 0 */

    return rc;
}

/*
 * netio_valid_fd
 *
 *  Convenient utility routine allowing an application to determine
 *  whether or not a file descriptor was opened using netio_open.
 */

int netio_valid_fd(int fd)
{
    int			i;

    for (i = 0; i < MAX_NETIO_OPEN; i++)
	if (netio_iob[i].entry_used && netio_iob[i].data_fd == fd)
	    return 1;

    return 0;
}

/*
 * netio_fopen
 *
 *   This routine is functionally similar to fopen.  The resulting file
 *   pointer can be used with stdio read and write routines.  It must be
 *   closed with netio_fclose, not fclose.  It is not legal to fseek.
 */

FILE *netio_fopen(char *filespec, char *fmode)
{
    NETIO_FILE		*ff;
    int			i, fd;
    int			oflag = 0;

    DPRINT("netio_fopen: spec=%s mode=%s\n", filespec, fmode);

    switch (fmode[0]) {
    case 'r':
	oflag = O_RDONLY;
	break;
    case 'w':
	oflag = O_WRONLY;
	break;
    case 'a':
	oflag = O_WRONLY | O_APPEND;
	break;
    default:
	return NULL;
    }

    if ((fd = netio_open(filespec, oflag)) < 0)
	return NULL;

    /* Search for entry in table of outstanding transfers */

    for (i = 0; i < MAX_NETIO_OPEN; i++)
	if (netio_iob[i].data_fd == fd)
	    break;

    assert (i < MAX_NETIO_OPEN);	/* Must be found */

    ff = &netio_iob[i];

    if ((ff->data_fp = fdopen(ff->data_fd, fmode)) == NULL) {
	perror("fdopen");
	netio_close(ff->data_fd);
	return NULL;
    }

    DPRINT("netio_fopen: fp=%p\n", (void *)ff->data_fp);

    return ff->data_fp;
}

/*
 * netio_fclose
 *
 *   Closes a file pointer returned from netio_fopen.  Do not use fclose
 *   directly.
 */

int netio_fclose(FILE *fp)
{
    NETIO_FILE		*ff;
    int			i, rc = 0;

    DPRINT("netio_fclose: fp=%p\n", (void *)fp);

    /* Search for entry in table of outstanding transfers */

    for (i = 0; i < MAX_NETIO_OPEN; i++)
	if (netio_iob[i].data_fp == fp)
	    break;

    if (i == MAX_NETIO_OPEN) {
	errno = EBADF;
	return -1;
    }

    ff = &netio_iob[i];

    /*
     * Flush before closing file descriptor
     */

    fflush(ff->data_fp);

    if (netio_close(ff->data_fd) < 0)
	rc = -1;

    /*
     * fclose will try to close ff->data_fd a second time.  Try to
     * prevent a problem with that by giving it something to close.
     */

    ff->data_fp->_file = open("/null", 0, 0);

    if (fclose(ff->data_fp) < 0)
	rc = -1;

    ff->data_fp = NULL;

    return rc;
}

/*
 * netio_valid_fp
 *
 *  Convenient utility routine allowing an application to determine
 *  whether or not a FILE * was opened using netio_fopen.
 */

int netio_valid_fp(FILE *fp)
{
    int			i;

    for (i = 0; i < MAX_NETIO_OPEN; i++)
	if (netio_iob[i].data_fp == fp)
	    return 1;

    return 0;
}

/*
 * netio_ls
 *
 *  Get a file listing and display output on stdout.
 */

int netio_ls(char *path, char *flags)
{
    char		*pass, *host, *file, *user, *dir, *c;
    int			rc = 0;
    int			c_fd, d_fd;		/* control/data FDs */
    char		buf[256];
    char		cmd[256];
    FILE		*f;
    int			f_on_line;
    int			method;
    int			one_per_line;

    if (flags == NULL)
	flags = "";

    if (netio_breakline(path, &host, &user, &file, &pass, &method))
	return(-1);

    one_per_line = ((method == METHOD_RSH) || strchr(flags, 'l') != NULL);

    switch (method) {
    case METHOD_RSH:
	sprintf(cmd, "ls -C %s %s", flags, file);

	if ((d_fd = rcmd(host, 514, user, user, cmd, &c_fd)) == ERROR) {
	    DPRINT("netio_ls: RSH connection failed to host %s\n", host);
	    errno = EIO;
	    return -1;
	}
	break;
    case METHOD_TFTP:
	DPRINT("netio_ls: cannot list files on TFTP connections\n");
	errno = EIO;
	return(-1);
	break;
    case METHOD_FTP:
	netio_split(file, &dir, &file);

	sprintf(cmd, "NLST %s%s%s", flags, flags[0] ? " " : "", file);

	if (ftpXfer(host, user, pass, "", cmd,
		    dir, "" /* not used */,
		    &c_fd, &d_fd) == ERROR) {
	    DPRINT("netio_ls: FTP connection failed to host %s\n", host);
	    errno = EIO;
	    return(-1);
	}
	break;
    }

    f = fdopen(d_fd, "r");
    f_on_line = 0;

    while (fgets(buf, sizeof(buf), f)) {
	if ((c = strchr(buf, '\n'))) {
	    *c = '\0';
	}
	if ((c = strchr(buf, '\r'))) {
	    *c = '\0';
	}

	f_on_line++;

	if (one_per_line) {
	    cli_out("%s\n", buf); /* Normal for "ls -l " */
	} else if (! (f_on_line % 4))
	    cli_out("%s\n", buf);
	else
	    cli_out("%-19s ", buf);
    }

    if (! one_per_line && (f_on_line % 4)) {
	cli_out("\n");
    }

    fclose(f);

    if (method == METHOD_FTP) {
	if (ftpReplyGet(c_fd, TRUE) != FTP_COMPLETE) {
	    perror("ERROR receiving FTP reply");
	    rc = -1;
	}

	if (ftpCommand(c_fd, "QUIT", 0, 0, 0, 0, 0, 0) != FTP_COMPLETE) {
	    perror("ERROR receiving FTP QUIT completion");
	    rc = -1;
	}
    }

    close(c_fd);

    /*
     * Note: if 'ls' command fails on remote host, e.g. for file not
     * found, the ftpXfer above fails and returns -1, but the rcmd
     * doesn't.
     */

    errno = EIO;	/* In case rc < 0 */

    return(rc);
}

/*
 * netio_remove
 *
 *   Delete a file.
 */

int
netio_remove(char *path)
{
    char		*pass, *host, *file, *user;
    char		cmd[256];
    int			rc = -1, c_fd = -1, d_fd = -1;
    int			method;

    if (netio_breakline(path, &host, &user, &file, &pass, &method))
	goto done;

    switch (method) {
    case METHOD_RSH:
	sprintf(cmd, "rm -f - %s", file);

	if ((d_fd = rcmd(host, 514, user, user, cmd, &c_fd)) == ERROR) {
	    DPRINT("netio_remove: RSH connection failed to host %s\n", host);
	    goto done;
	}
	break;
    case METHOD_TFTP:
	DPRINT("netio_remove: cannot remove files on TFTP connections\n");
	errno = EIO;
	return(-1);
	break;
    case METHOD_FTP:
	if (ftpXfer(host, user, pass, "", "DELE %s", "", file,
		    &c_fd, NULL) == ERROR) {
	    DPRINT("netio_remove: FTP connection failed to host %s\n", host);
	    goto done;
	}
	break;
    }

    rc = 0;

 done:

    if (c_fd >= 0) {
	if (method == METHOD_FTP)	/* ftp */
	    (void) ftpCommand(c_fd, "QUIT", 0, 0, 0, 0, 0, 0);
	close(c_fd);
    }

    if (d_fd >= 0)
	close(d_fd);

    errno = EIO;	/* In case rc < 0 */

    return(rc);
}

/*
 * netio_rename
 *
 *   Rename a file.  The old_path is a usual netio filename spec, but
 *   the destination name must be a plain path without %, @, or :.
 */

int netio_rename(char *old_path, char *new_path)
{
    char		*pass, *host, *file, *user, *st;
    char		cmd[256];
    int			rc = -1, c_fd = -1, d_fd = -1, len;
    int			method;

    if (netio_breakline(old_path, &host, &user, &file, &pass, &method))
	goto done;

    if (new_path == NULL) {
	DPRINT("netio_rename: Null destination filename\n");
	errno = EINVAL;
	return -1;
    }

    switch (method) {
    case METHOD_RSH:
	sprintf(cmd, "mv -f %s %s", file, new_path);
	if ((d_fd = rcmd(host, 514, user, user, cmd, &c_fd)) == ERROR) {
	    DPRINT("netio_rename: RSH connection failed to host %s\n", host);
	    errno = EIO;
	    return -1;
	}
	break;
    case METHOD_TFTP:
	DPRINT("netio_rename: cannot rename files on TFTP connections\n");
	errno = EIO;
	return(-1);
	break;
    case METHOD_FTP:
	if ((st = strrchr(file, '/'))) {
	    len = PTR_TO_INT(st) - PTR_TO_INT(file) + 1;
	    strncpy(cmd, file, len);
	    cmd[len] = '\0';
	}
	if (ftpXfer(host, user, pass, "", "TYPE I", st ? cmd : "", 0,
		    &c_fd, NULL) == ERROR) {
	    DPRINT("netio_rename: FTP(TYPE I) connection failed to host %s\n", host);
	    goto done;
	}

	if (ftpCommand(c_fd, "RNFR %s", PTR_TO_INT(file),
		       0, 0, 0, 0, 0) != FTP_CONTINUE) {
	    DPRINT("netio_rename: FTP(RNFR) completion failed to host %s\n", host);
	    goto done;
	}
	if (ftpCommand(c_fd, "RNTO %s", PTR_TO_INT(new_path),
		       0, 0, 0, 0, 0) != FTP_COMPLETE) {
	    DPRINT("netio_rename: FTP(RNTO) completion failed to host %s\n", host);
	    goto done;
	}
	break;
    }

    rc = 0;

 done:

    if (c_fd >= 0) {
	if (method == METHOD_FTP)
	    (void) ftpCommand(c_fd, "QUIT", 0, 0, 0, 0, 0, 0);
	close(c_fd);
    }

    if (d_fd >= 0)
	close(d_fd);

    errno = EIO;	/* In case rc < 0 */

    return(rc);
}

/*
 * netio_mkdir
 *
 *   Make a directory.
 */

int netio_mkdir(char *path)
{
    char		*pass, *host, *file, *user;
    char		cmd[256];
    int			rc = -1, c_fd = -1, d_fd = -1;
    int			method;

    if (netio_breakline(path, &host, &user, &file, &pass, &method))
	goto done;

    switch (method) {
    case METHOD_RSH:
	sprintf(cmd, "mkdir %s", file);
	if ((d_fd = rcmd(host, 514, user, user, cmd, &c_fd)) == ERROR) {
	    DPRINT("netio_mkdir: RSH connection failed to host %s\n", host);
	    errno = EIO;
	    return -1;
	}
	break;
    case METHOD_TFTP:
	DPRINT("netio_mkdir: cannot make directories on TFTP connections\n");
	errno = EIO;
	return(-1);
	break;
    case METHOD_FTP:
	if (ftpXfer(host, user, pass, "", "MKD %s", "", file,
		    &c_fd, NULL) == ERROR) {
	    DPRINT("netio_mkdir: FTP connection failed to host %s\n", host);
	    goto done;
	}
	break;
    }

    rc = 0;

 done:

    if (c_fd >= 0) {
	if (method == METHOD_FTP)
	    (void) ftpCommand(c_fd, "QUIT", 0, 0, 0, 0, 0, 0);
	close(c_fd);
    }

    if (d_fd >= 0)
	close(d_fd);

    errno = EIO;	/* In case rc < 0 */

    return(rc);
}

/*
 * netio_rmdir
 *
 *   Remove a directory.
 */

int netio_rmdir(char *path)
{
    char		*pass, *host, *file, *user;
    char		cmd[256];
    int			rc = -1, c_fd = -1, d_fd = -1;
    int			method;

    if (netio_breakline(path, &host, &user, &file, &pass, &method))
	goto done;

    switch (method) {
    case METHOD_RSH:
	sprintf(cmd, "rmdir %s", file);
	if ((d_fd = rcmd(host, 514, user, user, cmd, &c_fd)) == ERROR) {
	    DPRINT("netio_rmdir: RSH connection failed to host %s\n", host);
	    errno = EIO;
	    return -1;
	}
	break;
    case METHOD_TFTP:
	DPRINT("netio_rmdir: cannot remove directories on TFTP connections\n");
	errno = EIO;
	return(-1);
	break;
    case METHOD_FTP:
	if (ftpXfer(host, user, pass, "", "RMD %s", "", file,
		    &c_fd, NULL) == ERROR) {
	    DPRINT("netio_rmdir: FTP connection failed to host %s\n", host);
	    goto done;
	}
	break;
    }


    rc = 0;

 done:

    if (c_fd >= 0) {
	if (method == METHOD_FTP)
	    (void) ftpCommand(c_fd, "QUIT", 0, 0, 0, 0, 0, 0);
	close(c_fd);
    }

    if (d_fd >= 0)
	close(d_fd);

    errno = EIO;	/* In case rc < 0 */

    return(rc);
}

/*
 * netio_opendir
 *
 *   This routine is functionally similar to opendir.  The resulting DIR
 *   pointer can be used with netio_readdir and netio_closedir.
 *
 *   This routine transfers the entire directory and buffers it for call
 *   to readdir.
 */
NETIO_DIR *
netio_opendir(char *name)
{
    char		*pass, *host, *file, *user, *dir, *c;
    NETIO_DIR		*dirp = NULL;
    int			c_fd = -1, d_fd= -1;	/* control/data FDs */
    char		buf[256];
    char		cmd[256];
    FILE		*f = NULL;
    int			method;

    if (netio_breakline(name, &host, &user, &file, &pass, &method))
	goto error;

    switch (method) {
    case METHOD_RSH:
	sprintf(cmd, "ls -1 %s", file);

	if ((d_fd = rcmd(host, 514, user, user, cmd, &c_fd)) == ERROR) {
	    DPRINT("netio_opendir: RSH connection failed to host %s\n", host);
	    goto error;
	}
	break;
    case METHOD_TFTP:
	DPRINT("netio_opendir: cannot open directories on TFTP connections\n");
	errno = EIO;
	return NULL;
	break;
    case METHOD_FTP:
	netio_split(file, &dir, &file);

	sprintf(cmd, "NLST %s", file);

	if (ftpXfer(host, user, pass, "", cmd,
		    dir, "" /* not used */,
		    &c_fd, &d_fd) == ERROR) {
	    DPRINT("netio_opendir: FTP connection failed to host %s\n", host);
	    goto error;
	}
	break;
    }

    f = fdopen(d_fd, "r");

    if ((dirp = malloc(sizeof (*dirp))) == NULL)
	goto error;

    memset(dirp, 0, sizeof (*dirp));

    dirp->names_alloc = 32;
    dirp->names_used = 0;
    dirp->names_ptr = 0;

    if ((dirp->names = malloc(dirp->names_alloc * sizeof (char *))) == NULL)
	goto error;

    while (fgets(buf, sizeof(buf), f)) {
	if ((c = strchr(buf, '\n')))
	    *c = 0;
	if ((c = strchr(buf, '\r')))
	    *c = 0;

	if (dirp->names_used == dirp->names_alloc) {
	    dirp->names_alloc *= 2;

	    dirp->names =
		realloc(dirp->names, dirp->names_alloc * sizeof (char *));

	    if (dirp->names == NULL)
		goto error;
	}

	dirp->names[dirp->names_used] = malloc(strlen(buf) + 1);

	if (dirp->names[dirp->names_used] == NULL)
	    goto error;

	strcpy(dirp->names[dirp->names_used], buf);

	dirp->names_used++;
    }

    fclose(f);
    f = 0;

    if (method == METHOD_FTP) {
	if (ftpReplyGet(c_fd, TRUE) != FTP_COMPLETE) {
	    perror("ERROR receiving FTP reply");
	    goto error;
	}

	if (ftpCommand(c_fd, "QUIT", 0, 0, 0, 0, 0, 0) != FTP_COMPLETE) {
	    perror("ERROR receiving FTP QUIT completion");
	    goto error;
	}
    }

    close(c_fd);
    c_fd = -1;

    /*
     * Note: if 'ls' command fails on remote host, e.g. for file not
     * found, the ftpXfer above fails and returns -1, but the rcmd
     * doesn't.
     */

    return dirp;

 error:
    if (f != NULL) {
	fclose(f);
	d_fd = -1;
    }

    if (d_fd >= 0)
	close(d_fd);

    if (c_fd >= 0)
	close(c_fd);

    netio_closedir(dirp);

    return NULL;
}

/*
 * netio_readdir
 *
 *   This routine is functionally similar to readdir.  It returns an
 *   equivalent type "netio_dirent" that has a d_name field.
 */
struct netio_dirent *
netio_readdir(NETIO_DIR *dirp)
{
    static struct netio_dirent dir;

    if (dirp->names_ptr < dirp->names_used) {
	strcpy(dir.d_name, dirp->names[dirp->names_ptr++]);
	return &dir;
    }

    return NULL;
}

/*
 * netio_closedir
 *
 *   This routine is functionally similar to closedir.
 */
int
netio_closedir(NETIO_DIR *dirp)
{
    if (dirp != NULL) {
	if (dirp->names != NULL) {
	    while (dirp->names_used > 0) {
		dirp->names_used--;
		free(dirp->names[dirp->names_used]);
	    }

	    free(dirp->names);
	}

	free(dirp);
    }

    return 0;
}

/*
 * netio_rewinddir
 *
 *   This routine is functionally similar to rewinddir.
 */
void
netio_rewinddir(NETIO_DIR *dirp)
{
    dirp->names_ptr = 0;
}
