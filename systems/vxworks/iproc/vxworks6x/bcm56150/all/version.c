/* version.c - creation version/date/time module */

/*
 * Copyright (c) 1996, 2001, 2004, 2006-2008 Wind River Systems, Inc.
 *
 * The right to copy, distribute, modify or otherwise make use
 * of this software may be licensed only pursuant to the terms
 * of an applicable Wind River License agreement.
 */

/*
modification history
--------------------
01f,25aug08,jpb Removed SMP specific code.  Not used in bsp builds.
01e,05mar07,kk  changed runtimeVersion and runtimeName for SMP
01d,15nov06,cjj Use different runtime name and VxWorks version for SMP
01c,11may04,md  add global variables for VxWorks version numbers
01b,17jan01,sn  simplify job of host val  
01a,06mar96,dat	written
*/

/*
This module is always built with each executable image.  It provides
the VxWorks version id, and the time and date it was built.

The date stamp may be overriden by defining RUNTIME_CREATION_DATE. This
will be primarily used by tools that compare images built on different hosts 
(host validation).

The ANSI predefined macros __DATE__ and __TIME__ are used to provide
the date/time information.  ANSI compliant compilers are required for
building all VxWorks executables.
*/

#include <vxWorks.h>
#include <version.h>

/* numerical values for VxWorks version */

const unsigned int vxWorksVersionMajor = _WRS_VXWORKS_MAJOR;
const unsigned int vxWorksVersionMinor = _WRS_VXWORKS_MINOR;
const unsigned int vxWorksVersionMaint = _WRS_VXWORKS_MAINT;

/* string identifiers for VxWorks and VxWorks SMP version */

char * runtimeName    = RUNTIME_NAME;
char * vxWorksVersion = VXWORKS_VERSION;
char * runtimeVersion = RUNTIME_VERSION;

#ifdef RUNTIME_CREATION_DATE
char * creationDate   = RUNTIME_CREATION_DATE;
#else
char * creationDate   = __DATE__ ", " __TIME__;
#endif
