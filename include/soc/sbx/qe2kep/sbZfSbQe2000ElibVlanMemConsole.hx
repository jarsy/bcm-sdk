/*
 * $Id: sbZfSbQe2000ElibVlanMemConsole.hx,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfSbQe2000ElibVlanMem.hx"
#ifndef SB_ZF_SB_QE2000_ELIB_VLAN_MEM_CONSOLE_H
#define SB_ZF_SB_QE2000_ELIB_VLAN_MEM_CONSOLE_H



void
sbZfSbQe2000ElibVlanMem_Print(sbZfSbQe2000ElibVlanMem_t *pFromStruct);
char *
sbZfSbQe2000ElibVlanMem_SPrint(sbZfSbQe2000ElibVlanMem_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfSbQe2000ElibVlanMem_Validate(sbZfSbQe2000ElibVlanMem_t *pZf);
int
sbZfSbQe2000ElibVlanMem_SetField(sbZfSbQe2000ElibVlanMem_t *s, char* name, int value);


#endif /* ifndef SB_ZF_SB_QE2000_ELIB_VLAN_MEM_CONSOLE_H */
