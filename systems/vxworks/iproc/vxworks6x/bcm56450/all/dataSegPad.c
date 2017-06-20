/* dataSegPad.c - padding for beginning of data segment */

/* Copyright 1984-2004 Wind River Systems, Inc. */
#include "copyright_wrs.h"

/*
modification history
--------------------
01g,27sep04,tam  replaced INCLUDE_EDR_KH by INCLUDE_KERNEL_HARDENING
01f,02sep04,pes  Conditionalize the alignment of dataSegPad
01e,08jun04,tam  added EDR_KH handling
01d,03may04,tam  updated data segment alignment code
01c,19oct92,jcf  change to include when INCLUDE_MMU_FULL defined.
01b,28jul92,rdc  changed PAGE_SIZE to VM_PAGE_SIZE.
01a,21jul92,rdc  written.
*/

/*
DESCRIPTION

This module conditionally defined the variable dataSegPad which is aligned
on a MMU page size biundary by default; it is explicility listed as the first
module on the load line when VxWorks is linked to insure that this data
structure is the first item in the data segment. This mechanism is needed
to insure that the data segment does not overlap a page that is occupied
by the text segment;  when text segment protection is turned on, all pages
that contain text are write protected. This insures that the data segment
does not lie in a page that has been write protected. If text segment
protection has not been included, this module compiles into a null object
module. Note that if the data segment needs to be aligned on a boundary
larger than an MMU page size, then KERNEL_DATA_SEG_ALIGN should be defined
to that alignment, otherwise it defaults to a MMU page size (VM_PAGE_SIZE).

In an embedded system, this mechanism may not be needed if the loader
explicitly places the data segment in a section of memory seperate from
the text segment.
*/

#include "vxWorks.h"
#include "config.h"

#if !defined (_WRS_LINKER_DATA_SEG_ALIGN)
/*
 * TODO: When SPR96639, filed against the DIAB toolchain, is fixed, the code in
 * between #ifndef SPR96639_FIXED ...  #endif should be removed.
 */

#ifndef	SPR96639_FIXED
typedef struct kernelDataSeg{
    char data1;
    char data2;
} KERNEL_DATA_SEG;
#endif	/* SPR96639_FIXED */

#ifndef	KERNEL_DATA_SEG_ALIGN
#define	KERNEL_DATA_SEG_ALIGN	VM_PAGE_SIZE
#endif	/* KERNEL_DATA_SEG_ALIGN */

#if	(defined(INCLUDE_PROTECT_TEXT) || defined(INCLUDE_KERNEL_HARDENING))
/* bootroms will not ref dataSegPad.o */

#ifndef	SPR96639_FIXED		/* code to use when SPR #96639 is fixed */
KERNEL_DATA_SEG	_WRS_DATA_ALIGN_BYTES(KERNEL_DATA_SEG_ALIGN) dataSegPad[1] = {{1}};
#else
char  	_WRS_DATA_ALIGN_BYTES(KERNEL_DATA_SEG_ALIGN) dataSegPad = 1;
#endif	/* SPR96639_FIXED */

#endif /* INCLUDE_PROTECT_TEXT */
#else /* _WRS_LINKER_DATA_SEG_ALIGN */
/*
 * declaration for architectures that use the linker to ensure proper data
 * segment alignment.
 */
char  	dataSegPad = 1;
#endif /* _WRS_LINKER_DATA_SEG_ALIGN */
