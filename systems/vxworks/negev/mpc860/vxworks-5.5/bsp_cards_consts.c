/*
 * $Id$
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


/******************************************************************
*
* FILENAME:       bsp_cards_consts.c
*
* SYSTEM:           Dune Semiconductors LTD Proprietary information
*
* CREATION DATE:  15-Feb-06
*
* LAST CHANGED:   15-Feb-06
*
* REVISION:       $Revision: 1.2 $
*
* FILE DESCRIPTION:
*
* REMARKS:
*******************************************************************/


/*************
 * INCLUDES  *
 *************/
/* { */

#include "bsp_cards_consts.h"
/* } */

/*************
 * DEFINES   *
 *************/
/* { */

/* } */

/*************
 * TYPED EFS *
 *************/
/* { */

/* } */

/*************
 * GLOBALS   *
 *************/
/* { */

/* } */

/*************
 * FUNCTIONS *
 *************/
/* { */


/*****************************************************
*NAME
*
*  bsp_card_is_same_family
*FUNCTION:
*  Returns:
*    1 -- IFF the 2 card types are the same family.
*    0 -- IFF the 2 card types are NOT the same family.
*INPUT:
*  DIRECT:
*    const BSP_CARDS_DEFINES card_x -
*    const BSP_CARDS_DEFINES card_y -
*  INDIRECT:
*REMARKS:
*  None.
*SEE ALSO:
*****************************************************/
unsigned int
  bsp_card_is_same_family(
    const BSP_CARDS_DEFINES card_x,
    const BSP_CARDS_DEFINES card_y
  )
{
  unsigned int
    same_familiy=0;

  switch(card_x)
  {
  case LINE_CARD_01:
  case LINE_CARD_FAP10M_B:
    if (
          (card_y == LINE_CARD_01) ||
          (card_y == LINE_CARD_FAP10M_B)
       )
    {
      same_familiy = 1;
    }
    else
    {
      same_familiy = 0;
    }
    break;

  case LINE_CARD_GFA_FAP20V:
  case LINE_CARD_GFA_MB_FAP20V:
    if (
          (card_y == LINE_CARD_GFA_FAP20V) ||
          (card_y == LINE_CARD_GFA_MB_FAP20V)
       )
    {
      same_familiy = 1;
    }
    else
    {
      same_familiy = 0;
    }
    break;

  case LINE_CARD_GFA_FAP21V:
  case LINE_CARD_GFA_MB_FAP21V:
    if (
          (card_y == LINE_CARD_GFA_FAP21V) ||
          (card_y == LINE_CARD_GFA_MB_FAP21V)
       )
    {
      same_familiy = 1;
    }
    else
    {
      same_familiy = 0;
    }
    break;

  case LINE_CARD_GFA_PETRA_DDR2:
  case LINE_CARD_GFA_PETRA_DDR3:
  case LINE_CARD_GFA_PETRA_DDR3_STREAMING:
  case LINE_CARD_GFA_PETRA_B_WITH_PETRA_A_DDR3:
  case LINE_CARD_GFA_PETRA_B_WITH_PETRA_A_DDR3_STREAMING:
    if (
      (card_y == LINE_CARD_GFA_PETRA_DDR2) ||
      (card_y == LINE_CARD_GFA_PETRA_DDR3) ||
      (card_y == LINE_CARD_GFA_PETRA_DDR3_STREAMING) ||
      (card_y == LINE_CARD_GFA_PETRA_B_WITH_PETRA_A_DDR3) ||
      (card_y == LINE_CARD_GFA_PETRA_B_WITH_PETRA_A_DDR3_STREAMING) ||
      (card_y == LOAD_BOARD_PB) ||
      (card_y == LINE_CARD_GFA_PETRA_A) ||
      (card_y == LINE_CARD_GFA_PETRA_X)
      )
    {
      same_familiy = 1;
    }
    else
    {
      same_familiy = 0;
    }
    break;
  case LINE_CARD_GFA_PETRA_B_DDR3:
  case LINE_CARD_GFA_PETRA_B_DDR3_STREAMING:
    if (
      (card_y == LINE_CARD_GFA_PETRA_DDR2) ||
      (card_y == LINE_CARD_GFA_PETRA_DDR3) ||
      (card_y == LINE_CARD_GFA_PETRA_DDR3_STREAMING) ||
      (card_y == LINE_CARD_GFA_PETRA_B_DDR3) ||
      (card_y == LINE_CARD_GFA_PETRA_B_DDR3_STREAMING) ||
      (card_y == LINE_CARD_GFA_PETRA_B_WITH_PETRA_A_DDR3) ||
      (card_y == LINE_CARD_GFA_PETRA_B_WITH_PETRA_A_DDR3_STREAMING) ||
      (card_y == LOAD_BOARD_PB) ||
      (card_y == LINE_CARD_GFA_PETRA_B) ||
      (card_y == LINE_CARD_GFA_PETRA_X)
      )
    {
      same_familiy = 1;
    }
    else
    {
      same_familiy = 0;
    }
    break;
  case LINE_CARD_GFA_PETRA_X:
  case LOAD_BOARD_PB:
    if (
        (card_y == LINE_CARD_GFA_PETRA_DDR2) ||
        (card_y == LINE_CARD_GFA_PETRA_DDR3) ||
        (card_y == LINE_CARD_GFA_PETRA_DDR3_STREAMING) ||
        (card_y == LINE_CARD_GFA_PETRA_B_DDR3) ||
        (card_y == LINE_CARD_GFA_PETRA_B_DDR3_STREAMING) ||
        (card_y == LINE_CARD_GFA_PETRA_B_WITH_PETRA_A_DDR3) ||
        (card_y == LINE_CARD_GFA_PETRA_B_WITH_PETRA_A_DDR3_STREAMING) ||
        (card_y == LOAD_BOARD_PB) ||
        (card_y == LINE_CARD_GFA_PETRA_A) ||
        (card_y == LINE_CARD_GFA_PETRA_B) ||
        (card_y == LINE_CARD_GFA_PETRA_X)
       )
    {
      same_familiy = 1;
    }
    else
    {
      same_familiy = 0;
    }
    break;
  case LINE_CARD_GFA_PETRA_A:
    if (
      (card_y == LINE_CARD_GFA_PETRA_DDR2) ||
      (card_y == LINE_CARD_GFA_PETRA_DDR3) ||
      (card_y == LINE_CARD_GFA_PETRA_DDR3_STREAMING) ||
      (card_y == LINE_CARD_GFA_PETRA_B_WITH_PETRA_A_DDR3) ||
      (card_y == LINE_CARD_GFA_PETRA_B_WITH_PETRA_A_DDR3_STREAMING) ||
      (card_y == LOAD_BOARD_PB) ||
      (card_y == LINE_CARD_GFA_PETRA_A) ||
      (card_y == LINE_CARD_GFA_PETRA_X)
      )
    {
      same_familiy = 1;
    }
    else
    {
      same_familiy = 0;
    }
    break;
  case LINE_CARD_GFA_PETRA_B:
    if (
      (card_y == LINE_CARD_GFA_PETRA_B_DDR3) ||
      (card_y == LINE_CARD_GFA_PETRA_B_DDR3_STREAMING) ||
      (card_y == LOAD_BOARD_PB) ||
      (card_y == LINE_CARD_GFA_PETRA_B) ||
      (card_y == LINE_CARD_GFA_PETRA_X)
      )
    {
      same_familiy = 1;
    }
    else
    {
      same_familiy = 0;
    }
    break;
  case LINE_CARD_TEVB:
  case FRONT_END_TEVB:
    if (
      (card_y == LINE_CARD_TEVB) ||
      (card_y == FRONT_END_TEVB)
      )
    {
      same_familiy = 1;
    }
    else
    {
      same_familiy = 0;
    }
    break;

  case FABRIC_CARD_FE600_01:
  case LOAD_BOARD_FE600:
    if (
      (card_y == FABRIC_CARD_FE600_01) ||
      (card_y == LOAD_BOARD_FE600)
      )
    {
      same_familiy = 1;
    }
    else
    {
      same_familiy = 0;
    }
    break;

  default:
    /*general case - only equality is considered same family*/
    if (card_x == card_y)
    {
      same_familiy = 1;
    }
    else
    {
      same_familiy = 0;
    }
  }

  return same_familiy;
}


/*****************************************************
*NAME
*   bsp_card_enum_to_str
*FUNCTION:
*  Convert BSP_CARDS_DEFINES to STR.
*INPUT:
*  DIRECT:
*    const BSP_CARDS_DEFINES card_type -
*****************************************************/
const char *
  bsp_card_enum_to_str(
    const BSP_CARDS_DEFINES card_type
  )
{
  const char
    *str = "";

  switch(card_type)
  {
    case(STANDALONE_MEZZANINE):
      str = "Stand alone";
      break;
    case(FABRIC_CARD_01):
      str = "Fabric card";
      break;
    case(LINE_CARD_01):
      str = "Line card";
      break;
    case(FABRIC_CARD_FE600_01):
      str = "FE600 card";
      break;
    case(LINE_CARD_FAP10M_B):
      str = "Line card 10M-B";
      break;
    case(LINE_CARD_GFA_FAP20V):
      str = "Line card GFA FAP20V";
      break;
    case(LINE_CARD_GFA_FAP21V):
      str = "Line card GFA FAP21V";
      break;
    case(LINE_CARD_GFA_PETRA_DDR3):
      str = "Line card GFA FAP80 DDR3";
      break;
    case(LINE_CARD_GFA_PETRA_DDR2):
      str = "Line card GFA FAP80 DDR2";
      break;
    case(LINE_CARD_GFA_PETRA_DDR3_STREAMING):
      str = "Line card GFA FAP80 DDR3 Streaming";
      break;
    case(LINE_CARD_GFA_PETRA_B_DDR3):
      str = "Line card GFA FAP100 DDR3";
      break;
    case(LINE_CARD_GFA_PETRA_B_DDR3_STREAMING):
      str = "Line card GFA FAP100 DDR3 Streaming";
      break;
    case(LINE_CARD_GFA_PETRA_B_WITH_PETRA_A_DDR3):
      str = "Line card GFA FAP100 DDR3 with FAP80 assembly";
      break;
    case(LINE_CARD_GFA_PETRA_B_WITH_PETRA_A_DDR3_STREAMING):
      str = "Line card GFA FAP100 DDR Streaming with FAP80 assembly";
      break;
    case(LINE_CARD_GFA_MB_FAP20V):
      str = "Line card GFA MB";
      break;
    case(LINE_CARD_GFA_MB_FAP21V):
      str = "Line card GFA MB";
      break;
    case(FRONT_END_FTG):
      str = "Front End FTG";
      break;
    case(FRONT_END_TGS):
      str = "Front End TGS";
      break;
    case(FRONT_END_PTG):
      str = "Front End PTG";
      break;
    case(FAP_MEZ_GDDR1):
      str = "Fap Mezzanine GDDR1";
      break;
    case(FAP_MEZ_DDR2):
      str = "Fap Mezzanine DDR2";
      break;
    case(FRONT_END_TEVB):
      str = "Front End Timna Evalution Board";
      break;
    case(LINE_CARD_TEVB):
      str = "Line Card Timna Evalution Board";
      break;
    case(FAP_MEZ_FAP21V):
      str = "Fap21v Mezzanine";
      break;
    case(LOAD_BOARD_FE600):
      str = "Load Board for FE 600";
      break;
    case(LOAD_BOARD_PB):
      str = "Load Board for Petra B";
      break;
    case(LINE_CARD_GFA_PETRA_A):
      str = "Petra A line card familiy";
      break;
    case(LINE_CARD_GFA_PETRA_B):
      str = "Petra B line card familiy";
      break;
    case(LINE_CARD_GFA_PETRA_X):
      str = "Petra A or Petra B line card familiy";
      break;
    case(LINE_CARD_GFA_PETRA_B_INTERLAKEN):
      str = "Line card GFA BI";
      break;
    case(LINE_CARD_GFA_PETRA_B_INTERLAKEN_2):
      str = "Line card GFA BI 2";
      break;
    case(FABRIC_CARD_FE1600):
      str = "Fabric card FE1600";
      break;
    case(FABRIC_CARD_FE1600_BCM88754):
      str = "Fabric card FE1600 BCM88754";
      break;
    case(FABRIC_CARD_FE3200):
      str = "Fabric card FE3200";
      break;
    case(UNKNOWN_CARD):
    default:
      str = "Unknown Card";
      break;
  }

  return str;
}

/* } */

