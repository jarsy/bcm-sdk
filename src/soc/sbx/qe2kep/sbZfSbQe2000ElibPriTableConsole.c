/*
 * $Id: sbZfSbQe2000ElibPriTableConsole.c,v 1.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypesGlue.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfSbQe2000ElibPriTableConsole.hx"



/* Print members in struct */
void
sbZfSbQe2000ElibPriTable_Print(sbZfSbQe2000ElibPriTable_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("SbQe2000ElibPriTable:: rsvd7=0x%02x"), (unsigned int)  pFromStruct->Rsvd7));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" pri63=0x%01x"), (unsigned int)  pFromStruct->Pri63));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" pri62=0x%01x"), (unsigned int)  pFromStruct->Pri62));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" pri61=0x%01x"), (unsigned int)  pFromStruct->Pri61));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" pri60=0x%01x"), (unsigned int)  pFromStruct->Pri60));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("SbQe2000ElibPriTable:: pri59=0x%01x"), (unsigned int)  pFromStruct->Pri59));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" pri58=0x%01x"), (unsigned int)  pFromStruct->Pri58));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" pri57=0x%01x"), (unsigned int)  pFromStruct->Pri57));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" pri56=0x%01x"), (unsigned int)  pFromStruct->Pri56));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" rsvd6=0x%02x"), (unsigned int)  pFromStruct->Rsvd6));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("SbQe2000ElibPriTable:: pri55=0x%01x"), (unsigned int)  pFromStruct->Pri55));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" pri54=0x%01x"), (unsigned int)  pFromStruct->Pri54));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" pri53=0x%01x"), (unsigned int)  pFromStruct->Pri53));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" pri52=0x%01x"), (unsigned int)  pFromStruct->Pri52));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" pri51=0x%01x"), (unsigned int)  pFromStruct->Pri51));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("SbQe2000ElibPriTable:: pri50=0x%01x"), (unsigned int)  pFromStruct->Pri50));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" pri49=0x%01x"), (unsigned int)  pFromStruct->Pri49));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" pri48=0x%01x"), (unsigned int)  pFromStruct->Pri48));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" rsvd5=0x%02x"), (unsigned int)  pFromStruct->Rsvd5));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" pri47=0x%01x"), (unsigned int)  pFromStruct->Pri47));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("SbQe2000ElibPriTable:: pri46=0x%01x"), (unsigned int)  pFromStruct->Pri46));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" pri45=0x%01x"), (unsigned int)  pFromStruct->Pri45));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" pri44=0x%01x"), (unsigned int)  pFromStruct->Pri44));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" pri43=0x%01x"), (unsigned int)  pFromStruct->Pri43));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" pri42=0x%01x"), (unsigned int)  pFromStruct->Pri42));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("SbQe2000ElibPriTable:: pri41=0x%01x"), (unsigned int)  pFromStruct->Pri41));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" pri40=0x%01x"), (unsigned int)  pFromStruct->Pri40));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" rsvd4=0x%02x"), (unsigned int)  pFromStruct->Rsvd4));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" pri39=0x%01x"), (unsigned int)  pFromStruct->Pri39));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" pri38=0x%01x"), (unsigned int)  pFromStruct->Pri38));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("SbQe2000ElibPriTable:: pri37=0x%01x"), (unsigned int)  pFromStruct->Pri37));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" pri36=0x%01x"), (unsigned int)  pFromStruct->Pri36));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" pri35=0x%01x"), (unsigned int)  pFromStruct->Pri35));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" pri34=0x%01x"), (unsigned int)  pFromStruct->Pri34));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" pri33=0x%01x"), (unsigned int)  pFromStruct->Pri33));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("SbQe2000ElibPriTable:: pri32=0x%01x"), (unsigned int)  pFromStruct->Pri32));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" rsvd3=0x%02x"), (unsigned int)  pFromStruct->Rsvd3));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" pri31=0x%01x"), (unsigned int)  pFromStruct->Pri31));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" pri30=0x%01x"), (unsigned int)  pFromStruct->Pri30));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" pri29=0x%01x"), (unsigned int)  pFromStruct->Pri29));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("SbQe2000ElibPriTable:: pri28=0x%01x"), (unsigned int)  pFromStruct->Pri28));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" pri27=0x%01x"), (unsigned int)  pFromStruct->Pri27));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" pri26=0x%01x"), (unsigned int)  pFromStruct->Pri26));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" pri25=0x%01x"), (unsigned int)  pFromStruct->Pri25));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" pri24=0x%01x"), (unsigned int)  pFromStruct->Pri24));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("SbQe2000ElibPriTable:: rsvd2=0x%02x"), (unsigned int)  pFromStruct->Rsvd2));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" pri23=0x%01x"), (unsigned int)  pFromStruct->Pri23));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" pri22=0x%01x"), (unsigned int)  pFromStruct->Pri22));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" pri21=0x%01x"), (unsigned int)  pFromStruct->Pri21));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" pri20=0x%01x"), (unsigned int)  pFromStruct->Pri20));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("SbQe2000ElibPriTable:: pri19=0x%01x"), (unsigned int)  pFromStruct->Pri19));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" pri18=0x%01x"), (unsigned int)  pFromStruct->Pri18));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" pri17=0x%01x"), (unsigned int)  pFromStruct->Pri17));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" pri16=0x%01x"), (unsigned int)  pFromStruct->Pri16));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" rsvd1=0x%02x"), (unsigned int)  pFromStruct->Rsvd1));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("SbQe2000ElibPriTable:: pri15=0x%01x"), (unsigned int)  pFromStruct->Pri15));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" pri14=0x%01x"), (unsigned int)  pFromStruct->Pri14));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" pri13=0x%01x"), (unsigned int)  pFromStruct->Pri13));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" pri12=0x%01x"), (unsigned int)  pFromStruct->Pri12));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" pri11=0x%01x"), (unsigned int)  pFromStruct->Pri11));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("SbQe2000ElibPriTable:: pri10=0x%01x"), (unsigned int)  pFromStruct->Pri10));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" pri9=0x%01x"), (unsigned int)  pFromStruct->Pri9));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" pri8=0x%01x"), (unsigned int)  pFromStruct->Pri8));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" rsvd0=0x%02x"), (unsigned int)  pFromStruct->Rsvd0));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" pri7=0x%01x"), (unsigned int)  pFromStruct->Pri7));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("SbQe2000ElibPriTable:: pri6=0x%01x"), (unsigned int)  pFromStruct->Pri6));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" pri5=0x%01x"), (unsigned int)  pFromStruct->Pri5));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" pri4=0x%01x"), (unsigned int)  pFromStruct->Pri4));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" pri3=0x%01x"), (unsigned int)  pFromStruct->Pri3));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" pri2=0x%01x"), (unsigned int)  pFromStruct->Pri2));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("SbQe2000ElibPriTable:: pri1=0x%01x"), (unsigned int)  pFromStruct->Pri1));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" pri0=0x%01x"), (unsigned int)  pFromStruct->Pri0));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfSbQe2000ElibPriTable_SPrint(sbZfSbQe2000ElibPriTable_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibPriTable:: rsvd7=0x%02x", (unsigned int)  pFromStruct->Rsvd7);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri63=0x%01x", (unsigned int)  pFromStruct->Pri63);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri62=0x%01x", (unsigned int)  pFromStruct->Pri62);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri61=0x%01x", (unsigned int)  pFromStruct->Pri61);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri60=0x%01x", (unsigned int)  pFromStruct->Pri60);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibPriTable:: pri59=0x%01x", (unsigned int)  pFromStruct->Pri59);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri58=0x%01x", (unsigned int)  pFromStruct->Pri58);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri57=0x%01x", (unsigned int)  pFromStruct->Pri57);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri56=0x%01x", (unsigned int)  pFromStruct->Pri56);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," rsvd6=0x%02x", (unsigned int)  pFromStruct->Rsvd6);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibPriTable:: pri55=0x%01x", (unsigned int)  pFromStruct->Pri55);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri54=0x%01x", (unsigned int)  pFromStruct->Pri54);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri53=0x%01x", (unsigned int)  pFromStruct->Pri53);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri52=0x%01x", (unsigned int)  pFromStruct->Pri52);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri51=0x%01x", (unsigned int)  pFromStruct->Pri51);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibPriTable:: pri50=0x%01x", (unsigned int)  pFromStruct->Pri50);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri49=0x%01x", (unsigned int)  pFromStruct->Pri49);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri48=0x%01x", (unsigned int)  pFromStruct->Pri48);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," rsvd5=0x%02x", (unsigned int)  pFromStruct->Rsvd5);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri47=0x%01x", (unsigned int)  pFromStruct->Pri47);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibPriTable:: pri46=0x%01x", (unsigned int)  pFromStruct->Pri46);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri45=0x%01x", (unsigned int)  pFromStruct->Pri45);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri44=0x%01x", (unsigned int)  pFromStruct->Pri44);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri43=0x%01x", (unsigned int)  pFromStruct->Pri43);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri42=0x%01x", (unsigned int)  pFromStruct->Pri42);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibPriTable:: pri41=0x%01x", (unsigned int)  pFromStruct->Pri41);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri40=0x%01x", (unsigned int)  pFromStruct->Pri40);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," rsvd4=0x%02x", (unsigned int)  pFromStruct->Rsvd4);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri39=0x%01x", (unsigned int)  pFromStruct->Pri39);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri38=0x%01x", (unsigned int)  pFromStruct->Pri38);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibPriTable:: pri37=0x%01x", (unsigned int)  pFromStruct->Pri37);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri36=0x%01x", (unsigned int)  pFromStruct->Pri36);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri35=0x%01x", (unsigned int)  pFromStruct->Pri35);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri34=0x%01x", (unsigned int)  pFromStruct->Pri34);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri33=0x%01x", (unsigned int)  pFromStruct->Pri33);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibPriTable:: pri32=0x%01x", (unsigned int)  pFromStruct->Pri32);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," rsvd3=0x%02x", (unsigned int)  pFromStruct->Rsvd3);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri31=0x%01x", (unsigned int)  pFromStruct->Pri31);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri30=0x%01x", (unsigned int)  pFromStruct->Pri30);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri29=0x%01x", (unsigned int)  pFromStruct->Pri29);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibPriTable:: pri28=0x%01x", (unsigned int)  pFromStruct->Pri28);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri27=0x%01x", (unsigned int)  pFromStruct->Pri27);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri26=0x%01x", (unsigned int)  pFromStruct->Pri26);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri25=0x%01x", (unsigned int)  pFromStruct->Pri25);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri24=0x%01x", (unsigned int)  pFromStruct->Pri24);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibPriTable:: rsvd2=0x%02x", (unsigned int)  pFromStruct->Rsvd2);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri23=0x%01x", (unsigned int)  pFromStruct->Pri23);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri22=0x%01x", (unsigned int)  pFromStruct->Pri22);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri21=0x%01x", (unsigned int)  pFromStruct->Pri21);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri20=0x%01x", (unsigned int)  pFromStruct->Pri20);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibPriTable:: pri19=0x%01x", (unsigned int)  pFromStruct->Pri19);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri18=0x%01x", (unsigned int)  pFromStruct->Pri18);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri17=0x%01x", (unsigned int)  pFromStruct->Pri17);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri16=0x%01x", (unsigned int)  pFromStruct->Pri16);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," rsvd1=0x%02x", (unsigned int)  pFromStruct->Rsvd1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibPriTable:: pri15=0x%01x", (unsigned int)  pFromStruct->Pri15);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri14=0x%01x", (unsigned int)  pFromStruct->Pri14);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri13=0x%01x", (unsigned int)  pFromStruct->Pri13);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri12=0x%01x", (unsigned int)  pFromStruct->Pri12);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri11=0x%01x", (unsigned int)  pFromStruct->Pri11);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibPriTable:: pri10=0x%01x", (unsigned int)  pFromStruct->Pri10);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri9=0x%01x", (unsigned int)  pFromStruct->Pri9);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri8=0x%01x", (unsigned int)  pFromStruct->Pri8);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," rsvd0=0x%02x", (unsigned int)  pFromStruct->Rsvd0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri7=0x%01x", (unsigned int)  pFromStruct->Pri7);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibPriTable:: pri6=0x%01x", (unsigned int)  pFromStruct->Pri6);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri5=0x%01x", (unsigned int)  pFromStruct->Pri5);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri4=0x%01x", (unsigned int)  pFromStruct->Pri4);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri3=0x%01x", (unsigned int)  pFromStruct->Pri3);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri2=0x%01x", (unsigned int)  pFromStruct->Pri2);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibPriTable:: pri1=0x%01x", (unsigned int)  pFromStruct->Pri1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri0=0x%01x", (unsigned int)  pFromStruct->Pri0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfSbQe2000ElibPriTable_Validate(sbZfSbQe2000ElibPriTable_t *pZf) {

  if (pZf->Rsvd7 > 0xff) return 0;
  if (pZf->Pri63 > 0x7) return 0;
  if (pZf->Pri62 > 0x7) return 0;
  if (pZf->Pri61 > 0x7) return 0;
  if (pZf->Pri60 > 0x7) return 0;
  if (pZf->Pri59 > 0x7) return 0;
  if (pZf->Pri58 > 0x7) return 0;
  if (pZf->Pri57 > 0x7) return 0;
  if (pZf->Pri56 > 0x7) return 0;
  if (pZf->Rsvd6 > 0xff) return 0;
  if (pZf->Pri55 > 0x7) return 0;
  if (pZf->Pri54 > 0x7) return 0;
  if (pZf->Pri53 > 0x7) return 0;
  if (pZf->Pri52 > 0x7) return 0;
  if (pZf->Pri51 > 0x7) return 0;
  if (pZf->Pri50 > 0x7) return 0;
  if (pZf->Pri49 > 0x7) return 0;
  if (pZf->Pri48 > 0x7) return 0;
  if (pZf->Rsvd5 > 0xff) return 0;
  if (pZf->Pri47 > 0x7) return 0;
  if (pZf->Pri46 > 0x7) return 0;
  if (pZf->Pri45 > 0x7) return 0;
  if (pZf->Pri44 > 0x7) return 0;
  if (pZf->Pri43 > 0x7) return 0;
  if (pZf->Pri42 > 0x7) return 0;
  if (pZf->Pri41 > 0x7) return 0;
  if (pZf->Pri40 > 0x7) return 0;
  if (pZf->Rsvd4 > 0xff) return 0;
  if (pZf->Pri39 > 0x7) return 0;
  if (pZf->Pri38 > 0x7) return 0;
  if (pZf->Pri37 > 0x7) return 0;
  if (pZf->Pri36 > 0x7) return 0;
  if (pZf->Pri35 > 0x7) return 0;
  if (pZf->Pri34 > 0x7) return 0;
  if (pZf->Pri33 > 0x7) return 0;
  if (pZf->Pri32 > 0x7) return 0;
  if (pZf->Rsvd3 > 0xff) return 0;
  if (pZf->Pri31 > 0x7) return 0;
  if (pZf->Pri30 > 0x7) return 0;
  if (pZf->Pri29 > 0x7) return 0;
  if (pZf->Pri28 > 0x7) return 0;
  if (pZf->Pri27 > 0x7) return 0;
  if (pZf->Pri26 > 0x7) return 0;
  if (pZf->Pri25 > 0x7) return 0;
  if (pZf->Pri24 > 0x7) return 0;
  if (pZf->Rsvd2 > 0xff) return 0;
  if (pZf->Pri23 > 0x7) return 0;
  if (pZf->Pri22 > 0x7) return 0;
  if (pZf->Pri21 > 0x7) return 0;
  if (pZf->Pri20 > 0x7) return 0;
  if (pZf->Pri19 > 0x7) return 0;
  if (pZf->Pri18 > 0x7) return 0;
  if (pZf->Pri17 > 0x7) return 0;
  if (pZf->Pri16 > 0x7) return 0;
  if (pZf->Rsvd1 > 0xff) return 0;
  if (pZf->Pri15 > 0x7) return 0;
  if (pZf->Pri14 > 0x7) return 0;
  if (pZf->Pri13 > 0x7) return 0;
  if (pZf->Pri12 > 0x7) return 0;
  if (pZf->Pri11 > 0x7) return 0;
  if (pZf->Pri10 > 0x7) return 0;
  if (pZf->Pri9 > 0x7) return 0;
  if (pZf->Pri8 > 0x7) return 0;
  if (pZf->Rsvd0 > 0xff) return 0;
  if (pZf->Pri7 > 0x7) return 0;
  if (pZf->Pri6 > 0x7) return 0;
  if (pZf->Pri5 > 0x7) return 0;
  if (pZf->Pri4 > 0x7) return 0;
  if (pZf->Pri3 > 0x7) return 0;
  if (pZf->Pri2 > 0x7) return 0;
  if (pZf->Pri1 > 0x7) return 0;
  if (pZf->Pri0 > 0x7) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfSbQe2000ElibPriTable_SetField(sbZfSbQe2000ElibPriTable_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "rsvd7") == 0) {
    s->Rsvd7 = value;
  } else if (SB_STRCMP(name, "pri63") == 0) {
    s->Pri63 = value;
  } else if (SB_STRCMP(name, "pri62") == 0) {
    s->Pri62 = value;
  } else if (SB_STRCMP(name, "pri61") == 0) {
    s->Pri61 = value;
  } else if (SB_STRCMP(name, "pri60") == 0) {
    s->Pri60 = value;
  } else if (SB_STRCMP(name, "pri59") == 0) {
    s->Pri59 = value;
  } else if (SB_STRCMP(name, "pri58") == 0) {
    s->Pri58 = value;
  } else if (SB_STRCMP(name, "pri57") == 0) {
    s->Pri57 = value;
  } else if (SB_STRCMP(name, "pri56") == 0) {
    s->Pri56 = value;
  } else if (SB_STRCMP(name, "rsvd6") == 0) {
    s->Rsvd6 = value;
  } else if (SB_STRCMP(name, "pri55") == 0) {
    s->Pri55 = value;
  } else if (SB_STRCMP(name, "pri54") == 0) {
    s->Pri54 = value;
  } else if (SB_STRCMP(name, "pri53") == 0) {
    s->Pri53 = value;
  } else if (SB_STRCMP(name, "pri52") == 0) {
    s->Pri52 = value;
  } else if (SB_STRCMP(name, "pri51") == 0) {
    s->Pri51 = value;
  } else if (SB_STRCMP(name, "pri50") == 0) {
    s->Pri50 = value;
  } else if (SB_STRCMP(name, "pri49") == 0) {
    s->Pri49 = value;
  } else if (SB_STRCMP(name, "pri48") == 0) {
    s->Pri48 = value;
  } else if (SB_STRCMP(name, "rsvd5") == 0) {
    s->Rsvd5 = value;
  } else if (SB_STRCMP(name, "pri47") == 0) {
    s->Pri47 = value;
  } else if (SB_STRCMP(name, "pri46") == 0) {
    s->Pri46 = value;
  } else if (SB_STRCMP(name, "pri45") == 0) {
    s->Pri45 = value;
  } else if (SB_STRCMP(name, "pri44") == 0) {
    s->Pri44 = value;
  } else if (SB_STRCMP(name, "pri43") == 0) {
    s->Pri43 = value;
  } else if (SB_STRCMP(name, "pri42") == 0) {
    s->Pri42 = value;
  } else if (SB_STRCMP(name, "pri41") == 0) {
    s->Pri41 = value;
  } else if (SB_STRCMP(name, "pri40") == 0) {
    s->Pri40 = value;
  } else if (SB_STRCMP(name, "rsvd4") == 0) {
    s->Rsvd4 = value;
  } else if (SB_STRCMP(name, "pri39") == 0) {
    s->Pri39 = value;
  } else if (SB_STRCMP(name, "pri38") == 0) {
    s->Pri38 = value;
  } else if (SB_STRCMP(name, "pri37") == 0) {
    s->Pri37 = value;
  } else if (SB_STRCMP(name, "pri36") == 0) {
    s->Pri36 = value;
  } else if (SB_STRCMP(name, "pri35") == 0) {
    s->Pri35 = value;
  } else if (SB_STRCMP(name, "pri34") == 0) {
    s->Pri34 = value;
  } else if (SB_STRCMP(name, "pri33") == 0) {
    s->Pri33 = value;
  } else if (SB_STRCMP(name, "pri32") == 0) {
    s->Pri32 = value;
  } else if (SB_STRCMP(name, "rsvd3") == 0) {
    s->Rsvd3 = value;
  } else if (SB_STRCMP(name, "pri31") == 0) {
    s->Pri31 = value;
  } else if (SB_STRCMP(name, "pri30") == 0) {
    s->Pri30 = value;
  } else if (SB_STRCMP(name, "pri29") == 0) {
    s->Pri29 = value;
  } else if (SB_STRCMP(name, "pri28") == 0) {
    s->Pri28 = value;
  } else if (SB_STRCMP(name, "pri27") == 0) {
    s->Pri27 = value;
  } else if (SB_STRCMP(name, "pri26") == 0) {
    s->Pri26 = value;
  } else if (SB_STRCMP(name, "pri25") == 0) {
    s->Pri25 = value;
  } else if (SB_STRCMP(name, "pri24") == 0) {
    s->Pri24 = value;
  } else if (SB_STRCMP(name, "rsvd2") == 0) {
    s->Rsvd2 = value;
  } else if (SB_STRCMP(name, "pri23") == 0) {
    s->Pri23 = value;
  } else if (SB_STRCMP(name, "pri22") == 0) {
    s->Pri22 = value;
  } else if (SB_STRCMP(name, "pri21") == 0) {
    s->Pri21 = value;
  } else if (SB_STRCMP(name, "pri20") == 0) {
    s->Pri20 = value;
  } else if (SB_STRCMP(name, "pri19") == 0) {
    s->Pri19 = value;
  } else if (SB_STRCMP(name, "pri18") == 0) {
    s->Pri18 = value;
  } else if (SB_STRCMP(name, "pri17") == 0) {
    s->Pri17 = value;
  } else if (SB_STRCMP(name, "pri16") == 0) {
    s->Pri16 = value;
  } else if (SB_STRCMP(name, "rsvd1") == 0) {
    s->Rsvd1 = value;
  } else if (SB_STRCMP(name, "pri15") == 0) {
    s->Pri15 = value;
  } else if (SB_STRCMP(name, "pri14") == 0) {
    s->Pri14 = value;
  } else if (SB_STRCMP(name, "pri13") == 0) {
    s->Pri13 = value;
  } else if (SB_STRCMP(name, "pri12") == 0) {
    s->Pri12 = value;
  } else if (SB_STRCMP(name, "pri11") == 0) {
    s->Pri11 = value;
  } else if (SB_STRCMP(name, "pri10") == 0) {
    s->Pri10 = value;
  } else if (SB_STRCMP(name, "pri9") == 0) {
    s->Pri9 = value;
  } else if (SB_STRCMP(name, "pri8") == 0) {
    s->Pri8 = value;
  } else if (SB_STRCMP(name, "rsvd0") == 0) {
    s->Rsvd0 = value;
  } else if (SB_STRCMP(name, "pri7") == 0) {
    s->Pri7 = value;
  } else if (SB_STRCMP(name, "pri6") == 0) {
    s->Pri6 = value;
  } else if (SB_STRCMP(name, "pri5") == 0) {
    s->Pri5 = value;
  } else if (SB_STRCMP(name, "pri4") == 0) {
    s->Pri4 = value;
  } else if (SB_STRCMP(name, "pri3") == 0) {
    s->Pri3 = value;
  } else if (SB_STRCMP(name, "pri2") == 0) {
    s->Pri2 = value;
  } else if (SB_STRCMP(name, "pri1") == 0) {
    s->Pri1 = value;
  } else if (SB_STRCMP(name, "pri0") == 0) {
    s->Pri0 = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
