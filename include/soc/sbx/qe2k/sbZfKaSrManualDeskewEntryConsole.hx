/*
 * $Id: sbZfKaSrManualDeskewEntryConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfKaSrManualDeskewEntry.hx"
#ifndef SB_ZF_ZFKASRMANUALDESKEWENTRY_CONSOLE_H
#define SB_ZF_ZFKASRMANUALDESKEWENTRY_CONSOLE_H



void
sbZfKaSrManualDeskewEntry_Print(sbZfKaSrManualDeskewEntry_t *pFromStruct);
char *
sbZfKaSrManualDeskewEntry_SPrint(sbZfKaSrManualDeskewEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfKaSrManualDeskewEntry_Validate(sbZfKaSrManualDeskewEntry_t *pZf);
int
sbZfKaSrManualDeskewEntry_SetField(sbZfKaSrManualDeskewEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_ZFKASRMANUALDESKEWENTRY_CONSOLE_H */
