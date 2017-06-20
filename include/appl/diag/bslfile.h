/*
 * $Id: $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Broadcom System Log State
 */

#ifndef _DIAG_BSLFILE_H
#define _DIAG_BSLFILE_H

int
bslfile_init(void);

char *
bslfile_name(void);

int
bslfile_close(void);

int
bslfile_open(char *filename, int append);

int
bslfile_is_enabled(void);

int
bslfile_enable(int enable);

#endif /* !_DIAG_BSLFILE_H */
