/*
 * $Id: netio.h,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * netio module for VxWorks
 *
 *   Abstracts FTP or RSH to look like ordinary file I/O.
 */

#include <stdio.h>

#define NETIO_NAME_MAX		256

void	netio_defaults(char *user, char *pass, char *host);

int	netio_open(char *filespec, int oflag, ...);
int	netio_close(int fd);
int	netio_valid_fd(int fd);

FILE	*netio_fopen(char *filespec, char *fmode);
int	netio_fclose(FILE *fp);
int	netio_valid_fp(FILE *fp);

int	netio_ls(char *filename, char *flags);
int	netio_remove(char *filename);
int	netio_rename(char *oldname, char *newname);
int	netio_mkdir(char *path);
int	netio_rmdir(char *path);

/* Directories */

typedef struct {
    char	**names;
    int		names_alloc;
    int		names_used;
    int		names_ptr;
} NETIO_DIR;

struct netio_dirent {
    char	d_name[NETIO_NAME_MAX];
};

NETIO_DIR	*netio_opendir(char *dirName);
int		netio_closedir(NETIO_DIR *pDir);
struct netio_dirent *
		netio_readdir(NETIO_DIR *pDir);
void		netio_rewinddir(NETIO_DIR *pDir);
