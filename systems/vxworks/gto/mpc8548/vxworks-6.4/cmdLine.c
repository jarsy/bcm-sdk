/* cmdLine.c - command line build file include  */

/* $Id: cmdLine.c,v 1.2 2011/07/21 16:14:13 yshtil Exp $
 * Copyright (c) 2005 Wind River Systems, Inc.
 *
 * The right to copy, distribute, modify, or otherwise make use
 * of this software may be licensed only pursuant to the terms
 * of an applicable Wind River license agreement.
 */

/*
modification history
--------------------
01a,04dec05,dtr  Created
*/

#ifndef INC_cmdLineH
#define INC_cmdLineH
#ifdef INCLUDE_VXBUS
#ifndef PRJ_BUILD

#include <vxWorks.h>
#include <config.h>

#include <../src/hwif/util/cmdLineBuild.c>

#endif /* PRJ_BUILD */
#endif /* INCLUDE_VXBUS */
#endif /* INC_cmdLineH */
