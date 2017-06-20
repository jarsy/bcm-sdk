/*
 * $Id: ucodemgr.h,v 1.12 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * 
 *    Ucode download manager defintions
 *
 */

#ifndef _SBX_CALADN3_UCODEMGR_H
#define _SBX_CALADN3_UCODEMGR_H

#include <soc/sbx/caladan3/asm3/asm3_pkg_intf.h>
#include <soc/sbx/caladan3/lrp.h>

/*
 * Microcode streams are not independent of each other, 
 * changes are managed only at bank level 
 */
#define BANK_DIRTY 1
#define BANK_CLEAN 0
#define BANK_INVALID -1

/* Override C3ASMPREFIX from makefiles, if required */
#ifndef C3ASMPREFIX 
#define C3ASMPREFIX C3Asm3
#endif

#define ASM_ADD_PREFIX(pfx, arg) pfx ## arg
#define ASM_PREFIX(pfx, arg) ASM_ADD_PREFIX(pfx, arg)
#define ASM_TAG(arg) ASM_PREFIX(C3ASMPREFIX, arg)

#define APP_PREFIX(pfx, arg)  ASM_ADD_PREFIX(pfx, arg)
#define APP_TAG(pfx) APP_PREFIX(pfx, __PkgInt__initDefault)

/* Ucode package */
#define ASM_PACKAGE   ASM_TAG(__PkgInt)

/* Asm package iread/iwrite signature */
#define ASM_INST_RW_F   ASM_TAG(__HwUcode)

/* Ucode package */
#define ASM_INIT_FUNC(pfx)   APP_TAG(pfx)


typedef ASM_PACKAGE soc_sbx_caladan3_ucode_pkg_t;

typedef ASM_INST_RW_F (*INST_F);

/* Ucode manager init routine signature */
typedef int (*INIT_F) (int unit, soc_sbx_caladan3_ucode_pkg_t *ucode);

/* ASM ucode package init routine signature */
typedef int (*PKG_INIT_F)(soc_sbx_caladan3_ucode_pkg_t *pkg);

/* ucode application init routine signature */
typedef int (*APP_INIT_F)(int unit,
                           soc_sbx_caladan3_ucode_pkg_t *pkg,
                            uint32 *contexts, uint32 *epoch, int reload);

#define MAX_UCODE_APPS 10

typedef struct ucode_app_s {
    char type[16];
    PKG_INIT_F asminit;
    APP_INIT_F handler;
} ucode_app_t;


/*
 * LRP Ucode download manager
 *
 *     Allows user to download ucode image by various means and 
 * initialize microcode application. Normally changes are always 
 * done to the standby bank, once change are done, the banks are swapped.
 * If allbanks is set, both the banks are always in sync.
 */

typedef struct soc_sbx_caladan3_ucodemgr_s {

   sal_mutex_t   lock;                   /* Limit access when downloading */
   soc_sbx_caladan3_ucode_pkg_t *ucode;  /* Handle to the asm interfaces */
   int bank;                             /* standby bank */
   int allbanks;                         /* download ucode to all banks? */
   int dirty[SOC_SBX_CALADAN3_LR_INST_NUM_BANKS]; 
   int reload;                           /* reload microcode */

   /* should there be a set of uninit calls first?? */
   INIT_F app_init;     /* Handle to the application init */
   INIT_F lr_prepare;   /* Called before ucodeimg is downloaded */
   INIT_F lr_download;  /* Called to dnload img into lrp memory */
   INIT_F lr_done;      /* Called after ucodeimg is downloaded */
   INIT_F asm_init;     /* Called to initialize the package */

   /* Callbacks, used for named constants */
   INST_F lr_iread;    /* Read an instruction from inst memory */ 
   INST_F lr_iwrite;   /* Write an instruction to inst memory */ 

   /* Microcode applications */
   ucode_app_t app_list[MAX_UCODE_APPS];

} soc_sbx_caladan3_ucodemgr_t;



int soc_sbx_caladan3_lr_ucodemgr_init(int unit);

int soc_sbx_caladan3_ucodemgr_register_app(int unit, char *type, APP_INIT_F func, PKG_INIT_F pkg);
APP_INIT_F soc_sbx_caladan3_ucodemgr_find_app(int unit, char* type);
PKG_INIT_F soc_sbx_caladan3_ucodemgr_find_asm_init(int unit, char* type);

int soc_sbx_caladan3_ucodemgr_bank_swapped(int unit);
int soc_sbx_caladan3_ucodemgr_standby_bank_get(int unit);
int soc_sbx_caladan3_ucodemgr_standby_bank_set(int unit, int bank);
int soc_sbx_caladan3_ucodemgr_bank_dirty_get(int unit, int bank);
int soc_sbx_caladan3_ucodemgr_bank_dirty_set(int unit, int bank, int flag);

int soc_sbx_caladan3_ucodemgr_sync_all_banks(int unit);

int soc_sbx_caladan3_ucodemgr_asm_init(int unit, 
                                       soc_sbx_caladan3_ucode_pkg_t *ucode);
int soc_sbx_caladan3_lr_download(int unit, 
                                 soc_sbx_caladan3_ucode_pkg_t *ucode);
int soc_sbx_caladan3_ucodemgr_app_init(int unit, 
                                       soc_sbx_caladan3_ucode_pkg_t *pkg);
int soc_sbx_caladan3_lr_ucode_done(int unit,
                                   soc_sbx_caladan3_ucode_pkg_t *ucode);

int soc_sbx_caladan3_lr_ucode_prepare(int unit,
                                      soc_sbx_caladan3_ucode_pkg_t *ucode);

int soc_sbx_caladan3_ucodemgr_sym_set(int unit,
                                      soc_sbx_caladan3_ucode_pkg_t *ucode,
                                      char *nc, uint32 val);
int soc_sbx_caladan3_ucodemgr_sym_get(int unit,
                                      soc_sbx_caladan3_ucode_pkg_t *ucode,
                                      char *nc, uint32 *val);

void soc_sbx_caladan3_ucodemgr_print_state(int unit);

soc_sbx_caladan3_ucodemgr_t*
 soc_sbx_caladan3_lr_ucodemgr_get(int unit);


/* 
 * Image download options 
 */

/* Download image from package */
int soc_sbx_caladan3_ucodemgr_loadimg_from_pkg(int unit, soc_sbx_caladan3_ucode_pkg_t *pkg, int reset, int force);

/* Download image internal  */
int soc_sbx_caladan3_ucodemgr_loadimg(int unit);




#endif /* _SBX_CALADN3_UCODEMGR_H */
