/* $Id: snprintf.c,v 1.3 2011/07/21 16:14:25 yshtil Exp $ */
#include "vxWorks.h"
#include "ctype.h"
#include "errno.h"
#include "ioLib.h"
#include "string.h"
#include "stdarg.h"
#include "limits.h"
#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"
#include "fioLib.h"

/* typedefs */

typedef struct snputbuf_arg     /* used by snprintf(), snputbuf() snprintf() */
    { 
    char *pBuf;                 /* running pointer to the next char */
    char *pBufEnd;              /* pointer to buffer end */
    } SNPUTBUF_ARG;
		
/*******************************************************************************
*
* fioSnBufPut - put characters in a buffer (safe version of fioBufPut)
*
* This is a support routine for snprintf().  This routine copies <length>
* bytes from source to destination, leaving the destination buffer pointer
* (pArg->pBuf) pointing at byte following block copied. 
*
* If <length> exceeds the number of bytes available in the buffer, only the
* number of bytes that fit within the buffer are copied.  In this case
* OK is still returned (although no further copying will be performed) so
* that snprintf() can return the number of characters that would have been
* copied if the supplied buffer was of sufficient size.
*
* RETURNS: OK always
*
* \NOMANUAL
*/

STATUS fioSnBufPut
    (
    char *         pInBuf,       /* pointer to input buffer */
    int            length,       /* length of input buffer */
    SNPUTBUF_ARG * pArg          /* fioSnBufPut argument structure */
    )
    {
    int remaining;

    /* check if sufficient free space remains in the buffer */
    
    remaining = pArg->pBufEnd - pArg->pBuf - 1;

    /* bail if at the end of buffer, recall need a single byte for null */

    if (remaining <= 0)
	return (OK);
    else if (length > remaining)
	length = remaining;

    bcopy (pInBuf, pArg->pBuf, length);

    pArg->pBuf += length;

    return (OK);
    }

/*******************************************************************************
*
* snprintf - write a formatted string to a buffer, not exceeding buffer size (ANSI)
*
* This routine copies a formatted string to a specified buffer, up to a given 
* number of characters.  The formatted string will be null terminated.  This 
* routine guarantees never to write beyond the provided buffer regardless of
* the format specifier or the arguments to be formatted.  The <count> 
* argument specifies the maximum number of characters to store in the buffer,
* including the null terminator.
*
* Its function and syntax are otherwise identical to printf().
*
* RETURNS: The number of characters copied to <buffer>, not including the
* NULL terminator.  Even when the supplied <buffer> is too small to
* hold the complete formatted string, the return value represents
* the number of characters that would have been written to <buffer>
* if <count> was sufficiently large.
*
* SEE ALSO: sprintf(), printf(),
* \tb "International Organization for Standardization, ISO/IEC 9899:1999,"
* \tb "Programming languages - C: Input/output (stdio.h)"
*
* VARARGS2
*/

int snprintf
    (
    char *       buffer, /* buffer to write to */
    size_t       count,  /* max number of characters to store in buffer */
    const char * fmt,    /* format string */
    ...                  /* optional arguments to format */
    )
    {
    va_list	  vaList;	 /* traverses the argument list */
    int		  nChars;
    SNPUTBUF_ARG  snputbufArg;

    snputbufArg.pBuf 	= buffer;
    snputbufArg.pBufEnd	= (char *)(buffer + count);

    va_start (vaList, fmt);
    nChars = fioFormatV (fmt, vaList, fioSnBufPut, (int) &snputbufArg);
    va_end (vaList);

    if (count != 0)
	*snputbufArg.pBuf = EOS; /* null-terminate the string */

    return (nChars);
    }

/*******************************************************************************
*
* vsnprintf - write a string formatted with a variable argument list to a buffer, not exceeding buffer size (ANSI)
*
* This routine copies a string formatted with a variable argument list to a 
* specified buffer, up to a given  number of characters.  The formatted string
* will be null terminated.  This routine guarantees never to write beyond the
* provided buffer regardless of the format specifier or the arguments to be
* formatted.  The <count> argument specifies the maximum number of characters
* to store in the buffer, including the null terminator.
*
* This routine is identical to snprintf(), except that it takes the variable
* arguments to be formatted as a list <vaList> of type `va_list' rather than
* as in-line arguments.
*
* RETURNS
* The number of characters copied to <buffer>, not including the
* NULL terminator.  
*
* Even when the supplied <buffer> is too small to
* hold the complete formatted string, the return value represents
* the number of characters that would have been written to <buffer>
* if <count> was sufficiently large.
*
* SEE ALSO: sprintf(), printf(),
* \tb "International Organization for Standardization, ISO/IEC 9899:1999, Programming languages - C: Input/output (stdio.h)"
*
* VARARGS2
*/

int vsnprintf
    (
    char *	 buffer,  /* buffer to write to */
    size_t 	 count,	  /* max number of characters to store in buffer */
    const char * fmt,	  /* format string */
    va_list	 vaList	  /* optional arguments to format */
    )
    {
    int		  nChars;
    SNPUTBUF_ARG  snputbufArg;

    snputbufArg.pBuf 	= buffer;
    snputbufArg.pBufEnd	= (char *)(buffer + count);

    nChars = fioFormatV (fmt, vaList, fioSnBufPut, (int) &snputbufArg);

    if (count != 0)
	*snputbufArg.pBuf = EOS; /* null-terminate the string */

    return (nChars);
    }


