/*
 * $Id: cmdlist.c,v 1.19 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * socdiag command list
 *
 * Since this version of the socdiag is run on top of the bcm-core 
 * module, we need to exclude certain features. We cannot do this
 * through the makefile because we share the diag library with
 * the bcm-diag-full module, which needs all features enabled.
 *
 */

#ifdef INCLUDE_BCMX_DIAG
#undef INCLUDE_BCMX_DIAG
#endif

#ifdef INCLUDE_BCMX
#undef INCLUDE_BCMX
#endif

#ifdef INCLUDE_LIB_CPUDB
#undef INCLUDE_LIB_CPUDB
#endif

#ifdef INCLUDE_TEST
#undef INCLUDE_TEST
#endif

#ifndef NO_SAL_APPL
#ifndef NO_MEMTUNE
#define NO_MEMTUNE
#endif
#endif

#include "../../../../src/appl/diag/cmdlist.c"
