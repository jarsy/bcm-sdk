/*
 * $Id: fileio.c,v 1.23 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        fileio.c
 * Purpose:     File I/O
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

#define __PROTOTYPE_5_0         /* Get stdarg prototypes for logMsg */
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
#include <tyLib.h>
#include <shellLib.h>
#include <selectLib.h>
#include <dirent.h>
#include "config.h"                     /* For BOOT_LINE_ADRS */

#include <sal/appl/sal.h>
#include <sal/appl/io.h>
#include <sal/appl/vxworks/hal.h>

#ifdef SAL_NO_FLASH_FS
#else
#include "flashFsLib.h"
#endif

#include "netio.h"

char    defaultdir_path[256];           /* Default (home) directory */
char    cwd_path[256];                  /* Current Working Directory */

/* This variable is managed from the diag shell */
int     _diag_nfs_mounts;

/*
 * sal_flash_sync
 *
 * Sync routine to call after each file operation to prevent loss of
 * data.  This is extremely important on the primitive Mousse flashFs.
 */

int
sal_flash_sync(void)
{
#ifdef SAL_NO_FLASH_FS
    cli_out("Platform does not support flash filesystem.\n");
    return -1;
#else
    if (!SAL_IS_PLATFORM_INFO_VALID) {
        cli_out("Platform not attached\n");
        return -1;
    }

    if (platform_info->caps & PLATFORM_CAP_FLASH_FS) {
        flashFsSync();
    }
    return 0;
#endif        
}

/*
 * sal_normalize_file
 *
 * Canonicalize a filename, simplifying occurrences of / and . and ..
 *
 * This procedure is a little complicated and has been regressed through
 * a large set of test cases.
 */

static void
sal_normalize_file(char *path)
{
    char        *s, *d;
    int         dot, dotdot;

    /*
     * Ignore portion of path before ':', if any.
     * Handles 'flash:' as well as netio files.
     */

    if ((s = strchr(path, ':')) != NULL)
        path = s + 1;

    /* Suck path elements, performing simulated chdirs on path forming at d */

    s = d = path;

    /* If absolute path, copy leading / */

    if (*s == '/')
        *d++ = *s++;

    while (*s != 0) {
        dot    = (s[0] == '.' && (s[1] == 0 || s[1] == '/'));
        dotdot = (s[0] == '.' && s[1] == '.' && (s[2] == 0 || s[2] == '/'));

        if (*s == '/' || dot) {
            s++;                /* Null path element, or chdir to . */
        } else if (dotdot && d == path) {
            *d++ = '.';         /* Retain .. at beginning of path */
            *d++ = '.';
            s += 2;
        } else if (dotdot && d == path + 1 && path[0] == '/') {
            /* Ignore .. if at top of absolute path */
            s += 2;
        } else if (dotdot && (d <= path + 1 || d[-1] != '.' || d[-2] != '.')) {
            /* Discard previous path element, only if it is not .. */
            while (--d > path && *d != '/')
                ;
            if (d == path && path[0] == '/')
                d++;    /* Beginning of absolute path keeps / */
            *d = 0;     /* Truncate rest of path element */
            s += 2;
        } else {
            /* Copy plain path element, adding / if not the first */
            if (d > path && ! (d == path + 1 && path[0] == '/'))
                *d++ = '/';
            while (s[0] != '/' && s[0] != 0)
                *d++ = *s++;
        }
    }

    if (d == path)
        *d++ = '.';

    *d = 0;
}

#define SAL_NETIO_FILE(_s)      (0 != strchr((_s), '@'))

int
sal_expand_file(char *file, char *fname)
{
    char        *c;

    if (*file == '/') {         /* ABS in file system */
        strcpy(fname, file);    /* (allows /null, etc. to work) */
    } else if (NULL != (c = strchr(file, ':'))) {
        strcpy(fname, file);    /* FS and file name */
    } else {                    /* Relative in file system */
        strcpy(fname, cwd_path);
        strcat(fname, file);
    }
    return(0);
}

/*
 * Set default directory (for cd with no argument)
 */
int
sal_homedir_set(char *dir)
{
    char                *buf;

    buf = defaultdir_path;

    if (dir != NULL) {
        strcpy(buf, dir);
    } else {
        BOOT_PARAMS     p;
        char            *s, *u;

        /*
         * Figure out our current working directory based on the VxWorks
         * boot line.
         */

        bootStringToStruct(BOOT_LINE_ADRS, &p);

        /*
         * If boot string begins with "/flash/", then CWD is on the
         * flash disk, otherwise, we assume it is netio.
         */

        if (p.flags & 0x80) {
            u = "tftp";
        } else {
            u = p.usr;
        }
        if (!strncmp(p.bootFile, "/flash/", 7)) {
            strcpy(buf, "flash:");
            strcat(buf, p.bootFile + 6);
        } else if (!strncmp(p.bootFile, "flash:", 6)) {
            strcpy(buf, p.bootFile);
        } else if (!strncmp(p.bootDev, "flash", 5)) {
            strcpy(buf, "flash:");
        } else if (!strncmp(p.bootFile, "flsh:", 5)) {
            strcpy(buf, p.bootFile);
        } else if (!strncmp(p.bootDev, "flsh", 4)) {
            strcpy(buf, "flsh:");
        } else {
            sprintf(buf, "%s%s%s@%s:%s",
                    u,
                    p.passwd[0] ? "%" : "",
                    p.passwd,
                    p.hostName,
                    p.bootFile);
        }

        netio_defaults(u, p.passwd, p.hostName);

        /* Nuke file name - leaving just directory */

        for (s = buf + strlen(buf);
             s > buf && s[-1] != '/' && s[-1] != ':';
             s--) {
        }

        *s = 0;
    }

    return 0;
}

char *
sal_homedir_get(char *buf, size_t size)
{
    strncpy(buf, defaultdir_path, size);
    buf[size - 2] = 0;

    if (buf[strlen(buf) - 1] != '/') {
        strcat(buf, "/");
    }

    return buf;
}

/*
 * Get current working directory.
 */
char *sal_getcwd(char *buf, size_t size)
{
    return(strncpy(buf, cwd_path, size));
}

/*
 * Set current working directory, it is stupid in that is just sets
 * a prefix for open/close/ and friends.
 *
 * If dir is NULL, changes to default (home) directory.
 */
int sal_cd(char *dir)
{
    char        *c;
    char        buf[256];

    if (dir == NULL) {
        dir = sal_homedir_get(buf, sizeof (buf));
    }

    if (_diag_nfs_mounts > 0) {
        /*
         * Normal behavior
         */
        if (dir[0] == '/' || strchr(dir, ':') != NULL) {
            /* Absolute path */
            strcpy(cwd_path, dir);
        } else {
            /* Append relative path */
            c = cwd_path + strlen(cwd_path);
            strcpy(c, dir);
        }
    } else {
        /*
         * Backward-compatible behavior when nothing is NFS-mounted
         *
         * The following ensures that one can cd to an FTP path such as:
         *              tornado%tornado+@borg:/home/user
         * and that, if the CWD already has the FTP spec in it, then
         *              cd /home/tornado
         * results in a path
         *              tornado%tornado+@borg:/home/tornado
         */
        if (strchr(dir, ':')) {         /* Total path given */
            strcpy(cwd_path, dir);
        } else {
            if (NULL == (c = strchr(cwd_path, ':'))) { /* Need abs path */
                cli_out("Absolute path required with ':'\n");
                return(-1);
            }
            if (*dir == '/') {  /* Absolute path in current file system */
                c++;
            } else {            /* just add to end */
                c += strlen(c);
            }
            strcpy(c, dir);
        }
    }

    sal_normalize_file(cwd_path);

    /* Be sure string ends in "/" */

    strcat(cwd_path, "/");              /* Add 1 */
    c = cwd_path + strlen(cwd_path) - 1; /* Move to end */
    while ((c > cwd_path) && (*c == '/')) {
        c--;
    }
    *(c + 2) = '\0';

    return(0);
}

/*
 * Do an "ls"
 */
int sal_ls(char *f, char *flags)
{
    char fname[256];
    int rv;

    if (sal_expand_file(f, fname)) {
        cli_out("Failed to expand file name: \"%s\"\n", f);
        rv = -1;
    } else if (SAL_NETIO_FILE(fname)) {
        rv = netio_ls(fname, flags);
    } else {
#ifdef SAL_NO_FLASH_FS
        cli_out("Platform does not support flash filesystem.\n");
        return -1;
#else
        char *fl_devname;
        int long_fmt = (flags != NULL && strchr(flags, 'l') != NULL);
        rv = (ls(fname, long_fmt) != OK) ? -1 : 0;

        if (long_fmt && SAL_IS_PLATFORM_INFO_VALID && 
            platform_info->f_flash_device_name && 
            ((fl_devname = platform_info->f_flash_device_name()) != NULL) &&
            (sal_strcasecmp(fname, fl_devname) == 0)) {
            int fd;
            uint32 free_bytes;
            if ((fd = open("flash:", 2, 0)) >= 0) {
                if (ioctl(fd, FIONFREE, PTR_TO_INT(&free_bytes)) >= 0) {
                    cli_out("Free space: %u bytes\n", free_bytes);
                }
                close(fd);
            }
        }
#endif
    }

    if (rv < 0) {
        cli_out("ls: Listing failed.\n");
    }

    return rv;
}

int
sal_open(char *file, int oflag, ...)
/*
 * Function:    sal_open
 * Purpose:     Open a file.
 * Parameters:  name - name of file to open, look specifically for
 *                      a colon in the name for netio format
 *              oflag - open mode (O_RDONLY, etc).
 * Returns:     File descriptor, or -1 on error
 * Notes:       Not all file modes are supported for netio.
 */
{
    char fname[256];
    int mode;
    va_list ap;

    va_start(ap, oflag);
    mode = va_arg(ap, int);
    va_end(ap);

    if (sal_expand_file(file, fname)) {
        cli_out("Error: Cannot expand file name: %s\n", file);
        return(-1);
    } else if (SAL_NETIO_FILE(fname)) {
        return(netio_open(fname, oflag, mode));
    } else if (strcmp(file, "flash:") == 0) {
        return -1;      /* Disaster if no filename --uses raw partition! */
    } else {
        return(open(fname, oflag, mode));
    }
}

int
sal_close(int fd)
/*
 * Function:    sal_close
 * Purpose:     Close a file opened with sal_open
 * Parameters:  fd - File descriptor
 * Returns:     non-zero on error
 */
{
    int         rv;

    if (netio_valid_fd(fd)) {
        rv = netio_close(fd);
    } else {
        rv = close(fd);
        sal_flash_sync();
    }

    return rv;
}

FILE *
sal_fopen(char *file, char *mode)
/*
 * Function:    sal_fopen
 * Purpose:     "fopen" a file.
 * Parameters:  name - name of file to open, look specifically for
 *                      "flash:" for netio format.
 *              mode - file mode.
 * Returns:     NULL or FILE * pointer.
 */
{
    char fname[256];

    if (sal_expand_file(file, fname)) {
        cli_out("Error: Cannot expand file name: %s\n", file);
        return(NULL);
    } else if (SAL_NETIO_FILE(fname)) {
        return(netio_fopen(fname, mode));
    } else if (strcmp(file, "flash:") == 0) {
        return NULL;    /* Disaster if no filename --uses raw partition! */
    } else {
        return(fopen(fname, mode));
    }
}

int
sal_fclose(FILE *fp)
/*
 * Function:    sal_fclose
 * Purpose:     Close a file opened with sal_fopen
 * Parameters:  fp - FILE pointer.
 * Returns:     non-zero on error
 */
{
    int         rv;

    if (netio_valid_fp(fp)) {
        rv = netio_fclose(fp);
    } else {
        rv = fclose(fp);
        sal_flash_sync();
    }

    return rv;
}


int
sal_fread(void *buf, int size, int num, FILE *fp)
/*
 * Function: 	sal_fread
 * Purpose:	read() a file
 * Parameters:	buf - buffer
 *		size - size of an object
 *		num - number of objects
 *		fp - FILE * pointer
 * Returns:	number of bytes read
 */
{
    return fread(buf, size, num, fp);
}

int
sal_feof(FILE *fp)
/*
 * Function: 	sal_feof
 * Purpose:	Return TRUE if EOF of a file is reached
 * Parameters:	FILE * pointer
 * Returns:	TRUE or FALSE
 */
{
    return feof(fp);
}

int
sal_ferror(FILE *fp)
/*
 * Function: 	sal_ferror
 * Purpose:	Return TRUE if an error condition for a file pointer is set
 * Parameters:	FILE * pointer
 * Returns:	TRUE or FALSE
 */
{
    return ferror(fp);
}

int
sal_fsize(FILE *fp)
/*
 * Function: 	sal_fsize
 * Purpose:	Return the size of a file if possible
 * Parameters:	FILE * pointer.
 * Returns:	File size or -1 in case of failure
 */
{
    int rv;
    
    fseek(fp, 0, SEEK_END);
    rv = (int)ftell(fp);
    if (rv < 0) {
        char buf[128];
        int len, bufsz;
        bufsz = sizeof(buf);
        rv = 0;
        do {
            len = sal_fread(buf, 1, bufsz, fp);
            rv += len;
        } while (len >= bufsz);
    }
    fseek(fp, 0, SEEK_SET);

    return rv < 0 ? -1 : rv;
}

int
sal_remove(char *file)
/*
 * Function:    sal_remove
 * Purpose:     Remove a file from a file system.
 * Parameters:  file - name of file to remove.
 * Returns:     0 - OK
 *              -1 - failed.
 */
{
    char        fname[256];
    int         rv;

    if (sal_expand_file(file, fname)) {
        cli_out("Error: Cannot expand file name\n");
        return(-1);
    } else if (SAL_NETIO_FILE(fname)) {
        return(netio_remove(fname));
    } else {
        rv = remove(fname);
        sal_flash_sync();
        return rv;
    }
}

int
sal_rename(char *file_old, char *file_new)
/*
 * Function:    sal_rename
 * Purpose:     Rename a file on a file system.
 * Parameters:  file_old - name of existing file to rename.
 *              file_new - new name of file.
 * Returns:     0 - OK
 *              -1 - failed.
 */
{
    char        fname_old[256];
    char        fname_new[256];
    int         rv;

    if (sal_expand_file(file_old, fname_old)) {
        cli_out("Error: Cannot expand file name\n");
        return(-1);
    } else if (SAL_NETIO_FILE(fname_old)) {
        if (SAL_NETIO_FILE(file_new)) {
            cli_out("Error: Destination must be plain pathname\n");
            return -1;  /* File name only (no path) allowed for netio */
        }
        return(netio_rename(fname_old, file_new));
    } else {
        if (SAL_NETIO_FILE(file_new)) 
            return -1;
        if (strrchr(file_new, ':'))
            strcpy(fname_new, file_new);
        else {
            if (!sal_getcwd(fname_new, sizeof(fname_new)))
                return -1;
            strcpy(&fname_new[strlen(fname_new)], file_new);
        }
        rv = rename(fname_old, fname_new);
        if (rv != OK) {
            struct stat st;
            /*
             * In VxWorks 5.5 'rename' fails if the destination file
             * exists. VxWorks 5.4 compatible behavior is accomplished
             * by deleting the destination file first, but only if our
             * source file actually exists.
             */
            if (stat(fname_old, &st) == OK) {
                (void)remove(fname_new);
                rv = rename(fname_old, fname_new); /* Try again */
            }
        }
        sal_flash_sync();
        return rv;
    }
}

int
sal_mkdir(char *path)
{
    char        path_full[256];
    int         rv;

    if (sal_expand_file(path, path_full)) {
        cli_out("Error: Cannot expand file name\n");
        return(-1);
    } else if (SAL_NETIO_FILE(path_full)) {
        return netio_mkdir(path_full);
    } else {
        rv = mkdir(path_full);
        sal_flash_sync();
        return rv;
    }
}

char *
sal_dirname(char *path, char *dest)
{
    sal_printf("Check!!! system without real dirname\n");
    sal_strcpy(dest,".");
    return dest;
}

char *
sal_basename(char *path, char *dest)
{
    sal_printf("Check!!! system without real basename\n");
    sal_strcpy(dest,path);
    return path;
} 

int
sal_rmdir(char *path)
{
    char        path_full[256];
    int         rv;

    if (sal_expand_file(path, path_full)) {
        cli_out("Error: Cannot expand file name\n");
        return(-1);
    } else if (SAL_NETIO_FILE(path_full)) {
        return netio_rmdir(path_full);
    } else {
        rv = rmdir(path_full);
        sal_flash_sync();
        return rv;
    }
}

SAL_DIR *
/*
 * Function:    sal_opendir
 * Purpose:     Open a directory
 * Parameters:  name - name of directory to open
 * Returns:     NULL or SAL_DIR pointer.
 * Notes:       SAL_DIR pointer can be passed to sal_readdir, etc.
 *              sal_closedir should be used to free resources.
 */
sal_opendir(char *dirName)
{
    char fname[SAL_NAME_MAX];
    SAL_DIR *dirp;

    if (dirName == NULL || dirName[0] == 0) {
        dirName = ".";
    }

    if (sal_expand_file(dirName, fname)) {
        cli_out("Error: Cannot expand directory name: %s\n", dirName);
        return(NULL);
    }

    if ((dirp = malloc(sizeof (*dirp))) == 0) {
        return NULL;
    }

    if (SAL_NETIO_FILE(fname)) {
        dirp->is_netio = 1;
        dirp->dirp = netio_opendir(fname);
    } else {
        dirp->is_netio = 0;
        dirp->dirp = opendir(fname);
    }

    if (dirp->dirp == NULL) {
        free(dirp);
        dirp = NULL;
    }

    return (SAL_DIR *) dirp;
}

int
/*
 * Function:    sal_closedir
 * Purpose:     Close a directory
 * Parameters:  dirp - A valid SAL_DIR pointer returned from sal_opendir
 * Returns:     Non-zero on failure
 */
sal_closedir(SAL_DIR *dirp)
{
    int         rv;

    if (dirp->is_netio) {
        rv = netio_closedir(dirp->dirp);
    } else {
        rv = closedir(dirp->dirp);
    }

    return rv;
}

struct sal_dirent *
/*
 * Function:    sal_readdir
 * Purpose:     Return next directory entry
 * Parameters:  dirp - A valid SAL_DIR pointer returned from sal_opendir
 * Returns:     NULL or SAL_DIR pointer.
 */
sal_readdir(SAL_DIR *dirp)
{
    struct dirent *unix_de;
    struct netio_dirent *netio_de;
    char *s;

    if (dirp->is_netio) {
        if ((netio_de = netio_readdir(dirp->dirp)) == NULL) {
            return NULL;
        }
        s = netio_de->d_name;
    } else {
        if ((unix_de = readdir(dirp->dirp)) == NULL) {
            return NULL;
        }
        s = unix_de->d_name;
    }

    strncpy(dirp->de.d_name, s, sizeof (dirp->de.d_name));

    dirp->de.d_name[sizeof (dirp->de.d_name) - 1] = 0;

    return &dirp->de;
}

void
/*
 * Function:    sal_rewinddir
 * Purpose:     Start reading directory from beginning again
 * Parameters:  dirp - A valid SAL_DIR pointer returned from sal_opendir
 * Returns:     Nothing
 */
sal_rewinddir(SAL_DIR *dirp)
{
    if (dirp->is_netio) {
        netio_rewinddir(dirp->dirp);
    } else {
        rewinddir(dirp->dirp);
    }
}
