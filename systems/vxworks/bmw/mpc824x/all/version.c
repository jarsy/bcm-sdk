/* version.c - creation version/date/time module */

#include "vxWorks.h"
#include "version.h"

/* $Id: version.c,v 1.5 2011/07/21 16:14:11 yshtil Exp $
modification history
--------------------
01a,06mar96,dat	written
*/

/*
This module is always built with each executable image.  It provides
the VxWorks version id, and the time and date it was built.

The ANSI predefined macros __DATE__ and __TIME__ are used to provide
the date/time information.  ANSI compliant compilers are required for
building all VxWorks executables.
*/

#if VX_VERSION == 55 || VX_VERSION == 62
char * runtimeName    = RUNTIME_NAME;
char * runtimeVersion = RUNTIME_VERSION;
char * vxWorksVersion = VXWORKS_VERSION;

#ifdef RUNTIME_CREATION_DATE
char * creationDate   = RUNTIME_CREATION_DATE;
#else
char * creationDate   = __DATE__ ", " __TIME__;
#endif

#else
char * vxWorksVersion = VXWORKS_VERSION;
char * creationDate   = __DATE__ ", " __TIME__;
#endif
