/*
 * $Id$
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */
/*
 **************************************************************************************
 Copyright 2009-2012 Broadcom Corporation

 This program is the proprietary software of Broadcom Corporation and/or its licensors,
 and may only be used, duplicated, modified or distributed pursuant to the terms and 
 conditions of a separate, written license agreement executed between you and 
 Broadcom (an "Authorized License").Except as set forth in an Authorized License, 
 Broadcom grants no license (express or implied),right to use, or waiver of any kind 
 with respect to the Software, and Broadcom expressly reserves all rights in and to 
 the Software and all intellectual property rights therein.  
 IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY 
 WAY,AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.  

 Except as expressly set forth in the Authorized License,

 1. This program, including its structure, sequence and organization, constitutes the 
    valuable trade secrets of Broadcom, and you shall use all reasonable efforts to 
    protect the confidentiality thereof,and to use this information only in connection
    with your use of Broadcom integrated circuit products.

 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS" AND WITH 
    ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR WARRANTIES, EITHER 
    EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM 
    SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, 
    NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR 
    COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. 
    YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.

 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS LICENSORS 
    BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES 
    WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE 
    THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; 
    OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF 
    OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING 
    ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 **************************************************************************************
 */
 

/*@@NlmCmFile Module

   Summary
   This module provides the file i/o interface for all Nlmnapse
   components. Nlmnapse does not use naked calls to the standard i/o routines; 
   rather, all Nlmnapse functions conform to this interface.
   
   Description
   The Nlmnapse API and its components do not make any assumptions about the
   file input-output capability of a client. All of Nlmnapse will use the 
   NlmCmFile interface to perform file and console i/o.

   The client is required to provide hooks for the primitive i/o functions
   open,read,write and close that are required by the NlmCmFile interface.

*/
#if 0
#ifndef INCLUDED_NLMCMFILE_H
#define INCLUDED_NLMCMFILE_H

#include <nlmcmbasic.h>
#include <nlmcmallocator.h>
#include <nlmcmvxprintf.h>
#include <nlmcmexterncstart.h>

/* Stub implementations as necessary */

#ifndef NLM_HAVE_FILE_IO
#define NLMCM_CREATE_MODE       0
#define NLMCM_O_WRONLY          0
#define NLMCM_O_RDONLY          0
#define NLMCM_O_CREAT           0
#define NLMCM_O_TRUNC           0
#define NLMCM_O_APPEND	       0

#define NlmCm__sysopen 	NlmCm_BITBUCKET__open
#define NlmCm__sysclose  NlmCm_BITBUCKET__close
#endif /* NLM_HAVE_FILE_IO */

#ifndef NLM_HAVE_CONSOLE
#define NlmCm__sysread 	NlmCm_BITBUCKET__read
#define NlmCm__syswrite	NlmCm_BITBUCKET__write
#endif /* NLM_HAVE_CONSOLE */


int	NlmCm_BITBUCKET__open(const char *, int , mode_t );
size_t	NlmCm_BITBUCKET__write(int, const void*,size_t);
size_t	NlmCm_BITBUCKET__read(int, void*,size_t);
int	NlmCm_BITBUCKET__close(int);


/*
 * Enumerations for NlmCmFile API return values
 */
typedef enum NlmCmFile_Status_t 
{
    NLMCMFILE_SUCCESS,
    NLMCMFILE_NULL_FILE_NAME,
    NLMCMFILE_INVALID_FILE_MODE,
    NLMCMFILE_INVALID_FILE_TYPE,
    NLMCMFILE_ALLOC_ERROR,
    NLMCMFILE_OPEN_ERROR,
    NLMCMFILE_WRITE_ERROR,
    NLMCMFILE_READ_ERROR,
    NLMCMFILE_READ_TRUNCATED,
    NLMCMFILE_CLOSE_ERROR
}NlmCmFile_Status;

/* 
 * Structure definition for NlmCmFile
 */

#ifndef __doxygen__
/* NB: if struct NlmCmFile changes,
 * then the static values of stdin, stdout, stderr
 * in nlmcmfile.c must also change
 */
#endif
typedef struct NlmCmFile
{
    NlmCmAlloc*	m_alloc_p;
    nlm_32 	m_fd;			/* File descriptor id */
    const char*	m_fname;		/* Name of the file */
    nlm_32 	m_status; 		/* Indicates current status of last operation  */
    NlmBool 	m_isbitbucket;   	/* Indicates whether I/O is disabled in the platform */
    NlmBool 	m_pvt_eof;         	/* Indicates whether EOF is reached on this file */
    nlm_u32 	m_pvt_flags;	  	/* eg, read or write operations allowed on the file */
    nlm_u8*	m_pvt_head;		/* head of buffer -- start of buffer */
    nlm_u8*	m_pvt_past;		/* past of buffer -- just past end of buffer */
    nlm_u8*	m_pvt_here;		/* here in buffer -- next byte to use */
    nlm_u8*	m_pvt_tail;		/* tail of buffer -- just past in use portion */
}NlmCmFile;

/* End of File value returned by file API's */
#define NlmCmFile__EOF (-1)

/* Standard Files for console I/O*/
extern NlmCmFile *NlmCm__stdin, *NlmCm__stdout, *NlmCm__stderr ;


/*  NlmCmFile APIs */

/*  Summary
    Opens a file.

    Description
    This function will open the file indicated by sFileName, create
    a NlmCmFile instance and associate it with the file opened. A new file 
    will be created if the file does not exist and the mode specified is
    write or append.
    The character string mode specifies the type of access requested for 
    the file, as follows:
    "r" -   Opens for reading. If the file does not exist or cannot be found, 
	    the fopen call fails.
    "w" -   Opens an empty file for writing. If the given file exists, 
	    its contents are destroyed.
    "a" -   Opens a file for writing. If the given file exists, writing
            begins at the end of the file. If the file doesn't exist,
	    this mode behaves as the "w" mode.
    In addition, one of the following characters can be appended to the mode
    string to indicate the type of the file
    "b" -   binary
    "d" -   null file
    "m" -   memory (not supported in first version)

    The function returns the pointer to the newly created NlmCmFile instance,
    if the open was success, and NULL otherwise. Detailed error information
    will be inidicated in the nRetCode paramater.

    Parameters
    @param pAlloc Pointer to the allocator to use.
    @param sFileName Name of the file to open
    @param sMode Mode in which the file is to be opened.
    @param nRetCode Extended error information
    See Also
    NlmCmFile__fclose
*/
extern NlmCmFile* 
NlmCmFile__fopen(
    NlmCmAllocator* pAlloc, 
    const char* sFileName,
    const char* sMode,
    nlm_u32* nRetCode );

/*  Summary
    Write a character to the file.

    Description
    Writes character ch into the file. Returns the character added if successful,
    and NlmCmFile__EOF otherwise.

    Parameters
    @para self The pointer to the file instance.
    @param ch  Character to be added the file
    See Also
    NlmCmFile__fgetc
    NlmCmFile__fputs
*/

extern int
NlmCmFile__fputc(
    NlmCmFile* self, 
    int ch);

/*  Summary
    Writes a string to the file.

    Description
    Writes the string str into the file. 
    Returns a non negative value if successful,and NlmCmFile__EOF on error.

    Parameters
    @para self The pointer to the file instance.
    @param str String to be added the file
    See Also
    NlmCmFile__fputc
    NlmCmFile__fgets
*/

extern int
NlmCmFile__fputs(
    NlmCmFile* self, 
    const char* str);

/*  Summary
    Binary output to the file.

    Description
    Emulates standard fwrite behaviour. The function writes nmemb elements  
    of  data,  each  size  bytes long, to the stream pointed to by stream, 
    obtaining them from the location given by ptr. It return the number of 
    items successfully written  (i.e.,  not the number of characters).
    If an error occurs, or the end-of-file is reached, the return value is  
    a  short  item  count  (or zero).

    Parameters
    @para ptr       The pointer to data.
    @param size     Size of each object
    @param nmemb    Number of objects to be written
    @param self     File pointer
    See Also
    NlmCmFile__fprintf
    NlmCmFile__fgets
    NlmCmFile__fread
*/
extern size_t 
NlmCmFile__fwrite( 
    NlmCmFile *self,
    const void *ptr, 
    size_t size, 
    size_t nmemb);


/*  Summary
    A formatted output routine to write to the file.

    Description
    Performs formatted output to the file.This api formats
    and prints a set of characters to the file self. The format
    argument has the same use and syntax as withe ANSI printf
    routine.

    Parameters
    @para self The pointer to the file instance.
    @param format The format string, that is followed by the list of 
		  arguments.
    See Also
    NlmCmFile__printf
    NlmCmFile__vfprintf
*/
extern int
NlmCmFile__fprintf(
    NlmCmFile* self, 
    const char* format,
     ...)
#if defined(__GNUC__) || defined(__GNUG__)
    __attribute__ ((format (printf, 2, 3)))
#endif
    ;

/*  Summary
    A formatted output routine to write to the file.

    Description
    Performs formatted output to the file.This api formats
    and prints a set of characters to the file self. The format
    argument has the same use and syntax as withe ANSI printf
    routine.

    Parameters
    @para self The pointer to the file instance.
    @param format The format string.
    @param argptr  The variable argument list containing all arguments required
		   by the format string.

    See Also
    NlmCmFile__fprintf
*/
extern int
NlmCmFile__vfprintf(
    NlmCmFile* self, 
    const char* format,
    va_list argptr);
/*  Summary
    Reads a character from the file.

    Description
    Reads a character from the current position of the file. 
    Returns the character read if successful,and NlmCmFile__EOF otherwise.

    Parameters
    @para self The pointer to the file instance.
    See Also
    NlmCmFile__fgets
    NlmCmFile__fputc
*/

extern int
NlmCmFile__fgetc(
    NlmCmFile* self);


/*  Summary
    Reads a character from the file.

    Description
    Reads a character from the current position of the file.
    Returns the character read if successful,and NlmCmFile__EOF otherwise.

    Parameters
    @para self The pointer to the file instance.
    See Also
    NlmCmFile__fgetc
    NlmCmFile__fgets
*/

extern int
NlmCmFile__getc(
    NlmCmFile* self);


/*  Summary
    Reads a string from the file.

    Description
    Attempts to read n-1 characters from the file into the buffer.
    It stops reading in characters if it reaches an EOF or newline. The 
    string read, including the newline if present, will be NULL terminated.
    Returns the string read if successful,and NULL on error.

    Parameters
    @para self The pointer to the file instance.
    @param n The maximum number of characters to be read.
    @param buff The buffer where read data is to be stored.

    See Also
    NlmCmFile__fputc
    NlmCmFile__fgets
*/

extern char*
NlmCmFile__fgets(
    NlmCmFile* self, 
    nlm_32 n, 
    char* buff);

/*  Summary
    Binary input from the file.

    Description
    Emulates standard fread behaviour. The function reads nmemb elements  
    of  data,  each  size  bytes long, to the stream pointed to by stream, 
    obtaining them from the location given by ptr. It return the number of 
    items successfully read (i.e.,  not the number of characters).
    If an error occurs, or the end-of-file is reached, the return value is  
    a  short  item  count  (or zero).

    Parameters
    @para ptr       The pointer to data.
    @param size     Size of each object
    @param nmemb    Number of objects to be written
    @param self     File pointer
    See Also
    NlmCmFile__fprintf
    NlmCmFile__fgets
    NlmCmFile__fwrite
*/
extern size_t 
NlmCmFile__fread(
    NlmCmFile *self,
    void *ptr, 
    size_t size, 
    size_t nmemb);

/*  Summary 
    Checks whether End Of File is reached

    Description
    This function returns m_iseof field of the given file.

    Parameters 
    @para self A pointer to the file structure

*/
extern NlmBool 
NlmCmFile__feof(
    NlmCmFile* self) ;

/*  Summary
    Closes a file.

    Description
    Closes the file self and frees the memory used.

    Parameters
    @para self The pointer to the file instance.
    @para nRetCode Used to pass back error status.  
    
    See Also
    NlmCmFile__fopen
*/

extern int 
NlmCmFile__fclose(
    NlmCmFile* self,
    nlm_u32* nRetCode);

/* Console I/O routines */


/*  Summary 
    Reads a line of data from standard input NlmCm__stdin. 
    It continues to read characters until NEWLINE or EOF is seen.

    Description 
    Reads string into buff, the newline character is not placed 
    in the buffer.

    Parameters
    @para buf The pointer to the buffer to place read string

    See Also
    NlmCmFile__fgets
    NlmCmFile__puts

*/
extern char*
NlmCmFile__gets(
    char *buff) ;

/*  Summary 
    Writes a string and a newline character to the standard output NlmCm__stdout. 
    
    Description 
    Writes string in str to the standard output. Returns a non-negetive number 
    on success or NlmCmFile__EOF on error.
    in the buffer.

    Parameters
    @para buff The string to be written onto standard output. 

    See Also
    NlmCmFile__fputs
    NlmCmFile__gets

*/

extern int 
NlmCmFile__puts(
    const char* buff) ;

/*  Summary
    A formatted output routine to output to the console.

    Description
    Performs formatted output to the console.This api formats
    and prints a set of characters to the console. The format
    argument has the same use and syntax as withe ANSI printf
    routine.

    Parameters
    @para self The pointer to the file instance.
    @param format The format string, that is followed by the list of
                  arguments.
    See Also
    NlmCmFile__vprintf
    NlmCmFile__fprintf
*/

extern int
NlmCmFile__printf(
    const char* format, 
    ...)
#if defined(__GNUC__) || defined(__GNUG__)
    __attribute__ ((format (printf,1, 2)))
#endif

    ;

/*  Summary
    A formatted output routine to output to the console.

    Description
    Performs formatted output to the console.This api formats
    and prints a set of characters to the console. The format
    argument has the same use and syntax as withe ANSI printf
    routine.

    Parameters
    @para self The pointer to the file instance.
    @param format The format string.
    @param argptr The variable argument list containing all the
                  arguments required by the format string.
    See Also
    NlmCmFile__printf
*/
int
NlmCmFile__vprintf(
    const char* format,
    va_list argptr);

#ifndef __doxygen__
#define NlmCmFile__getc NlmCmFile__fgetc

/*Dummy FD for use when NLM_HAVE_FILE_IO is not set */
#define NlmCmFile__DUMMYFD 2600

extern void		NlmCmFile__Verify(void);
#endif /* __doxygen__ */

#include <nlmcmexterncend.h>

#endif /*INCLUDED_NLMCMFILE_H*/
#endif
/*[]*/
