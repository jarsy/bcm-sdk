/*
 * $Id: configSdkPre.h,v 1.4 2012/03/02 15:27:07 yaronm Exp $
 * $Copyright: Copyright 2007, Broadcom Corporation All Rights Reserved.
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES
 * OF ANY KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE.
 * BROADCOM SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.$
 * Description: This file configure the VxWorks kernel to includes
 *              additional components necessary for SDK.
 *              Includes this file before configAll.h
 *              in your BSP config.h
 */

#ifndef _SDK_CONFIG_PRE_H_
#define _SDK_CONFIG_PRE_H_

#define INET
/* #define INET6 */

#endif /* _SDK_CONFIG_PRE_H_ */
