/*
 * $Id: bcmnvram.h,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * NVRAM variable manipulation
 */

#ifndef _bcmnvram_h_
#define _bcmnvram_h_

#ifndef _LANGUAGE_ASSEMBLY

#include <shared/et/typedefs.h>

struct nvram_header {
	unsigned long magic;
	unsigned long len;
	unsigned long crc_ver_init;	/* 0:7 crc, 8:15 ver, 16:27 init, mem. test 28, 29-31 reserved */
	unsigned long config_refresh;	/* 0:15 config, 16:31 refresh */
	unsigned long reserved;
};

struct nvram_tuple {
	char *name;
	char *value;
	struct nvram_tuple *next;
};

/* Compatibility */
typedef struct nvram_tuple EnvRec;

/*
 * Get the value of an NVRAM variable
 * @param	name	name of variable to get
 * @return	value of variable or NULL if undefined
 */
extern char * nvram_get(const char *name);

/* 
 * Get the value of an NVRAM variable
 * @param	name	name of variable to get
 * @return	value of variable or NUL if undefined
 */
#define nvram_safe_get(name) (nvram_get(name) ? : "")

/*
 * Set the value of an NVRAM variable
 * @param	name	name of variable to set
 * @param	value	value of variable
 * @return	0 on success and errno on failure
 * NOTE: use nvram_commit to commit this change to flash.
 */
extern int nvram_set(const char *name, const char *value);

/*
 * Unset an NVRAM variable
 * @param	name	name of variable to unset
 * @return	0 on success and errno on failure
 * NOTE: use nvram_commit to commit this change to flash.
 */
extern int nvram_unset(const char *name);

/*
 * Permanently commit NVRAM variables
 * @return	0 on success and errno on failure
 */
extern int nvram_commit(void);

/*
 * Get all NVRAM variables (format name=value\0 ... \0\0)
 * @param	buf	buffer to store variables
 * @param	count	size of buffer in bytes
 * @return	0 on success and errno on failure
 */
extern int nvram_getall(char *buf, int count);

/*
 * Invalidate the current NVRAM header
 * @return	0 on success and errno on failure
 */
extern int nvram_invalidate(void);

/*
 * Dump NVRAM data including header into a buffer for file backup. 
 * The data format is the same as what is in the flash - 5 32-bit words 
 * header followed by name=value pairs (see nvram_getall() for name=value 
 * pairs format).
 *
 * @param	buf	- buffer to nvram data
 * @param	count	- size of buffer in bytes
 * @return	0 on success and others on failure
 *		-1 - buffer pointer is NULL or buffer is not 4 byte aligned,
 * 		-2 - no nvram,
 *		-3 - buffer too small
 * @param	count	- buffer used in bytes
 */
extern int nvram_dump(char *buf, int *count);

/*
 * Verify NVRAM data read from backup file and update nvram. 
 * The data format is the same as what is in the flash - 5 32-bit words 
 * header followed by name=value pairs (see nvram_getall() for name=value 
 * pairs format).
 *
 * @param	buf 	- buffer to nvram data
 * @param	count	- size of buffer in bytes
 * @return	0 on success and others on failure
 *		-1 - buffer pointer is NULL, buffer is not 4 byte aligned,
 *		-2 - wrong magic,
 *		-3 - wrong buffer size,
 *		-4 - CRC checking failure,
 *		-5 - flash init failed
 */
extern int nvram_restore(char *buf, int count);

#endif /* _LANGUAGE_ASSEMBLY */

#define NVRAM_MAGIC		0x48534C46	/* 'FLSH' */
#define NVRAM_VERSION		1
#define NVRAM_HEADER_SIZE	20
#define NVRAM_FIRST_LOC		0xbfcf8000
#define NVRAM_LAST_LOC		0xbfff8000
#define NVRAM_LOC_GAP		0x100000
#define NVRAM_SPACE		0x8000

#endif /* _bcmnvram_h_ */
