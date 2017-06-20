/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfFabWredTableEntry.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_WRED_TABLE_ENTRY_H
#define SB_ZF_WRED_TABLE_ENTRY_H

#define SB_ZF_WRED_TABLE_ENTRY_SIZE_IN_BYTES 24
#define SB_ZF_WRED_TABLE_ENTRY_SIZE 24
#define SB_ZF_WRED_TABLE_ENTRY_M_NDP0_BITS "191:128"
#define SB_ZF_WRED_TABLE_ENTRY_M_NDP1_BITS "127:64"
#define SB_ZF_WRED_TABLE_ENTRY_M_NDP2_BITS "63:0"


/**

 * Copyright (c) Sandburst, Inc. 2005
 * All Rights Reserved.  Unpublished rights reserved under the copyright
 * laws of the United States.
 *
 * The software contained on this media is proprietary to and embodies the
 * confidential technology of Sandburst, Inc. Possession, use, duplication
 * or dissemination of the software and media is authorized only pursuant
 * to a valid written license from Sandburst, Inc.
 *
 * RESTRICTED RIGHTS LEGEND Use, duplication, or disclosure by the U.S.
 * Government is subject to restrictions as set forth in Subparagraph
 * (c) (1) (ii) of DFARS 252.227-7013, or in FAR 52.227-19, as applicable.
 */
typedef struct _sbZfFabWredTableEntry {
  sbZfFabWredParameters_t m_nDp0;
  sbZfFabWredParameters_t m_nDp1;
  sbZfFabWredParameters_t m_nDp2;
} sbZfFabWredTableEntry_t;

uint32
sbZfFabWredTableEntry_Pack(sbZfFabWredTableEntry_t *pFrom,
                           uint8 *pToData,
                           uint32 nMaxToDataIndex);
void
sbZfFabWredTableEntry_Unpack(sbZfFabWredTableEntry_t *pToStruct,
                             uint8 *pFromData,
                             uint32 nMaxToDataIndex);
void
sbZfFabWredTableEntry_InitInstance(sbZfFabWredTableEntry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
/* No _SET defined for member, m_nDp0, that is a ZFRAME */
/* No _SET defined for member, m_nDp1, that is a ZFRAME */
/* No _SET defined for member, m_nDp2, that is a ZFRAME */
#else
/* No _SET defined for member, m_nDp0, that is a ZFRAME */
/* No _SET defined for member, m_nDp1, that is a ZFRAME */
/* No _SET defined for member, m_nDp2, that is a ZFRAME */
#endif
#ifdef SAND_BIG_ENDIAN_HOST
/* No _SET defined for member, m_nDp0, that is a ZFRAME */
/* No _SET defined for member, m_nDp1, that is a ZFRAME */
/* No _SET defined for member, m_nDp2, that is a ZFRAME */
#else
/* No _SET defined for member, m_nDp0, that is a ZFRAME */
/* No _SET defined for member, m_nDp1, that is a ZFRAME */
/* No _SET defined for member, m_nDp2, that is a ZFRAME */
#endif
#ifdef SAND_BIG_ENDIAN_HOST
/* No _GET defined for member, m_nDp0, that is a ZFRAME */
/* No _GET defined for member, m_nDp1, that is a ZFRAME */
/* No _GET defined for member, m_nDp2, that is a ZFRAME */
#else
/* No _GET defined for member, m_nDp0, that is a ZFRAME */
/* No _GET defined for member, m_nDp1, that is a ZFRAME */
/* No _GET defined for member, m_nDp2, that is a ZFRAME */
#endif
#ifdef SAND_BIG_ENDIAN_HOST
/* No _GET defined for member, m_nDp0, that is a ZFRAME */
/* No _GET defined for member, m_nDp1, that is a ZFRAME */
/* No _GET defined for member, m_nDp2, that is a ZFRAME */
#else
/* No _GET defined for member, m_nDp0, that is a ZFRAME */
/* No _GET defined for member, m_nDp1, that is a ZFRAME */
/* No _GET defined for member, m_nDp2, that is a ZFRAME */
#endif
#endif
/*
 * $Id: sbZfFabWredTableEntry.hx,v 1.2 Broadcom SDK $
 * $Copyright (c) 2012 Broadcom Corporation
 * All rights reserved.$
 */



#ifdef SB_ZF_INCLUDE_CONSOLE
#ifndef SB_ZF_WRED_TABLE_ENTRY_CONSOLE_H
#define SB_ZF_WRED_TABLE_ENTRY_CONSOLE_H



void
sbZfFabWredTableEntry_Print(sbZfFabWredTableEntry_t *pFromStruct);
int
sbZfFabWredTableEntry_SPrint(sbZfFabWredTableEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfFabWredTableEntry_Validate(sbZfFabWredTableEntry_t *pZf);
int
sbZfFabWredTableEntry_SetField(sbZfFabWredTableEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_WRED_TABLE_ENTRY_CONSOLE_H */
#endif /* ifdef SB_ZF_INCLUDE_CONSOLE */

