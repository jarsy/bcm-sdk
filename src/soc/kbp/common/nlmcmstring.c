/*
 * $Id: nlmcmstring.c,v 1.2.8.1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


 #include <nlmcmctype.h>
#include <nlmcmstring.h>

/* NlmCmString
 * 
 * Portable versions of some of the common string comparison and manipulation
 * routines found in string.h / strings.h
 */

/* Portable versions of strlwr and strupr */
char*
NlmCm__strlwr(
    char* str)
{
    char* cur = str ;

    if( str == NULL ) return NULL ;

    while( cur[0] != '\0' )
    {
    /* NB: NlmCm__tolower returns int in VC++ */
    cur[0] = (char)NlmCm__tolower(cur[0]) ;
    cur++ ;
    }

    return str ;
}

 
char*
NlmCm__strupr(
    char* str)
{
    char* cur = str ;

    if( str == NULL ) return NULL ;

    while( cur[0] != '\0' )
    {
    /* NB: NlmCm__toupper returns int in VC++ */
    cur[0] = (char)NlmCm__toupper(cur[0]) ;
    cur++ ;
    }

    return str ;
}

/* Portable versions of strcasecmp and strncasecmp */
int
NlmCm__strcasecmp(
    const char* str1, 
    const char* str2)
{
    int idx = 0 ;

    /* N.B., glibc strcasecmp segfaults on NULL arguments. The goal is to
     * emulate its behavior as close as possible, so we don't do anything
     * special but provide an assert to make finding bugs easier.
     */
    
    NlmCmAssert(str1 && str2, "Have non-null arguments");

    while( str1[idx] != '\0' && str2[idx] != '\0' )
    {
    if( NlmCm__tolower(str1[idx]) < NlmCm__tolower(str2[idx]) ) return -1 ;
    if( NlmCm__tolower(str1[idx]) > NlmCm__tolower(str2[idx]) ) return 1 ;
    idx++ ;
    }

    if( str1[idx] == '\0' && str2[idx] != '\0' ) return -(NlmCm__tolower(str2[idx])) ;
    if( str1[idx] != '\0' && str2[idx] == '\0' ) return   NlmCm__tolower(str1[idx]) ;

    return 0 ;
}

int
NlmCm__strncasecmp(
    const char* str1, 
    const char* str2,
    size_t n)
{
    size_t idx = 0 ;

    /* N.B., glibc strcasecmp segfaults on NULL arguments. The goal is to
     * emulate its behavior as close as possible, so we don't do anything
     * special but provide an assert to make finding bugs easier.
     */

    NlmCmAssert(str1 && str2, "Have non-null arguments");
    
    while( (idx < n) && (str1[idx] != '\0') && (str2[idx] != '\0') )
    {
    if( NlmCm__toupper(str1[idx]) < NlmCm__toupper(str2[idx]) ) return -1 ;
    if( NlmCm__toupper(str1[idx]) > NlmCm__toupper(str2[idx]) ) return 1 ;
    idx++ ;
    }

    if( idx == n ) return 0 ;
    if( str1[idx] == '\0' && str2[idx] != '\0' ) return -(NlmCm__tolower(str2[idx])) ;
    if( str1[idx] != '\0' && str2[idx] == '\0' ) return   NlmCm__tolower(str1[idx]) ;

    return 0 ;
}

/* Portable, allocator aware strdup. Returns NULL on error. */
char*
NlmCm__strdup(
    NlmCmAllocator* alloc,
    const char*    str)
{
    char* cpy ;
    int len ;

    NlmCmAssert(alloc && str, "Valid allocator and input string");

    if( alloc == NULL || str == NULL )
        return NULL ;

    len = (nlm_32)NlmCm__strlen(str) + 1 ;
    cpy = (char*) NlmCmAllocator__malloc(alloc, sizeof(char) * len) ;
    if( cpy == NULL ) return NULL ;

    NlmCm__memcpy(cpy, str, len) ;

    return cpy ;
}

nlm_u32
NlmCm__strhash(
    const char *str)
{
    /* PJW hash (Aho, Sethi, and Ullman pp. 434-438) */
    nlm_u32 h = 0;
    nlm_u32 g = 0;
    nlm_u32 ki = 0;
    nlm_u32 mask = 0xf0000000;

    for( ; *str ; str++ ) {
    ki = *str;
    h = (h << 4) + ki;
    g = h & mask;
    if (g != 0)
    {
        h = h ^ (g >> 24);
        h = h ^ g;                    
    }
    }

    return h;
}

NlmBool
NlmCm__atoi(
    const char* str,
    nlm_32*    o_num)
{
    const char *ptr;
    NlmBool negValue, retValue = NlmTrue;
    nlm_32 charValue;
    nlm_32  highest, highest_add ;
    nlm_32 temp;
    nlm_32 a;
    const nlm_32 base = 10 ; 

    NlmCmAssert(str != NULL, "Null string passed.");

    
    /*
     * Skip white space 
     */
    ptr = str;
    
    
    charValue = (unsigned char) *ptr++;
    
    while (NlmCm__isspace(charValue))
        charValue = (unsigned char) *ptr++;

    negValue = NlmFalse;
    if (charValue == '-') 
    {
        charValue = *ptr++;
        negValue = NlmTRUE;
    } 
    
    if (charValue == '+') 
            charValue = *ptr++;
    

    if(negValue)
    {
        highest = INT32_MIN; 
    }
    else
    {
        highest = INT32_MAX;
    }
       
    highest_add = highest % base;
    highest /= base;

    if (negValue) 
    {
        if (highest_add > 0) 
        {
            highest_add -= base;
            highest += 1;
        }
        highest_add = -highest_add;
    }

    for (a = 0, temp = 0;; charValue = (unsigned char) *ptr++) 
    {
        if (NlmCm__isdigit(charValue))
            charValue -= '0';
        else
            break;

        if (temp < 0)
            continue;
    
        if (negValue) 
        {
            if ( (a == highest && charValue > highest_add) || a < highest ) 
            {
                a = INT32_MIN;
                retValue = NlmFALSE;
                temp = -1;
            } 
            else 
            {
                a *= base;
                a -= charValue;
                temp = 1;
            }
        } 
        else 
        {
            if ((a == highest && charValue > highest_add) || a > highest) 
            {
                a = INT32_MAX;
                retValue = NlmFALSE;
                temp = -1;
            } 
            else 
            {
                a *= base;
                a += charValue;
                temp = 1;
            }
        }
    }

    *o_num = a ;

    return retValue;
}


/*
 * Compare strings.
 */
int
NlmCm__strcmp(
    const char* s1, 
    const char* s2)
{
    NlmCmAssert(s1 != NULL, "valid string");
    NlmCmAssert(s2 != NULL, "valid string");
    
    while (*s1 == *s2++)
        if (*s1++ == '\0')
            return 0;

    return *s1 - *(s2 - 1);
}

int
NlmCm__strncmp(
    const char* s1,
    const char* s2, 
    size_t      n)
{
    NlmCmAssert(s1 != NULL, "Invalid string");
    NlmCmAssert(s2 != NULL, "Invalid string");
    
    if (n == 0)
        return (0);

    while (n-- > 0) {
        if (*s1 != *s2++)
            return *s1 - *(s2 - 1);
        if (*s1++ == '\0')
            break;
    }

    return 0;
}


char*
NlmCm__strchr(
    const char* p, 
    int         ch)
{
    char *tmp = (char *)p;
    NlmCmAssert(p != NULL, "valid string");
        
    while (*tmp) {
      if (*tmp == ch) return tmp;
      tmp++;
      if (!*tmp && (*tmp == ch)) 
              return tmp;
    }
    return NULL;
}


/*
 * Find the first occurrence of find in s.
 */
char*
NlmCm__strstr(
    const char* s, 
    const char* find)
{
    char c, sc;
    size_t len;

    NlmCmAssert(s != NULL, "Not valid string");
    NlmCmAssert(find != NULL, "Not valid string");

    if (!*find) return (char *)s;

    c = *find++;
    
    len = NlmCm__strlen(find);
    do {
        do {
            if ((sc = *s++) == '\0') {
                /* Did not find 'find' in 's' */
                return NULL;
            }
        } while (sc != c); /* while we haven't found the current search char */

        /* Found the search char, so strncmp to see if find is a prefix of
           the remainder of s.  */

    } while (NlmCm__strncmp(s, find, len) != 0);

    return (char*) s - 1;
}


#ifdef _MSC_VER
#pragma warning(default: 4146)
#endif

size_t
NlmCm__strlen(
    const char* string)
{
    const char *s = string;

    NlmCmAssert(string != NULL, "valid string");

    /*Go to the end of the string */
    while (*s) s++;

    return(s - string);
}

/* Copy src to dst. Returns dst. */

char*
NlmCm__strcpy(
    char*       dst, 
    const char* src)
{
    char* ptr = dst;

    NlmCmAssert(dst != NULL, "valid dst string");
    NlmCmAssert(src != NULL, "valid src string");

    while((*dst++ = *src++) != '\0');

    return ptr;
}

/*
 * Copy src to dst, truncating or null-padding to always copy n bytes.
 * Return dst.
 */

char*
NlmCm__strncpy(
    char*       to, 
    const char* from, 
    size_t      n)
{
    char       *d = to;
    const char *s = from;

    NlmCmAssert(to != NULL, "valid to string");
    NlmCmAssert(from != NULL, "valid from string");

    if(n == 0) return to;

    do {
        if ((*d++ = *s++) == '\0') {
            /* NULL-pad the remaining n-1 bytes */
            while (--n != 0)
                *d++ = 0;
            return to;
        }
    } while (--n != 0);
    
    *d = '\0';
    return to;
}

/* Appends the string 'append' to the end of s for specified number of chars.
   Returns s. */
char*
NlmCm__strncat(
    char*       to,
    const char* append,
    size_t count)
{
    char * tmp = to;

    NlmCmAssert(tmp != NULL, "Not valid string");
    NlmCmAssert(append != NULL, "Not valid string");

    if(!count)
        return to;
    
    /* Go to the end of first string */
    while (*tmp) tmp++;
    while(count--)
    {
        int result = (*tmp++ = *append++);
        if (!(result))
            return(to);
        *tmp = '\0';
    }

    return to;
}

/* Appends the string 'append' to the end of s.  Returns s. */
char*
NlmCm__strcat(
    char*       to,
    const char* append)
{
    char * tmp = to;

    NlmCmAssert(tmp != NULL, "Not valid string");
    NlmCmAssert(append != NULL, "Not valid string");
    
    /* Go to the end of first string */
    while (*tmp) tmp++;
    while ((*tmp++ = *append++) != '\0') ;
    return to;
}


/*[]*/
