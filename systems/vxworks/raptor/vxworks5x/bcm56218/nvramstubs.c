/* $Id: nvramstubs.c,v 1.3 2011/07/21 16:14:55 yshtil Exp $ */
#include <stdio.h>
#include <string.h>

extern void sysSerialPrintString(char *s);

/* stub for flash-based nvram_get() function */
char*
nvram_get(const char *name)
{
    if (!strcmp("boardtype", name)) {
        return("0x1");
    }
    sysSerialPrintString("nvram_get:");
    sysSerialPrintString(name);
	return ((char*)0);
}

/* stub for flash-based nvram_getall() function */
int
nvram_getall(char *buf, int count)
{
    sysSerialPrintString("nvram_getall");
	return 0;
}
