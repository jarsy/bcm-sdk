/* $Id: ui_rom_defi.h,v 1.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
#ifndef UI_ROM_DEFI_INCLUDED
/* { */
#define UI_ROM_DEFI_INCLUDED
/*
 * General include file for reference design.
 */
#if !DUNE_BCM
#ifdef __VXWORKS__
/* { */
  #include <ChipSim/chip_sim.h>
  #include <appl/diag/dpp/ref_sys.h>
  #include <vxWorks.h>
  #include <drv/mem/eeprom.h>
  #include <appl/diag/dpp/ui_defx.h>
  #include <appl/diag/dpp/utils_defx.h>
/* } */
#elif !(defined(LINUX) || defined(UNIX))
/* { */
  #include <bcm_app/dpp/../H/drv/mem/eeprom.h>
/* } */
#endif
#else
  #include <appl/diag/dpp/ref_sys.h>
  #include <appl/diag/dpp/ui_defx.h>
  #include <appl/diag/dpp/utils_defx.h>
#endif

/********************************************************
********************************************************/
extern CONST
   SUBJECT
     Subjects_list_rom[];

#ifdef __VXWORKS__

extern const
  MEMORY_BLOCK
    Memory_block_rom_00[];

extern const
  MEMORY_BLOCK
    Memory_block_rom_01[];

/*
 * This is the new PCI mezzanine block.
 */
extern const
  MEMORY_BLOCK
    Memory_block_rom_02[];

/*
 * This is the new PCI mezzanine block on GFA line-card.
 */

extern const
  MEMORY_BLOCK
    Memory_block_rom_03[];

/*
 * GFA-MB line-card.
 */

extern const
  MEMORY_BLOCK
    Memory_block_rom_gfa_mb[];

/*
 * GFA-SOC_SAND_FAP21V line-card.
 */

extern const
  MEMORY_BLOCK
    Memory_block_rom_gfa_fap21v[];

extern const
  MEMORY_BLOCK
    Memory_block_rom_gfa_petra[];

extern const
  MEMORY_BLOCK
    Memory_block_rom_gfa_petra_streaming[];

/* 
 *  GFA-BI line-card
 */
extern const
  MEMORY_BLOCK
    Memory_block_rom_gfa_bi[];

/*
 * This is front-end TEVB in standalone mode (no GFA connected)
 */
extern const
  MEMORY_BLOCK
    Memory_block_rom_04[];

  /* } __VXWORKS__ */
#endif


/* } */
#endif
