/*
 * $Id: sbZfKaRbPolicePortQMapEntryConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfKaRbPolicePortQMapEntry.hx"
#ifndef SB_ZF_ZFKARBPOLPORTQMAPENTRY_CONSOLE_H
#define SB_ZF_ZFKARBPOLPORTQMAPENTRY_CONSOLE_H



void
sbZfKaRbPolicePortQMapEntry_Print(sbZfKaRbPolicePortQMapEntry_t *pFromStruct);
char *
sbZfKaRbPolicePortQMapEntry_SPrint(sbZfKaRbPolicePortQMapEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfKaRbPolicePortQMapEntry_Validate(sbZfKaRbPolicePortQMapEntry_t *pZf);
int
sbZfKaRbPolicePortQMapEntry_SetField(sbZfKaRbPolicePortQMapEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_ZFKARBPOLPORTQMAPENTRY_CONSOLE_H */
