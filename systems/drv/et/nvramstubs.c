/*
 * $Id: nvramstubs.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * BCM53xx RoboSwitch utility functions
 */
/* stub for flash-based nvram_get() function */
char*
nvram_get(const char *name)
{
	return ((char*)0);
}

/* stub for flash-based nvram_getall() function */
int
nvram_getall(char *buf, int count)
{
	return 0;
}
