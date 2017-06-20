/*
 * Stubs for NVRAM functions for platforms without flash
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * $Id: nvramstubs.c,v 1.1 Broadcom SDK $
 */

#include <typedefs.h>
#include <bcmutils.h>
#undef strcmp
#define strcmp(s1,s2)	0	/* always match */
#include <bcmnvram.h>

int
nvram_init(void *sih)
{
	return 0;
}

int
nvram_append(void *sb, char *vars, uint varsz)
{
	return 0;
}

void
nvram_exit(void *sih)
{
}

char *
nvram_get(const char *name)
{
	return (char *) 0;
}

int
nvram_set(const char *name, const char *value)
{
	return 0;
}

int
nvram_unset(const char *name)
{
	return 0;
}

int
nvram_commit(void)
{
	return 0;
}

int
nvram_getall(char *buf, int count)
{
	/* add null string as terminator */
	if (count < 1)
		return BCME_BUFTOOSHORT;
	*buf = '\0';
	return 0;
}
