/*
 * $Id: sbZfKaQsE2QEntryConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfKaQsE2QEntry.hx"
#ifndef SB_ZF_ZFKAQSE2QENTRY_CONSOLE_H
#define SB_ZF_ZFKAQSE2QENTRY_CONSOLE_H



void
sbZfKaQsE2QEntry_Print(sbZfKaQsE2QEntry_t *pFromStruct);
char *
sbZfKaQsE2QEntry_SPrint(sbZfKaQsE2QEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfKaQsE2QEntry_Validate(sbZfKaQsE2QEntry_t *pZf);
int
sbZfKaQsE2QEntry_SetField(sbZfKaQsE2QEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_ZFKAQSE2QENTRY_CONSOLE_H */
