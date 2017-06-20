/*
 * $Id: sbZfKaRbPoliceEBSEntryConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfKaRbPoliceEBSEntry.hx"
#ifndef SB_ZF_ZFKARBPOLEBSENTRY_CONSOLE_H
#define SB_ZF_ZFKARBPOLEBSENTRY_CONSOLE_H



void
sbZfKaRbPoliceEBSEntry_Print(sbZfKaRbPoliceEBSEntry_t *pFromStruct);
char *
sbZfKaRbPoliceEBSEntry_SPrint(sbZfKaRbPoliceEBSEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfKaRbPoliceEBSEntry_Validate(sbZfKaRbPoliceEBSEntry_t *pZf);
int
sbZfKaRbPoliceEBSEntry_SetField(sbZfKaRbPoliceEBSEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_ZFKARBPOLEBSENTRY_CONSOLE_H */
