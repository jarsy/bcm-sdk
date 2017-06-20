/*
 * $Id: sbZfKaEbMvtAddressConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfKaEbMvtAddress.hx"
#ifndef SB_ZF_ZFKAEBMVTADDRESS_CONSOLE_H
#define SB_ZF_ZFKAEBMVTADDRESS_CONSOLE_H



void
sbZfKaEbMvtAddress_Print(sbZfKaEbMvtAddress_t *pFromStruct);
char *
sbZfKaEbMvtAddress_SPrint(sbZfKaEbMvtAddress_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfKaEbMvtAddress_Validate(sbZfKaEbMvtAddress_t *pZf);
int
sbZfKaEbMvtAddress_SetField(sbZfKaEbMvtAddress_t *s, char* name, int value);


#endif /* ifndef SB_ZF_ZFKAEBMVTADDRESS_CONSOLE_H */
