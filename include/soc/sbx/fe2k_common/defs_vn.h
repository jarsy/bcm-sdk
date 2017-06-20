/******************************************************************************
 * $Id: defs_vn.h,v 1.7 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 ******************************************************************************/

/*********************************************************************
 Beg of File: defs_vn.h
*********************************************************************/

#ifndef defs_vn__h_defined
#define defs_vn__h_defined

#include <shared/alloc.h>
#include <sal/core/libc.h>

#ifndef ASSERT_VN
#include <assert.h>
#define ASSERT_VN assert                                                                               
#endif /*ASSERT_VN*/ 


#ifdef BCM_CALADAN3_SUPPORT
#define MALLOC_PKGC3(x) sal_alloc((x), "C3AsmIntf")
#define FREE_PKGC3(x) sal_free((x))
#endif

#ifdef NJ_BUILD
#define MALLOC_VN host_malloc
#define FREE_VN host_free
#endif /*NJ_BUILD*/                                                                                          

#ifndef MALLOC_VN
#ifndef __KERNEL__
#include <stdlib.h>/*for the exit()*/
#endif
#define MALLOC_VN sal_alloc
#endif /*MALLOC_VN*/                                                                                           

#ifndef FREE_VN
#ifndef __KERNEL__
#include <stdlib.h>/*for the exit()*/
#endif
#define FREE_VN sal_free
#endif /*MALLOC_VN*/ 

#ifndef STRLEN_VN
#ifndef __KERNEL__
#include <string.h>
#endif
#define STRLEN_VN strlen
#endif  /*STRLEN_VN*/

#ifndef STRNCMP_VN
#ifndef __KERNEL__
#include <string.h>
#endif
#define STRNCMP_VN strncmp 
#endif  /*STRNCMP_VN*/

#ifndef STRNCPY_VN
#ifndef __KERNEL__
#include <string.h>
#endif
#define STRNCPY_VN strncpy
#endif  /*STRNCPY_VN*/

#ifndef MEMSET_VN
#ifndef __KERNEL__
#include <string.h>
#endif
#define MEMSET_VN sal_memset
#endif /*MEMSET_VN*/                                                                                         

#ifndef BOOL_VN
#define BOOL_VN int
#endif /*BOOL_VN*/

#ifndef TRUE_VN
#define TRUE_VN 1
#endif /*TRUE_VN*/

#ifndef FALSE_VN
#define FALSE_VN 0
#endif /*FALSE_VN*/

#ifndef ITEMS_VN
#define ITEMS_VN(x) (sizeof(x)/sizeof(x[0]))
#endif  /*ITEMS_VN*/

#ifndef MIN_VN
#define MIN_VN(x,y) ((x)>(y)?y:x)
#endif  /*MIN_VN*/

#ifndef MAX_VN
#define MAX_VN(x,y) ((x)<(y)?y:x)
#endif  /*MAX_VN*/

#ifndef STRMAX_VN
#define STRMAX_VN 128
#endif /*STRMAX_VN*/

#endif /* defs_vn__h_defined */

/*********************************************************************
 End of File: defs_vn.h
*********************************************************************/

