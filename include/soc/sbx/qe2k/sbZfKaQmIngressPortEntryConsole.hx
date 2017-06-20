/*
 * $Id: sbZfKaQmIngressPortEntryConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfKaQmIngressPortEntry.hx"
#ifndef SB_ZF_ZFKAQMINGRESSPORTENTRY_CONSOLE_H
#define SB_ZF_ZFKAQMINGRESSPORTENTRY_CONSOLE_H



void
sbZfKaQmIngressPortEntry_Print(sbZfKaQmIngressPortEntry_t *pFromStruct);
char *
sbZfKaQmIngressPortEntry_SPrint(sbZfKaQmIngressPortEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfKaQmIngressPortEntry_Validate(sbZfKaQmIngressPortEntry_t *pZf);
int
sbZfKaQmIngressPortEntry_SetField(sbZfKaQmIngressPortEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_ZFKAQMINGRESSPORTENTRY_CONSOLE_H */
