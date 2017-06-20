/*
 * $Id: sbZfFabBm9600InaNmPriorityUpdateConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfFabBm9600InaNmPriorityUpdate.hx"
#ifndef SB_ZF_FAB_BM9600_INANMPRIUPDATE_CONSOLE_H
#define SB_ZF_FAB_BM9600_INANMPRIUPDATE_CONSOLE_H



void
sbZfFabBm9600InaNmPriorityUpdate_Print(sbZfFabBm9600InaNmPriorityUpdate_t *pFromStruct);
char *
sbZfFabBm9600InaNmPriorityUpdate_SPrint(sbZfFabBm9600InaNmPriorityUpdate_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfFabBm9600InaNmPriorityUpdate_Validate(sbZfFabBm9600InaNmPriorityUpdate_t *pZf);
int
sbZfFabBm9600InaNmPriorityUpdate_SetField(sbZfFabBm9600InaNmPriorityUpdate_t *s, char* name, int value);


#endif /* ifndef SB_ZF_FAB_BM9600_INANMPRIUPDATE_CONSOLE_H */
