/*
 * $Id: sbZfKaEpIpMplsLabelsConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfKaEpIpMplsLabels.hx"
#ifndef SB_ZF_ZFKAEPIPMPLSLABELS_CONSOLE_H
#define SB_ZF_ZFKAEPIPMPLSLABELS_CONSOLE_H



void
sbZfKaEpIpMplsLabels_Print(sbZfKaEpIpMplsLabels_t *pFromStruct);
char *
sbZfKaEpIpMplsLabels_SPrint(sbZfKaEpIpMplsLabels_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfKaEpIpMplsLabels_Validate(sbZfKaEpIpMplsLabels_t *pZf);
int
sbZfKaEpIpMplsLabels_SetField(sbZfKaEpIpMplsLabels_t *s, char* name, int value);


#endif /* ifndef SB_ZF_ZFKAEPIPMPLSLABELS_CONSOLE_H */
