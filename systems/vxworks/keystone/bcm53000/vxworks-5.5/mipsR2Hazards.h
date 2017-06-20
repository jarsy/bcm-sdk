/* mipsR2Hazards.h - assembler definitions header file */

/* Copyright  2009 Wind River Systems, Inc. */

/* $Id: mipsR2Hazards.h,v 1.3 2011/07/21 16:14:25 yshtil Exp $
modification history
--------------------
01a,15dec09,mdg  Created 
*/

/* These hazard macros are intended for use with MIPS architecture-
 * dependent assembly files which require handling hazards.
 * These macros support release 2 ISA cores
 * This file is used by the BSP assembly files to override any macros
 * defined in asmMips.h.  This is needed because of the way that the
 * Tornado project facility parses dependencies.
 */

#ifndef __INCmipsR2Hazardsh
#define __INCmipsR2Hazardsh


/* Undefine hazard macros that may have been defined in asmMips.h */
#undef EHB	                
#undef HAZARD_TLB		    
#undef HAZARD_ERET	    	
#undef HAZARD_CP_READ		
#undef HAZARD_CP_WRITE		
#undef HAZARD_CACHE_TAG	
#undef HAZARD_CACHE		
#undef HAZARD_INTERRUPT	

/* Hazard macros */
/* Keep macro definition for backward compatability */
#define EHB	                .word 0x000000c0

#define HAZARD_TLB		    .word 0x000000c0
#define HAZARD_ERET	    	.word 0x000000c0
#define HAZARD_CP_READ		.word 0x000000c0
#define HAZARD_CP_WRITE		.word 0x000000c0
#define HAZARD_CACHE_TAG	.word 0x000000c0
#define HAZARD_CACHE		.word 0x000000c0
#define HAZARD_INTERRUPT	.word 0x000000c0 ; nop

#endif /* __INCmipsR2Hazardsh */

