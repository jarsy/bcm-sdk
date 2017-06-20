/**  -*-  Mode:C; c-basic-offset:4 -*-
 *  sbG2EplibVersion.cx
 *
 * <pre>
 *
 * THIS IS A GENERATED FILE - DO NOT EDIT
 *
 *
 * ========================================================================
 * == sbG2EplibVersion.cx - Version Information for G2Eplib ==
 * ========================================================================
 *
 * $Id: sbG2EplibVersion.c,v 1.6 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * MODULE NAME:
 *
 *     sbG2EplibVersion.cx
 *
 * ABSTRACT:
 *
 *     Version Information For:  G2Eplib, Non Released Test Software
 *     Sandburst Product Code:   10
 *     Major Version:            0
 *     Minor Version:            0
 *     Patch Level:              999999
 *
 * LANGUAGE:
 *
 *     C
 *
 * CREATION DATE:
 *
 *     Tue May  1 12:09:20 2007
 *
 * </pre>
 */

#include "sbTypes.h"
#include "sbStatus.h"
#include "sbWrappers.h"

/*
 * Declare the prototype
 */
sbStatus_t sbG2EplibVersionGet( sbSwLibVersion_p_t pLibVersion );

/**
 *
 * Retrieve the version information for the G2Eplib Library
 *
 * @param pLibVersion  Pointer to an sbSwLibVersion_t.
 *
 * @return             Status, 0 always.
 */
sbStatus_t sbG2EplibVersionGet( sbSwLibVersion_p_t pLibVersion )
{
    sbSwLibVersion_t *pLibV = pLibVersion;
    SB_ASSERT(pLibVersion);

    pLibV->productCode = 10;
    pLibV->majorVersion = 0;
    pLibV->minorVersion = 0;
    pLibV->patchLevel = 999999;
    /* coverity[secure_coding] */
    sal_strcpy(pLibV->description, "Non Released Test Software");

    return( 0 );
}
