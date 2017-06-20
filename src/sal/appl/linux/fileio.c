/*
 * $Id: fileio.c,v 1.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File: 	fileio.c
 * Purpose:	File I/O
 */

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/string.h>

#include <sal/appl/io.h>

static char _homedir[256];

int
sal_homedir_set(char *dir)
{
    strncpy(_homedir, dir, sizeof(_homedir));
    return 0;
}

char *
sal_homedir_get(char *buf, size_t size)
{
    strncpy(buf, _homedir, size);
    return buf;
}

/*
 * Get current working directory.
 *
 * NOTE: this version of cwd always includes the trailing slash so
 * a filename should be directly appended.  This is in order to
 * interoperate better with vxWorks ftp filenames which may sometimes
 * have a trailing colon instead of slash.
 */

char *sal_getcwd(char *buf, size_t size)
{
    strcpy(buf, "/");
    return buf;
}

int sal_ls(char *f, char *flags)
{
    sal_printf("no filesystem\n");
    return 0;
}

int sal_cd(char *f)
{
    return 0;
}

FILE *
sal_fopen(char *file, char *mode)
/*
 * Function: 	sal_fopen
 * Purpose:	"fopen" a file.
 * Parameters:	name - name of file to open
 *		mode - file mode.
 * Returns:	NULL or FILE * pointer.
 */
{
    return NULL;
}

int
sal_fclose(FILE *fp)
/*
 * Function: 	sal_fclose
 * Purpose:	Close a file opened with sal_fopen
 * Parameters:	fp - FILE pointer.
 * Returns:	non-zero on error
 */
{
    return 0;
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
    return 0;
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
    return 0;
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
    return 0;
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
    return 0;
}

int
sal_remove(char *f)
{
    return 0;
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
    return dest;
}
int
sal_rename(char *f_old, char *f_new)
{
    return 0;
}

int
sal_mkdir(char *path)
{
    return 0;
}

int
sal_rmdir(char *path)
{
    return 0;
}

SAL_DIR *
sal_opendir(char *dirName)
{
    return NULL;
}

int
sal_closedir(SAL_DIR *dirp)
{
    return 0;
}

struct sal_dirent *
sal_readdir(SAL_DIR *dirp)
{
    return NULL;
}

void
sal_rewinddir(SAL_DIR *dirp)
{
}










