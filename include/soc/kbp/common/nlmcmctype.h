/*
 * $Id: nlmcmctype.h,v 1.2.8.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


 
/*
 * Replacement macros for ctype functions/macros. 
 * Note that we only support the C-Locale.
 */
#ifndef INCLUDED_NLMCMCTYPE_H
#define INCLUDED_NLMCMCTYPE_H

#ifndef NLMPLATFORM_BCM
#include <nlmcmbasic.h>
#include <nlmcmctype_def.h>
#include <nlmcmexterncstart.h>
#else
#include <soc/kbp/common/nlmcmbasic.h>
#include <soc/kbp/common/nlmcmctype_def.h>
#include <soc/kbp/common/nlmcmexterncstart.h>
#endif

extern unsigned short __NlmCmCtype_TestTbl[] ;

#define __NlmCmCtypeLookup(x,t) (__NlmCmCtype_TestTbl[(x)&0xff] & (1 << t))

#define _NlmCmCtypeLookup(x,t)  __NlmCmCtypeLookup((x),t)


#define NlmCm__isalnum(x)   _NlmCmCtypeLookup((x),_NlmCm__isalnum_pos)
#define NlmCm__isalpha(x)   _NlmCmCtypeLookup((x),_NlmCm__isalpha_pos)
#define NlmCm__iscntrl(x)   _NlmCmCtypeLookup((x),_NlmCm__iscntrl_pos)
#define NlmCm__isdigit(x)   _NlmCmCtypeLookup((x),_NlmCm__isdigit_pos)
#define NlmCm__isxdigit(x)  _NlmCmCtypeLookup((x),_NlmCm__isxdigit_pos)
#define NlmCm__islower(x)   _NlmCmCtypeLookup((x),_NlmCm__islower_pos)
#define NlmCm__isgraph(x)   _NlmCmCtypeLookup((x),_NlmCm__isgraph_pos)
#define NlmCm__isprint(x)   _NlmCmCtypeLookup((x),_NlmCm__isprint_pos)
#define NlmCm__ispunct(x)   _NlmCmCtypeLookup((x),_NlmCm__ispunct_pos)
#define NlmCm__isspace(x)   _NlmCmCtypeLookup((x),_NlmCm__isspace_pos)
#define NlmCm__isupper(x)   _NlmCmCtypeLookup((x),_NlmCm__isupper_pos)
#define NlmCm__isblank(x)   _NlmCmCtypeLookup((x),_NlmCm__isblank_pos)

#define NlmCm__isascii(x)   (((x) & ~0x7f) == 0)
#define NlmCm__toascii(x)   ((x) & 0x7f)

/* Define these as functions to avoid side-effects
 */
extern int NlmCm__tolower(int x) ;
extern int NlmCm__toupper(int x) ;

#ifndef NLMPLATFORM_BCM
#include <nlmcmexterncend.h>
#else
#include <soc/kbp/common/nlmcmexterncend.h>
#endif

#endif


/*[]*/

