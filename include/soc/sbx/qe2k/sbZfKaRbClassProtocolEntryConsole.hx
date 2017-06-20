/*
 * $Id: sbZfKaRbClassProtocolEntryConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfKaRbClassProtocolEntry.hx"
#ifndef SB_ZF_ZFKARBCLASSPROTOCOLENTRY_CONSOLE_H
#define SB_ZF_ZFKARBCLASSPROTOCOLENTRY_CONSOLE_H



void
sbZfKaRbClassProtocolEntry_Print(sbZfKaRbClassProtocolEntry_t *pFromStruct);
char *
sbZfKaRbClassProtocolEntry_SPrint(sbZfKaRbClassProtocolEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfKaRbClassProtocolEntry_Validate(sbZfKaRbClassProtocolEntry_t *pZf);
int
sbZfKaRbClassProtocolEntry_SetField(sbZfKaRbClassProtocolEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_ZFKARBCLASSPROTOCOLENTRY_CONSOLE_H */
