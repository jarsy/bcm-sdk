/*
 * $Id: sbZfKaEpIpTciDmacConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfKaEpIpTciDmac.hx"
#ifndef SB_ZF_ZFKAEPIPTCIDMAC_CONSOLE_H
#define SB_ZF_ZFKAEPIPTCIDMAC_CONSOLE_H



void
sbZfKaEpIpTciDmac_Print(sbZfKaEpIpTciDmac_t *pFromStruct);
char *
sbZfKaEpIpTciDmac_SPrint(sbZfKaEpIpTciDmac_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfKaEpIpTciDmac_Validate(sbZfKaEpIpTciDmac_t *pZf);
int
sbZfKaEpIpTciDmac_SetField(sbZfKaEpIpTciDmac_t *s, char* name, int value);


#endif /* ifndef SB_ZF_ZFKAEPIPTCIDMAC_CONSOLE_H */
