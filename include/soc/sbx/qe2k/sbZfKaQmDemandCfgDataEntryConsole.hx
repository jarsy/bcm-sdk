/*
 * $Id: sbZfKaQmDemandCfgDataEntryConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfKaQmDemandCfgDataEntry.hx"
#ifndef SB_ZF_ZFKAQMDEMANDCFGDATAENTRY_CONSOLE_H
#define SB_ZF_ZFKAQMDEMANDCFGDATAENTRY_CONSOLE_H



void
sbZfKaQmDemandCfgDataEntry_Print(sbZfKaQmDemandCfgDataEntry_t *pFromStruct);
char *
sbZfKaQmDemandCfgDataEntry_SPrint(sbZfKaQmDemandCfgDataEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfKaQmDemandCfgDataEntry_Validate(sbZfKaQmDemandCfgDataEntry_t *pZf);
int
sbZfKaQmDemandCfgDataEntry_SetField(sbZfKaQmDemandCfgDataEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_ZFKAQMDEMANDCFGDATAENTRY_CONSOLE_H */
