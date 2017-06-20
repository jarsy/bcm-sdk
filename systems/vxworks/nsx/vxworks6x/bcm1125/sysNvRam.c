/* sysNvRam.c - CFE interface to serial nvram */

/* Copyright 2002-2004 Wind River Systems, Inc. */

/* $Id: sysNvRam.c,v 1.3 2011/07/21 16:14:49 yshtil Exp $
**********************************************************************
*
*  Copyright 2000,2001
*  Broadcom Corporation. All rights reserved.
*
*  This software is furnished under license to Wind River Systems, Inc.
*  and may be used only in accordance with the terms and conditions
*  of this license.  No title or ownership is transferred hereby.
***********************************************************************
*/

/*
 * This file has been developed or significantly modified by the
 * MIPS Center of Excellence Dedicated Engineering Staff.
 * This notice is as per the MIPS Center of Excellence Master Partner
 * Agreement, do not remove this notice without checking first with
 * WR/Platforms MIPS Center of Excellence engineering management.
 */

/*
modification history
--------------------
01k,08nov04,mdo  Documentation fixes for apigen
01j,05may04,agf  fix compiler warnings
01i,18dec02,agf  modify sysNvRamSet to pass nvramEnvSet return value
01h,18dec02,agf  set M24LV128_EEPROM define to 1
01g,18dec02,agf  correct env driver logic and variable type errors
01f,04oct02,agf  fix call for writing to M24LV128; disable DEBUG msgs
01e,03oct02,agf  changes for shared sentosa support
01d,20jun02,pgh  Change path to bcm1250Lib.h.
01c,13jun02,pgh  Fix SPR 76014 and SPR 76016.
01b,26mar02,tlc  Clean up compiler warnings.
01a,08jan01,agf  written
*/

/*
DESCRIPTION

INCLUDE FILES:
*/

/* includes */

#include "vxWorks.h"

#include "bcm1250.h"
#include "bcm1250Lib.h"
#include "sysNvRam.h"
#include "x1240RtcEeprom.h"
#include "m24lv128Eeprom.h"
#if defined(BCM1250_SWARM)
#include "swarm.h"
#elif defined(BCM1250_SENTOSA)
#include "sentosa.h"
#endif  /* defined(BCM1250_SWARM) */


/* defines */

#define ENV_EXIST        1
#define ENV_NONEXIST     0

#define X1240_EEPROM     0
#define M24LV128_EEPROM  1

#define DEBUG_SMBUS_IO   0
#define DEBUG_NV_RAM     0

typedef struct
    {
    int        flag;
    uint8_t *  ptr;
    int        len;
    } env_t;


/* externals */


/* forward declarations */


/* globals */


/* locals */



/******************************************************************************
*
* strnchr - search a string
*
* This routine searches a string until, at most N characters or the first
* occurrence of a given character.
*
* RETURNS: pointer to string or NULL
*
* ERRNO
*/

static char * strnchr
    (
    const char *    dest,
    char            c,
    unsigned int    cnt
    )
    {
    while (*dest && (cnt > 0))
        {
        if (*dest == c)
            return (char *) dest;
        dest++;
        cnt--;
        }
    return (char *)NULL;
    }


/******************************************************************************
*
* nvramEnvSearch - search a character array
*
* This routine searches the given character array of environment variables
* for a given name.
*
* RETURNS: OK - env->flag: ENV_EXIST, or ERROR - env->flag: ENV_NONEXIST
*
* ERRNO
*/

static STATUS nvramEnvSearch
    (
    char *     name,
    uint8_t *  buf,
    int        size,
    env_t *    env
    )
    {
    uint8_t * ptr;
    unsigned char * envval;
    unsigned int    offset;
    unsigned int    flg;
    unsigned int    reclen;
    unsigned int    rectype;

    ptr = buf;
    offset = 0;

    while ((*ptr != ENV_TLV_TYPE_END)  && (size > 1))
        {
        /* Adjust pointer for TLV type */
        rectype = *ptr++;
        offset++;
        size--;

        /*
         * Read the length.  It can be either 1 or 2 bytes
         * depending on the code
         */
        if (rectype & ENV_LENGTH_8BITS)
            {
            /* Read the record type and length - 8 bits */
            reclen = *ptr++;
            size--;
            offset++;
            }
        else {
            /* Read the record type and length - 16 bits, MSB first */
            reclen = ((unsigned int) *(ptr) << 8) + *(ptr + 1);
            ptr += 2;
            size -= 2;
            offset += 2;
            }

        if (reclen > size)
            return ERROR;       /* should not happen, bad NVRAM */

        switch (rectype)
            {
            case ENV_TLV_TYPE_ENV:
                /* Read the TLV data */
                flg = *ptr++;
                envval = (unsigned char *) strnchr ((char *)ptr, '=', (reclen - 1));
                if (envval && !strncmp (name, (char *)ptr, strlen (name)) &&
                    (int)(ptr + strlen (name)) == (int)envval )
                    {
                    env->ptr  = ptr + strlen(name) + 1;
                    env->len = reclen - strlen(name) - 2;
                    env->flag = ENV_EXIST;
                    return OK;
                    }
                break;

            default:
                /* Unknown TLV type, skip it. */
                break;
            }

        /*
         * Advance to next TLV
         */

        size -= (int)reclen;
        offset += reclen;
        ptr = buf + offset;
        }

    env->flag = ENV_NONEXIST;
    env->ptr = (uint8_t *)ptr;
    env->len = size;

    return OK;
    }


/******************************************************************************
*
* nvramEnvGet - get the value associated to the name in non-volatile RAM
*
* This routine copies the value that associated to the specified name in CFE
* environment stored in non-volatile memory into a specified string. The string
* will be terminated with an EOS.
*
* RETURNS: OK, or ERROR if access is outside the non-volatile RAM range.
*
* ERRNO
*/

STATUS nvramEnvGet
    (
    char *  name,
    char *  string,
    int     strLen
    )
    {
    int         res, retlen;
    uint8_t *   buf;
    uint8_t *   ptr;
    env_t       env;
#if DEBUG_SMBUS_IO
    int         boardRev;
#endif
#if DEBUG_NV_RAM
    int         idx;
#endif

#if DEBUG_SMBUS_IO
    boardRev = G_SYS_CONFIG(SBREADCSR(A_SCD_SYSTEM_CFG)) & 0x3;
    printf("board rev: %x\n", boardRev);
#endif

    /* It is also possible to determine which EEPROM to use based on
     * the board revision number; however, the relationship is different
     * between the Swarm and Sentosa, and for maintenance reasons this 
     * module is being used by both of them
     */

    if (x1240RtcEepromOpen (X1240_SMBUS_CHAN) == OK)
        {
#if DEBUG_SMBUS_IO
        printf("found x1240 EEPROM\n");
#endif
        buf = malloc (X1241_EEPROM_SIZE);

        res = x1240RtcEepromRead (X1240_SMBUS_CHAN, 0, buf,
                                  X1241_EEPROM_SIZE, &retlen);

        x1240RtcEepromClose ();
        }
    else if (m24lv128EepromOpen (M24LV128_SMBUS_CHAN) == OK)
        {
#if DEBUG_SMBUS_IO
        printf("found m24lv128 EEPROM\n");
#endif
        buf = malloc (M24LV128_EEPROM_SIZE);

        res = m24lv128EepromRead (M24LV128_SMBUS_CHAN, 0, buf,
                                  M24LV128_EEPROM_SIZE, &retlen);

        m24lv128EepromClose ();
        }
    else
        {
#if DEBUG_SMBUS_IO
        printf ("could not open EEPROM for envGet\n");
#endif
        return ERROR;
        }

#if DEBUG_NV_RAM
    printf ("Offset %d Result %d\n", 0, res);
    for (idx = 0; idx < 512; idx++)
        {
        if ((idx % 16) == 0)
            printf ("\n%03x: ", idx);
        printf ("%02X ", buf[idx]);
        }
    printf ("\n");
#endif

    if ((nvramEnvSearch (name, buf, retlen, &env) == ERROR) ||
        (env.flag == ENV_NONEXIST) ||
        (strLen >= retlen))
        {
        free (buf);
        return ERROR;
        }
    else
        {
        ptr = env.ptr;
        memcpy (string, ptr, strLen);
        }

    free (buf);
    return OK;
    }

/******************************************************************************
*
* nvramEnvSet - set the value and its associated name in non-volatile RAM
*
* This routine sets the value and its associated name to CFE environment
* stored in non-volatile memory.  The value will be appended with 0xff, if
* string length of value is smaller than specified envLen.
*
* RETURNS: OK, or ERROR if access is outside the non-volatile RAM range
*
* ERRNO
*/

STATUS nvramEnvSet
    (
    char *  name,
    char *  string,
    int     envLen
    )
    {
    int         res, retlen, smbusDev;
    uint8_t *   buf;
    uint8_t *   ptr;
    env_t       env;
#if DEBUG_SMBUS_IO
    int         boardRev;
#endif
#if DEBUG_NV_RAM
    int         idx;
#endif

#if DEBUG_SMBUS_IO
    boardRev = G_SYS_CONFIG(SBREADCSR(A_SCD_SYSTEM_CFG)) & 0x3;
    printf("board rev: %x\n", boardRev);
#endif

    /* It is also possible to determine which EEPROM to use based on
     * the board revision number; however, the relationship is different
     * between the Swarm and Sentosa, and for maintenance reasons this 
     * module is being used by both of them
     */

    if (x1240RtcEepromOpen (X1240_SMBUS_CHAN) == OK)
        {
        buf = malloc (X1241_EEPROM_SIZE);

        res = x1240RtcEepromRead (X1240_SMBUS_CHAN, 0, buf,
                                  X1241_EEPROM_SIZE, &retlen);

        smbusDev = X1240_EEPROM;    /* save for EepromWrite () call */
        }
    else if (m24lv128EepromOpen (M24LV128_SMBUS_CHAN) == OK)
        {
        buf = malloc (M24LV128_EEPROM_SIZE);

        res = m24lv128EepromRead (M24LV128_SMBUS_CHAN, 0, buf,
                                  M24LV128_EEPROM_SIZE, &retlen);

        smbusDev = M24LV128_EEPROM; /* save for EepromWrite () call */
        }
    else
        {
#if DEBUG_SMBUS_IO
        printf ("could not open EEPROM for envSet");
#endif
        return ERROR;
        }


    if (nvramEnvSearch (name, &buf[0], retlen, &env) == ERROR)
        {
        free (buf);
        x1240RtcEepromClose ();
        m24lv128EepromClose ();
        return ERROR;
        }

    if (env.flag == ENV_NONEXIST)
        {
        /* type + len + flg + name + '=' + value + end */
        if ((1 + 1 + 1 + strlen (name) + 1 + envLen + 1) > env.len)
            {
            free (buf);
            x1240RtcEepromClose ();
            m24lv128EepromClose ();
            return ERROR;
            }

        ptr = env.ptr;

        *ptr++ = ENV_TLV_TYPE_ENV;                 /* type */
        *ptr++ = strlen (name) + envLen + 1 + 1;   /* length */
        *ptr++ = ENV_FLG_NORMAL;                   /* flag */
        memcpy (ptr, name, strlen (name));         /* name */
        ptr += strlen (name);
        *ptr++ = '=';                              /* '=' */
        memcpy (ptr, string, envLen);              /* value */
        ptr += envLen;
        *ptr = ENV_TLV_TYPE_END;                   /* type_end */

        /* increase 'envLen' by environment overhead so call to
         * EepromWrite routine has proper size field
         */

        envLen += 5 + strlen (name);
        }
    else
        {
#if DEBUG_NV_RAM
        printf ("strlen %d env.len %d\n", envLen, env.len);
#endif
        if (env.len < envLen)
            {
            free (buf);
            x1240RtcEepromClose ();
            m24lv128EepromClose ();
            return ERROR;
            }
        ptr = env.ptr;
        memcpy (env.ptr, string, envLen);       /* copy value */
        ptr += envLen;

        /* fill in 0xff to space left */

        while (envLen < env.len)
            {
            *ptr++ = 0xFF;
            envLen++;
            }
        }

#if DEBUG_NV_RAM
    for (idx = 0; idx < 512; idx++)
        {
        if ((idx % 16) == 0)
            printf ("\n%03x: ", idx);
        printf ("%02X ", buf[idx]);
        }
    printf("\n");
#endif

    switch (smbusDev)
        {
        case X1240_EEPROM:
            res = x1240RtcEepromWrite (X1240_SMBUS_CHAN, 
                                       ((int)env.ptr - (int)buf), 
                                       env.ptr,
                                       envLen, 
                                       &retlen);
           break;

        case M24LV128_EEPROM:
            res = m24lv128EepromWrite (M24LV128_SMBUS_CHAN, 
                                       ((int)env.ptr - (int)buf), 
                                       env.ptr,
                                       envLen, 
                                       &retlen);
           break;

        }

#if DEBUG_SMBUS_IO
    printf ("eepromWrite() Result %d, retlen = %d\n", res, retlen);
#endif

    free (buf);
    x1240RtcEepromClose ();
    m24lv128EepromClose ();
    return OK;
    }


/******************************************************************************
*
* sysNvRamGet - get the contents of non-volatile RAM
*
* This routine copies the contents of non-volatile memory into a specified
* string.  The string will be terminated with an EOS.
*
* RETURNS: OK, or ERROR if access is outside the non-volatile RAM range.
*
* ERRNO
*
* SEE ALSO: sysNvRamSet()
*/

STATUS sysNvRamGet
    (
    char *string,    /* where to copy non-volatile RAM           */
    int  strLen,     /* maximum number of bytes to copy          */
    int  offset      /* byte offset into non-volatile RAM        */
    )
    {
    char s[NV_RAM_SIZE+1];
    int key;
    int zz;

    if ((strLen < 0) || (offset < 0) || ((offset + strLen) > NV_RAM_SIZE))
        return (ERROR);

    bzero (s, NV_RAM_SIZE+1);
    key = intLock ();
    zz = nvramEnvGet ("vxwbootline", s, NV_RAM_SIZE);
    intUnlock (key);

    if (zz != OK)
        {
#if DEBUG_SMBUS_IO
        printf ("could not find vxwBootline in nvRAM\n");
#endif
        return (ERROR);
        }

    strncpy (string, s+offset, strLen);
    string[strLen+offset] = EOS;

    return (OK);
    }


/*******************************************************************************
*
* sysNvRamSet - write to non-volatile RAM
*
* This routine copies a specified string into non-volatile RAM.
*
* RETURNS: OK, or ERROR if access is outside the non-volatile RAM range.
*
* ERRNO
*
* SEE ALSO: sysNvRamGet()
*/

STATUS sysNvRamSet
    (
    char *  string,     /* string to be copied into non-volatile RAM */
    int     strLen,       /* maximum number of bytes to copy           */
    int     offset        /* byte offset into non-volatile RAM         */
    )
    {
    char    s[NV_RAM_SIZE+1];
    int     key;
    int     zz;

    if ((strLen < 0) || (offset < 0) || ((offset + strLen) > NV_RAM_SIZE))
        return (ERROR);

    key = intLock ();
    nvramEnvGet ("vxwbootline", s, NV_RAM_SIZE);
    strncpy (s+offset, string, strLen);
    *(s+offset+strLen) = '\0';

    zz = nvramEnvSet ("vxwbootline", s, NV_RAM_SIZE);
    intUnlock (key);

    return (zz);
    }



