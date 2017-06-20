/* $Id: sand_os_interface.h,v 1.8 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
#ifndef DNX_SAND_OS_INTERFACE_H_INCLUDED
#define DNX_SAND_OS_INTERFACE_H_INCLUDED
/* $Id: sand_os_interface.h,v 1.8 Broadcom SDK $
 * {
 */

#ifdef  __cplusplus
extern "C" {
#endif

#include <soc/dnx/legacy/SAND/Utils/sand_framework.h>
#include <soc/drv.h>
/* some os methods accept a timeout parameter,
 * that might indicate (a) infinite timeout (b) no timeout.
 * We use this instead, inorder to be os indepenedent
 */
#define DNX_SAND_INFINITE_TIMEOUT (-1)
#define DNX_SAND_NO_TIMEOUT       0

/* Helper for sal_alloc */
#define sal_dnx_alloc(size,str) sal_alloc(size,str)

/* Helper for sal_rand */
#ifndef __KERNEL__
#include <sal/appl/sal.h>
#else
extern int  dnx_sand_sal_rand_in_kernel_mode(void);
#define sal_rand dnx_sand_sal_rand_in_kernel_mode
#endif
/*
 * Macro "DNX_SAND_PRINTF_ATTRIBUTE":
 * This define is originated from GCC capabilities to check
 * "printf" like function formating.
 * For exact formating refer to GCC docs section "Declaring Attributes of Functions".
 *
 * Define "DNX_SAND_PRINTF_ATTRIBUTE_ENABLE_CHECK"
 * In order to do the checks, define "DNX_SAND_PRINTF_ATTRIBUTE_ENABLE_CHECK"
 * in your compilation system. In GCC add "-DSAND_PRINTF_ATTRIBUTE_ENABLE_CHECK"
 * to the compilation command line.
 */
#ifdef DNX_SAND_PRINTF_ATTRIBUTE_ENABLE_CHECK
  #ifdef __GNUC__
    #define DNX_SAND_PRINTF_ATTRIBUTE(string_index, first_to_check)  \
      __attribute__ ((format (__printf__, string_index, first_to_check)))
  #else
    #error  "Add your system support for 'DNX_SAND_PRINTF_ATTRIBUTE(string_index, first_to_check)'."
  #endif

#elif defined(COMPILER_ATTRIBUTE)
  #define DNX_SAND_PRINTF_ATTRIBUTE(string_index, first_to_check) \
    COMPILER_ATTRIBUTE ((format (__printf__, string_index, first_to_check)))
#else
  #define DNX_SAND_PRINTF_ATTRIBUTE(string_index, first_to_check)
#endif

/*
 *  ANSI C forbids casts from function pointers to data pointers and vice versa, so you can't 
 *  use void * to pass a pointer to a function.
 */
typedef void (*DNX_SAND_FUNC_PTR)(void*);

/*
 * {
 * Sorting and Searching
 */

/*
 * qsort
 */
void
  dnx_sand_os_qsort(
    void         *bot,
    uint32 nmemb,
    uint32 size,
    int (*compar)(void *, void *)
    ) ;
/*
 * bsearch
 */
void
  *dnx_sand_os_bsearch(
    void    *key,
    void    *base0,
    uint32  nmemb,
    uint32  size,
    int (*compar)(void *, void *)
    ) ;

/*
 * }
 */

/*
 * {
 * Task management
 */

/*
 * Lock and unlock task switching.
 */
DNX_SAND_RET
  dnx_sand_os_task_lock(
    void
    ) ;
DNX_SAND_RET
  dnx_sand_os_task_unlock(
    void
    ) ;


/*
 * taskSpawn is vxworks for windows CreateThread() or unix fork()
 */
sal_thread_t
  dnx_sand_os_task_spawn(
    DNX_SAND_IN     char          *name,     /* name of new task (stored at pStackBase) */
    DNX_SAND_IN     long          priority,  /* priority of new task */
    DNX_SAND_IN     long          options,   /* task option word */
    DNX_SAND_IN     long          stackSize, /* size (bytes) of stack needed plus name */
    DNX_SAND_IN     DNX_SAND_FUNC_PTR      entryPt,  /* entry point of new task */
    DNX_SAND_IN     long          arg1,      /* 1st of 10 req'd task args to pass to func */
    DNX_SAND_IN     long          arg2,
    DNX_SAND_IN     long          arg3,
    DNX_SAND_IN     long          arg4,
    DNX_SAND_IN     long          arg5,
    DNX_SAND_IN     long          arg6,
    DNX_SAND_IN     long          arg7,
    DNX_SAND_IN     long          arg8,
    DNX_SAND_IN     long          arg9,
    DNX_SAND_IN     long          arg10
  );
/*
 * delete a task
 */
DNX_SAND_RET
  dnx_sand_os_task_delete(
    DNX_SAND_IN     sal_thread_t          task_id
  );
/*
 * returns the running thread tid
 * should never be zero.
 */
sal_thread_t
  dnx_sand_os_get_current_tid(
    void
  );

/*
 * }
 */

/*
 * {
 * Memory Management
 */

/*
 * malloc
 */
void
  *dnx_sand_os_malloc(
    DNX_SAND_IN     uint32 size,
    char            *str
  );
/*
 * free
 */
DNX_SAND_RET
  dnx_sand_os_free(
    DNX_SAND_INOUT  void          *memblock
  );

/*
 * malloc
 */
void
  *dnx_sand_os_malloc_any_size(
    DNX_SAND_IN     uint32 size,
    char            *str
  );
/*
 * free
 */
DNX_SAND_RET
  dnx_sand_os_free_any_size(
    DNX_SAND_INOUT  void          *memblock
  );
/*
 * Get amount of bytes are currently allocated by dnx_sand_os_malloc()
 */
uint32
  dnx_sand_os_get_total_allocated_memory(
    void
  );


/*
 * copy a buffer from source to destination
 */
DNX_SAND_RET
  dnx_sand_os_memcpy(
    void          *destination, /* destination of copy */
    const void    *source,      /* source of copy */
    uint32 size          /* size of memory to copy (in bytes) */
    );
/*
 * compare 2 buffers
 */
int
  dnx_sand_os_memcmp(
    const     void *s1, /* array 1 */
    const     void *s2, /* array 2 */
    uint32 n   /* size of memory to compare */
  );
/*
 * loads memory with a char
 */
DNX_SAND_RET
  dnx_sand_os_memset(
    void *m,           /* array 1 */
    int   c,           /* array 2 */
    uint32 n   /* size of memory in bytes */
  );

/*
 * }
 */

/*
 * {
 * Sleep and Time
 */

/*
 * suspend a task for mili-second.
 */
DNX_SAND_RET
  dnx_sand_os_task_delay_milisec(
    DNX_SAND_IN     uint32 delay_in_milisec
  );
/*
 * This routine returns the system clock rate.
 * The number of ticks per second of the system clock.
 */
int
  dnx_sand_os_get_ticks_per_sec(
    void
  );
/*
 * most rt OS can suspend a task for several nano seconds
 */
int
  dnx_sand_os_nano_sleep(
    uint32     nano,
    uint32     *premature
  );
/*
 * number of ticks passed since system start up
 */
DNX_SAND_RET
  dnx_sand_os_tick_get(
    DNX_SAND_INOUT     uint32 *ticks
  );
/*
 * number of seconds (and nano seconds, withinn the last second)
 * passed since start up
 */
DNX_SAND_RET
  dnx_sand_os_get_time(
    DNX_SAND_INOUT     uint32 *seconds,
    DNX_SAND_INOUT     uint32 *nano_seconds
  );

/*
 *	Get time difference. 
 *  Assumes start_nano_seconds and end_nano_seconds
 *  are less then 1 second.
 *  Assumes that end_seconds >= start_seconds
 */
DNX_SAND_RET
  dnx_sand_os_get_time_diff(
    DNX_SAND_IN     uint32 start_seconds,
    DNX_SAND_IN     uint32 start_nano_seconds,
    DNX_SAND_IN     uint32 end_seconds,
    DNX_SAND_IN     uint32 end_nano_seconds,
    DNX_SAND_OUT    uint32 *diff_seconds,
    DNX_SAND_OUT    uint32 *diff_nano_seconds
  );
uint32
  dnx_sand_os_get_time_micro(
    void
  );

/*
 * returns the number of system ticks in 1 second
 */
uint32
  dnx_sand_os_sys_clk_rate_get(
    void
  );


/*
 * }
 */


/*
 * {
 * Semaphores
 */

/*
 * returns the sem_id to be used by the other procs
 */
sal_mutex_t
  dnx_sand_os_mutex_create(
    void
  );
/*
 * deletes a mutex
 */
DNX_SAND_RET
  dnx_sand_os_mutex_delete(
    DNX_SAND_IN     sal_mutex_t          sem_id
  );
/*
 * timeout might be DNX_SAND_INFINITE_TIMEOUT
 */
DNX_SAND_RET
  dnx_sand_os_mutex_take(
    DNX_SAND_IN     sal_mutex_t          sem_id,
    DNX_SAND_IN     long          timeout
  );
/*
 * release a mutex
 */
DNX_SAND_RET
  dnx_sand_os_mutex_give(
    DNX_SAND_IN     sal_mutex_t          sem_id
  );

/*
 * }
 */

/*
 * {
 * Level control
 */

/*
 * disable interrupts
 */
DNX_SAND_RET
  dnx_sand_os_stop_interrupts(
    uint32 *flags
  );
/*
 * resotore interrupts
 */
DNX_SAND_RET
  dnx_sand_os_start_interrupts(
    uint32 flags
  );

/*
 * }
 */

/*
 * {
 * Message queues
 */

/*
 * returns MSG_Q_ID used by the other procs
 */
void
  *dnx_sand_os_msg_q_create(
    int max_msgs,
    int max_msglength
    ) ;
/*
 * deletes a message queue
 */
DNX_SAND_RET
  dnx_sand_os_msg_q_delete(
    void *msg_q_id
    );
/*
 * sends a message to a message queue
 */
DNX_SAND_RET
  dnx_sand_os_msg_q_send(
    void         *msg_q_id,
    char         *data,
    uint32 data_size,
    int          timeout,
    int          priority
  ) ;
/*
 * return = 0 => got a message
 * return > 0 => time out
 * return < 0 => error
 * timeout might be DNX_SAND_INFINITE_TIMEOUT
 */
int
  dnx_sand_os_msg_q_recv(
    void          *msg_q_id,
    unsigned char *data,
    uint32  max_nbytes,
    long          timeout_in_ticks
    ) ;
int
  dnx_sand_os_msg_q_num_msgs(
    void          *msg_q_id
    ) ;

/*
 * }
 */

/*
 * {
 * STDIO and String - only for error/debug/printing
 */


/*
 * determine the length of a string (ANSI)
 */
uint32
  dnx_sand_os_strlen(
    const char* s /* string */
  );

/*
 * compare N chars
 */
int
  dnx_sand_os_strncmp(
    const char *string1,
    const char *string2,
    uint32 count
  );

/*
 * copy a string from source to destination
 */
DNX_SAND_RET
  dnx_sand_os_strncpy(
    char          *s1, /* string to copy to */
    const char    *s2, /* string to copy from */
    uint32 n   /* max no. of characters to copy */
  );

/* {  Random function. */
/*
 * The srand function sets the starting point for generating
 *  a series of pseudorandom integers
 */
void
  dnx_sand_os_srand(
    uint32
  );
/*
 * Generates a pseudo-random integer between 0 and RAND_MAX
 */
uint32
  dnx_sand_os_rand(
    void
  );
/* }  END Random function. */
/* {  IO printing functions. */
/*
 * printf
 */
int
  dnx_sand_os_printf(
    DNX_SAND_IN char* format, ...
  ) DNX_SAND_PRINTF_ATTRIBUTE(1, 2);
/* } END IO printing functions. */

/*
 * }
 */


#ifdef DNX_SAND_DEBUG
/* { */

/*
 * Report on the taken resources.
 */
DNX_SAND_RET
  dnx_sand_os_resource_print(
    void
  );

/* } */
#endif


#ifdef  __cplusplus
}
#endif


/*
 * }
 */
#endif

