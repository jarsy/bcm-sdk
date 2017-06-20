/* $Id: ui_module.c,v 1.13 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/*
 * Basic_include_file.
 */

/*
 * Tasks information.
 */
#include <appl/diag/dpp/tasks_info.h>
#if !DUNE_BCM
#include <appl/diag/dpp/utils_version.h>
#else
#include <appl/diag/dpp/utils_defx.h>
#define BOOL uint8
#endif
#include <appl/diag/dpp/utils_memory.h>
#include <soc/dpp/SAND/Utils/sand_os_interface.h>
#include <appl/dpp/UserInterface/ui_pure_defi.h>
#include <sal/appl/io.h>

#define UI_NUMERIC_STR_SIZE   64
/*
 * INCLUDE FILES:
 */

#ifdef WIN32
  #include <bcm_app/dpp/../H/usrApp.h>
#elif defined(__DUNE_HRP__)
  #include <DHRP/dhrp_defs.h>
#endif

#ifdef LINK_TIMNA_LIBRARIES
  #include <appl/diag/dpp/utils_line_TEVB.h>
#endif

#if !DUNE_BCM
#ifdef __VXWORKS__
/* { */
  #include <vxWorks.h>
  #include <taskLib.h>
  #include <inetLib.h>

  #include <shellLib.h>
  #include <dbgLib.h>
  #include <end.h>

  #include <appl/diag/dpp/dune_rpc.h>

  #include <usrApp.h>

  #include <appl/diag/dpp/utils_recovery_msg.h>
  #include <appl/diag/dpp/utils_fap10m.h>
  #include <appl/diag/dpp/utils_starter_script.h>
  #include <appl/diag/dpp/utils_aux_input.h>
  #include <appl/diag/dpp/utils_string.h>
  #include <appl/diag/dpp/utils_line_TEVB.h>

/* } */
#else
/* { */
  #define FIONREAD 0 
  #define FIOFLUSH 0 

  #include <malloc.h>
  #include <ctype.h>
  #include <stdlib.h>
  #include <appl/diag/dpp/ref_sys.h>
  #include <appl/diag/dpp/utils_string.h>
  #include <appl/diag/dpp/utils_starter_script.h>
  #include <soc/dpp/SAND/Utils/sand_os_interface.h>
  #include <sal/appl/io.h>
void
  set_recovery_msg_off(){return;}
int
  is_recovery_msg_on(){return FALSE;}
int
  get_run_cli_script_at_stratup(
    unsigned char *val_ptr
  )
{
  *val_ptr =0;
  return 0;
}

/* } */
#endif
#endif

#include <appl/diag/dcmn/utils_board_general.h>
#include <appl/diag/dpp/utils_string.h>
#include <appl/dpp/UserInterface/ui_defi.h>
#include <appl/dpp/UserInterface/ui_pure_defi.h>
#include <appl/dpp/UserInterface/ui_cli_files.h>
#if !DUNE_BCM
#else
#ifdef VXWORKS
  #include "vxWorks.h"
  #include "stdlib.h"
  #include "stdio.h"
  #include "inetLib.h"
  #include "sockLib.h"
  #include "pipeDrv.h"
  #if !defined(__DUNE_GTO_BCM_CPU__)
  #include "usrApp.h"
  #endif
  extern int sysClkRateGet(void);
  #define D_TICKS_PER_SEC sysClkRateGet()
char
  *get_addr_string(void);
#else
  #define D_TICKS_PER_SEC sysconf(_SC_CLK_TCK)
#endif
void
  set_recovery_msg_off(){return;}
int
  is_recovery_msg_on(){return FALSE;}

timer_t my_timer_id=0;
void
  set_ui_timer_id(timer_t id){my_timer_id = id;}
timer_t
  get_ui_timer_id(void){return my_timer_id;}

/* #endif */


  /* flag indicating that UI task should continue gettich characters */
  static BOOL S_ui_proc_continue = FALSE;
  /* flag indicating that UI task should continue gettich characters */
  static BOOL S_ui_proc_finished = TRUE;
  /* task id of UI task */
  static int S_ui_tid = -1;
  
  int get_ui_tid() { return S_ui_tid; }
#endif

#if !DUNE_BCM
#ifdef SAND_LOW_LEVEL_SIMULATION
  #if !(defined(LINUX) || defined(UNIX))
		#include <io.h>
    #include <conio.h>
    STATUS
		  ioctl(
		    int fd,
		    int fio,
		    int read_count
		  );

#if !(defined(LINUX) || defined(UNIX))
#endif
#endif
#else
  static
    unsigned
    int
    Periodic_display_flag ;
  void
  set_periodic_off(){
    return;
  }
  int
  is_display_periodic_on(
    void
    )
  {
    return (Periodic_display_flag) ;
  }
  void
  set_periodic_on(
    void
  )
  {
      Periodic_display_flag = 1 ;
    return ;
  }

#endif

STATUS
  ioctl(
    int fd,
    int fio,
    int read_count
  );
/* } SAND_LOW_LEVEL_SIMULATION */
#endif

#ifdef __DUNE_HRP__
  #include <DHRP/dhrp_gfa.h>
#endif

#if (defined(LINUX) || defined(UNIX))
/*{*/
#if !DUNE_BCM
  #include <appl/diag/dpp/utils_aux_input.h>
#endif
  #include <sys/types.h>
  #include <sys/uio.h>
  #include <unistd.h>
  #include <stdio.h>
  #include <stdlib.h>
  #include <termios.h>
  #define min(a,b)    (((a) < (b)) ? (a) : (b))
  
/*} LINUX */
#endif

#ifdef __DUNE_SSF__
/* { */
#include <appl/diag/dpp/utils_char_queue.h>
#define UI_CHAR_BUFF_SIZE (1024)

static
  CHAR_QUEUE
    Ui_char_queue;
static int
  Ui_char_queue_is_init = 0;
/* } */
#endif

#if !DUNE_BCM
#else
#include <appl/diag/dpp/utils_char_queue.h>
#endif

/*
 * Static variables for ui interpreter package.
 * {
 */
/*
 * }
 */

static
  int Activating_file = FALSE;

static
  unsigned int
    Ui_do_long_error_printing = FALSE;

uint32
  Default_unit = 0;

#if !DUNE_BCM
#else
#if !defined(__VXWORKS__)
/* { */
int d_getch() {
  int ch;

#ifdef WIN32
/* { */
  ch = _getch();
/* } */
#elif (defined(LINUX) || defined(UNIX))
/* { */
  struct termios oldt,
                 newt;
  tcgetattr( STDIN_FILENO, &oldt );
  newt = oldt;
  newt.c_lflag &= ~( ICANON | ECHO );
  tcsetattr( STDIN_FILENO, TCSANOW, &newt );
  ch = getchar();
  tcsetattr( STDIN_FILENO, TCSANOW, &oldt );
/* } */
#endif
  return ch;
}
/* } */
#else
int d_getch() {
  int ch; 
  
  ch = getchar();
  /*ch = getc(stdin);*/
  return ch;
}
#endif
#endif
static
  unsigned
  int
  Periodic_display_flag ;

int
is_display_periodic_on(
  void
  )
{
  return (Periodic_display_flag) ;
}

void set_display_periodic_off(void)
{
}


void
  set_ui_do_long_error_printing(
    unsigned int do_long_error_printing
  )
{
  Ui_do_long_error_printing = do_long_error_printing;
  return;
}

unsigned int
  get_ui_do_long_error_printing(
    void
  )
{
  return Ui_do_long_error_printing;
}

/*
 * OBJECT:
 *   handling of inactivity timeout for Telnet.
 * {
 */
/*
 * Telnet activity timeout, in units of seconds.
 */
static
  unsigned
    long
      Telnet_activity_timeout ;
/*
 * Last time activity was detected on telnet. Time is
 * measured from startup in units of 10 milliseconds.
 */
#if !DUNE_BCM
#else
#if defined(__VXWORKS__) && !defined(__DUNE_GTO_BCM_CPU__)
static
  unsigned
    long
      Last_telnet_activity_time_mark = 0 ;
#endif
#endif
static
  int
    Telnet_status = BOTH_DEAD ;
unsigned
  int
    is_telnet_active(
      void
    )
{
  unsigned
    int
      ret ;
  if (Telnet_status == BOTH_ALIVE)
  {
    ret = TRUE ;
  }
  else
  {
    ret = FALSE ;
  }
  return (ret) ;
}
void
  set_telnet_active(
    unsigned int val
  )
{
  if (val)
  {
    Telnet_status = BOTH_ALIVE ;
  }
  else
  {
    Telnet_status = BOTH_DEAD ;
  }
  return ;
}
unsigned
  long
    get_telnet_timeout_val(
      void
    )
{
  return (Telnet_activity_timeout) ;
}
void
  set_telnet_timeout_val(
    unsigned long val
  )
{
  /*
   * Set inactivity timeout value, in seconds.
   */
  Telnet_activity_timeout = val ;
  return ;
}
void
  set_telnet_activity_time_mark(
    void
  )
{
#if !DUNE_BCM
#else
#if defined(__VXWORKS__) && !defined(__DUNE_GTO_BCM_CPU__)
  if (is_telnet_active())
  {
    Last_telnet_activity_time_mark = get_watchdog_time(TRUE) ;
  }
#endif
#endif
  return ;
}
unsigned
  int
    has_telnet_inactivity_time_elapsed(
      void
  )
{
  unsigned
    int
      ret = 0;
  
#if !DUNE_BCM
#else
#if defined(__VXWORKS__) && !defined(__DUNE_GTO_BCM_CPU__)
    unsigned
            long
              telnet_timeout_val,
              current_time ;
          char
            char_telnet_timeout_val ;
  ret = FALSE ;
  if (is_telnet_active())
  {
    telnet_timeout_val = get_telnet_timeout_val() ;
    /*
     * A value of '-1' for timeout period indicates 'no timeout'
     */
    char_telnet_timeout_val = (char)telnet_timeout_val ;
    if (char_telnet_timeout_val != (char)(-1))
    {
      current_time = get_watchdog_time(TRUE) ;
      if ((current_time - Last_telnet_activity_time_mark) >
                                        (telnet_timeout_val * 100))
      {
        ret = TRUE ;
      }
    }
  }
#endif
#endif
  return (ret) ;
}
/*
 * }
 */
/*
 * OBJECT:
 *   Timeout on suspended ui task.
 * {
 */
#if LINK_PSS_LIBRARIES
/* { */
/*
 * When DPSS is included then ui task may occasionally
 * be suspended for short times because of 'blocked'
 * operations.
 */
static
  unsigned
    long
      Ui_task_suspend_timeout = MAX_SUSPENDED_BECAUSE_BLOCKED ;
/* } */
#else
/* { */
static
  unsigned
    long
      Ui_task_suspend_timeout = 0 ;
/* } */
#endif
/*
 * This is a counter of the number of contiguous times
 * ui task has been found suspended. Currently only used
 * in the 'LINK_PSS_LIBRARIES' case.
 */
static
  unsigned long
    Num_suspended = 0 ;
void
  inc_ui_suspend_timer(
    void
  )
{
  Num_suspended++ ;
}
void
  clear_ui_suspend_timer(
    void
  )
{
  Num_suspended = 0 ;
}
int
  is_ui_suspend_timer_overflown(
    void
  )
{
  int
    ret ;
  if (Num_suspended > Ui_task_suspend_timeout)
  {
    ret = TRUE ;
  }
  else
  {
    ret = FALSE ;
  }
  return (ret) ;
}
/*
 * }
 */
#if defined(__VXWORKS__) && !defined(__DUNE_GTO_BCM_CPU__)
/* { */
/*****************************************************
*$NAME$
*  ui_proc_signal
*TYPE: PROC
*DATE: 23/DEC/2001
*FUNCTION:
*  Function to be activated when a signal is sent to
*  user interface task (ui_proc).
*CALLING SEQUENCE:
*  ui_proc_signal(signal_id)
*INPUT:
*  SOC_SAND_DIRECT:
*    signal_id -
*      Int. Id of signal responsible for the activation.
*  SOC_SAND_INDIRECT:
*    Current_ui_fd
*OUTPUT:
*  SOC_SAND_DIRECT:
*    None.
*  SOC_SAND_INDIRECT:
*    Restarted task, if standard I/O file descriptor
*    has changed.
*REMARKS:
*  NONE.
*SEE ALSO:
*****************************************************/
void
  ui_proc_signal(
    int signal_id
  )
{
  static
    int
      current_fd,
      actual_fd ;
  static
    unsigned
      int
        trigger_delay_dead,
        trigger_delay_alive ;
  STATUS
    status ;
  char
    *proc_name ;
  timer_t
    timer_id ;
  static
    unsigned int
      telnet_in_task_id,
      telnet_in_alive ;
  static
    unsigned int
      telnet_out_task_id,
      telnet_out_alive ;
  static
    unsigned
      long
        telnet_timeout ;
  unsigned
    int
      telnet_timeout_elapsed,
      ui_suspended ;
  int
    ui_task_id ;
#if TELNET_DEBUG_PRINT
  static
    unsigned int
      signal_count ;
  static
    char
      *telnet_in_msg ;
  static
    char
      *telnet_out_msg ;
#endif
  proc_name = "ui_proc_signal" ;
#if TELNET_DEBUG_PRINT
  signal_count++ ;
#endif
  telnet_in_task_id = taskNameToId("tTelnetInTask") ;
  if (telnet_in_task_id != ERROR)
  {
#if TELNET_DEBUG_PRINT
    telnet_in_msg = "Telnet IN is alive!" ;
#endif
    telnet_in_alive = TRUE ;
  }
  else
  {
#if TELNET_DEBUG_PRINT
    telnet_in_msg = "Telnet IN is dead!" ;
#endif
    telnet_in_alive = FALSE ;
  }
  telnet_out_task_id = taskNameToId("tTelnetOutTask") ;
  if (telnet_out_task_id != ERROR)
  {
#if TELNET_DEBUG_PRINT
    telnet_out_msg = "Telnet OUT is alive!\n" ;
#endif
    telnet_out_alive = TRUE ;
  }
  else
  {
#if TELNET_DEBUG_PRINT
    telnet_out_msg = "Telnet OUT is dead!\n" ;
#endif
    telnet_out_alive = FALSE ;
  }
  if (!telnet_out_alive && !telnet_in_alive)
  {
    if (is_telnet_active() && (trigger_delay_dead == 0))
    {
      /*
       * Enter if status of telnet is not already BOTH_DEAD
       * (No point in doing it over and over again).
       *
       * If both telnet tasks are dead then set the global
       * standard IO back to original. This is necessary since,
       * sometimes, on unsolicted disconnect, the global IO
       * is not updated.
       */
      ioGlobalStdSet(STD_IN,get_startup_io_file_descriptor()) ;
      ioGlobalStdSet(STD_OUT,get_startup_io_file_descriptor()) ;
      ioGlobalStdSet(STD_ERR,get_startup_io_file_descriptor()) ;
      set_telnet_active(FALSE) ;
      trigger_delay_dead = MAX_TRIGGER_DELAY ;
    }
    else
    {
      /*
       * Either trigger delay is not completed or telnet status
       * has already been updated to BOTH_DEAD.
       */
      if (trigger_delay_dead > MAX_TRIGGER_DELAY)
      {
        /*
         * Just protection against error.
         */
        trigger_delay_dead = MAX_TRIGGER_DELAY ;
      }
      if ((trigger_delay_dead > 0) && is_telnet_active())
      {
        /*
         * decrement delay counter if file descriptor switch
         * has not been carried out yet.
         */
        trigger_delay_dead-- ;
      }
    }
  }
  else if (telnet_out_alive && telnet_in_alive)
  {
    if (!is_telnet_active() && (trigger_delay_alive == 0))
    {
      /*
       * Enter if status of telnet is not BOTH_ALIVE
       * already. No point in doing it over and over again.
       *
       * If both telnet tasks are alive then set the global
       * standard IO to the file descriptor related to the
       * "/pty/telnet. S" device. This is necessary since,
       * sometimes, on unsolicted disconnect, the global IO
       * is not updated.
       */
      DEV_HDR
        *dev_hdr_p ;
      char
        *dev_name,
        *name_tail ;
      set_telnet_active(TRUE) ;
      set_telnet_activity_time_mark() ;
#if TELNET_DEBUG_PRINT
      logMsg("TELNET alive...\n",
          (int)0,(int)0,0,0,0,0) ;
#endif
      dev_name = "/pty/telnet.M" ;
#if NEGEV
      dev_hdr_p = iosDevFind(dev_name,&name_tail) ;
#else
      name_tail = NULL;
      dev_hdr_p = NULL;
#endif
      if (dev_name != name_tail)
      {
        /*
         * Enter if device has been found.
         * Now find the corresponding file descriptor.
         * Check up to 30 file descriptors (3 -> 32)
         */
        static
          int
            device_fd ;
        DEV_HDR
          *dev_specific ;
#if TELNET_DEBUG_PRINT
        logMsg("Device has been found\n",
          (int)0,(int)0,0,0,0,0) ;
#endif
        for (device_fd = 3 ; device_fd < 32 ; device_fd++)
        {
#if NEGEV
          dev_specific = (DEV_HDR *)iosFdValue(device_fd) ;
#else
          dev_specific = NULL;
#endif
          if ((int)dev_specific != ERROR)
          {
            /*
             * Device has been found. Now find whether this is
             * the device we are looking for.
             */
            if (dev_specific == dev_hdr_p)
            {
              /*
               * This is the file descriptor related to "/pty/telnet. M".
               * Make it the global standard.
               */
              /*
               * Somehow, the same device has two file descriptors
               * and the second is the right one). Increase FD by one.
               */
              device_fd++ ;
#if TELNET_DEBUG_PRINT
              logMsg("-> -> -> -> Device FD has been found: %d\n",
                (int)device_fd,(int)0,0,0,0,0) ;
#endif
              d_ioGlobalStdSet(STD_IN,device_fd) ;
              d_ioGlobalStdSet(STD_OUT,device_fd) ;
              d_ioGlobalStdSet(STD_ERR,device_fd) ;
              break ;
            }
          }
        }
      }
      trigger_delay_alive = MAX_TRIGGER_DELAY ;
    }
    else
    {
      /*
       * Either trigger delay is not completed or telnet status
       * has already been updated to BOTH_ALIVE.
       */
      if (trigger_delay_alive > MAX_TRIGGER_DELAY)
      {
        /*
         * Just protection against error.
         */
        trigger_delay_alive = MAX_TRIGGER_DELAY ;
      }
      if ((trigger_delay_alive > 0) && !is_telnet_active())
      {
        /*
         * decrement delay counter if file descriptor switch
         * has not been carried out yet.
         */
        trigger_delay_alive-- ;
      }
    }
  }
  actual_fd = d_ioGlobalStdGet(STD_IN) ;
  current_fd = get_current_ui_fd() ;
  timer_id = get_ui_timer_id() ;
  {
    /*
     * Make sure task is alive. If it is not, revive
     * it. This may happen when some fatal error caused the
     * operating system to suspend the task.
     * Apply this protection only if it is not a debug
     * session since, while debugging, tasks are suspended
     * when breakpoints are encountered.
     */
    ui_suspended = FALSE ;
    status = taskNameToId("tDbgTask") ;
    if (status == ERROR)
    {
      /*
       * Handle recovery only if debug task is NOT alive.
       */
      ui_task_id = get_task_id(USER_INTERFACE_TASK_ID) ;
      if (taskIsSuspended(ui_task_id))
      {
        /*
         * Enter here only if user interface task is suspended.
         */
        /*
         * When DPSS is included then ui task may occasionally
         * be suspended for short times because of 'blocked'
         * operations.
         */
        inc_ui_suspend_timer() ;
        if (is_ui_suspend_timer_overflown())
        {
          ui_suspended = TRUE ;
          /*
           * Set indication on recovery from fatal exception
           * [to display at User Interface task, ui_proc()].
           */
#if !DUNE_BCM
          set_recovery_msg_on() ;
#endif
          /*
           * Make sure task is not 'safe'. This is not strictly legal
           * but neither is this whole situation.
           */
          d_force_task_unsafe(ui_task_id) ;
          clear_ui_suspend_timer() ;
        }
      }
      else
      {
        clear_ui_suspend_timer() ;
      }
    }
  }
  {
    /*
     * Check whether telnet connection has timed out. If so, disconnect.
     */
    telnet_timeout_elapsed = has_telnet_inactivity_time_elapsed() ;
  }
  if ((actual_fd != current_fd) || ui_suspended || telnet_timeout_elapsed)
  {
    int
      user_prep_task_id ;
    /*
     * EITHER
     *   Both
     *    Standard file descriptor has changed
     *   And
     *    Local delay counter expired
     * OR
     *   User interface task has been suspended
     * OR
     *   Timeout on telnet activity has been detected
     *   (This could only happen when Telnet is active)
     * Kick the user interface task.
     */
    if (actual_fd != current_fd)
    {
#if TELNET_DEBUG_PRINT
      logMsg(
        "ui_proc_signal - Entry #%d - FDs not equal: actual: %d, current_fd: %d.\n\n",
        signal_count,actual_fd,current_fd,0,0,0) ;
      logMsg(
        "ui_proc_signal - %s %s\r\n",
        (int)telnet_in_msg,(int)telnet_out_msg,0,0,0,0) ;
#endif
    }
    if (telnet_timeout_elapsed)
    {
      telnet_timeout = get_telnet_timeout_val() ;
      logMsg(
        "Telnet inactivity timeout (%d seconds). Disconnecting...\r\n",
        (int)telnet_timeout,0,0,0,0,0) ;
      logout() ;
      taskDelay(60) ;
    }
    /*
     * Delete timer. It will be created again at the restarted task.
     */
    status = timer_delete(timer_id) ;
    gen_err(TRUE,TRUE,(int)status,ERROR,
          "ui_proc_signal - timer_delete",
              proc_name,SVR_FTL,UI_PROC_SIGNAL_ERR_01,FALSE,0,-1,FALSE) ;
    user_prep_task_id = get_task_id(USER_PREP_TASK_ID) ;
    status = taskRestart(user_prep_task_id) ;
    gen_err(TRUE,TRUE,(int)status,ERROR,
          "ui_proc_signal - taskRestart",
              proc_name,SVR_FTL,UI_PROC_SIGNAL_ERR_02,FALSE,0,-1,FALSE) ;
  }
  else
  {
    /*
     * Just restart timer for periodic check up.
     */
    struct itimerspec
      timer_spec ;
    /*
     * Set timer to expire 1000 ms from now.
     */
    timer_spec.it_value.tv_sec = 1 ;
    timer_spec.it_value.tv_nsec = 0 ;
    timer_spec.it_interval.tv_sec = 0 ;
    timer_spec.it_interval.tv_nsec = 0 ;
    status = timer_settime(timer_id,0,&timer_spec,NULL) ;
    gen_err(TRUE,TRUE,(int)status,ERROR,
          "ui_proc_signal - timer_settime",
                proc_name,SVR_FTL,UI_PROC_SIGNAL_ERR_03,FALSE,0,-1,FALSE) ;
  }
  return ;
}
/*****************************************************
*$NAME$
*  ui_tmr_new_n_kick
*TYPE: PROC
*DATE: 17/JAN/2002
*FUNCTION:
*  Create the user interface timer (which checks
*  whether it is telnet or standard local terminal
*  which is currently connected.
*CALLING SEQUENCE:
*  ui_tmr_new_n_kick()
*INPUT:
*  SOC_SAND_DIRECT:
*    None.
*  SOC_SAND_INDIRECT:
*    Ui timer variables and utilities
*OUTPUT:
*  SOC_SAND_DIRECT:
*    None.
*  SOC_SAND_INDIRECT:
*    Created and armed timer.
*REMARKS:
*  Should be called within the context of UI_PREP_TASK.
*SEE ALSO:
*****************************************************/
void
  ui_tmr_new_n_kick(
    void
  )
{
  /*
   * Timer for user prep (and user interface) task.
   */
  timer_t
    timer_id ;
  /*
   * Timer specifications structure.
   */
  struct itimerspec
    timer_spec ;
  struct sigaction
    new_action,
    old_action ;
  sigset_t
    old_set ;
  STATUS
    status ;
  char
    *proc_name ;
  proc_name = "ui_tmr_new_n_kick" ;
  /*
   * Timer and signal handling
   */
  /*
   * Create a timer for this task. Set 'sigevent'
   * procedure to 'NULL' so that the default signal
   * (SIGALRM) is queued to the task.
   */
  d_timer_create(CLOCK_REALTIME,NULL,&timer_id) ;
  set_ui_timer_id(timer_id) ;
  /*
   * Point to subroutine to call when timer expires.
   */
  memset(&new_action,0,sizeof(new_action)) ;
  memset(&old_action,0,sizeof(new_action)) ;
  new_action.sa_handler = ui_proc_signal ;
  status = sigaction(SIGALRM,&new_action,&old_action) ;
  gen_err(TRUE,TRUE,(int)status,ERROR,
        "sigaction - ",proc_name,
                SVR_FTL,UI_TMR_NEW_N_KICK_ERR_01,FALSE,0,-1,FALSE) ;
  status = sigprocmask(SIG_SETMASK,NULL,&old_set) ;
  gen_err(TRUE,TRUE,(int)status,ERROR,
        "sigprocmask - ",proc_name,
              SVR_FTL,UI_TMR_NEW_N_KICK_ERR_02,FALSE,0,-1,FALSE) ;
  /*
   * Set timer to expire 1000 ms from now.
   */
  timer_spec.it_value.tv_sec = 1 ;
  timer_spec.it_value.tv_nsec = 0 ;
  timer_spec.it_interval.tv_sec = 0 ;
  timer_spec.it_interval.tv_nsec = 0 ;
  status = timer_settime(timer_id,0,&timer_spec,NULL) ;
  gen_err(TRUE,TRUE,(int)status,ERROR,
        "timer_settime - ",proc_name,
              SVR_FTL,UI_TMR_NEW_N_KICK_ERR_03,FALSE,0,-1,FALSE) ;
  return ;
}
/* } */
#endif /* __VXWORKS__ */

/*****************************************************
*NAME
*  handle_back_space
*TYPE:
*  PROC
*DATE:
*  24/SEP/2002
*FUNCTION:
*  Handle back space key entered on input command
*  line: Change processing state and redisplay.
*CALLING SEQUENCE:
*  handle_back_space(current_line)
*INPUT:
*  SOC_SAND_DIRECT:
*    CURRENT_LINE *current_line -
*      Pointer to current prompt line.
*  SOC_SAND_INDIRECT:
*    None.
*OUTPUT:
*  SOC_SAND_DIRECT:
*    int -
*      If non zero then some error has been
*      encountered. In that case, this procedure
*      has already called 'gen_err'.
*  SOC_SAND_INDIRECT:
*    Updated current_line.
*REMARKS:
*  None
*SEE ALSO:
*****************************************************/
int
  handle_back_space(
    CURRENT_LINE *current_line
  )
{
  int
    ret ;
  const char
    *proc_name ;
  proc_name = "handle_back_space" ;
  ret = FALSE ;
  switch (current_line->proc_state)
  {
    case RECEIVING_SUBJECT:
    {
      /*
       * Erase last typed in character which is
       * assumed to be last on line (not 'insert' mode).
       */
      current_line->line_buf_index-- ;
      current_line->num_loaded-- ;
      current_line->num_loaded_this_field-- ;
      current_line->subject_entered = FALSE ;
      if (current_line->num_loaded == 0)
      {
        current_line->proc_state =
                get_blank_proc_state(current_line) ;
      }
      back_space_line(current_line) ;
      break ;
    }
    case EXPECTING_ENTER:
    case EXPECTING_PARAM_STRING:
    case (EXPECTING_PARAM_STRING | EXPECTING_PARAM_VAL):
    case EXPECTING_PARAM_VAL:
    {
      /*
       * Erase last typed in character which is
       * assumed to be a apace.
       */
      unsigned
        int
          param_index ;
      current_line->line_buf_index-- ;
      current_line->num_loaded-- ;
      /*
       * Note: current_line->num_loaded_this_field should
       * be '0' at this point. Must be updated below.
       */
      /*
       * Go back to previous state.
       */
      param_index = current_line->param_index ;
      if ((param_index == 0) &&
           (current_line->num_param_names == 0))
      {
        /*
         * If this is the very first parameter then
         * go back to 'RECEIVING_SUBJECT'.
         */
        current_line->proc_state = RECEIVING_SUBJECT ;
        current_line->subject_entered = FALSE ;
        current_line->
          num_loaded_this_field = current_line->num_loaded ;
        current_line->start_index_this_field = 0 ;
        /*
         * Not necessary but safer.
         */
        current_line->param_val_pair[
                            param_index].in_val_index = 0 ;
        current_line->param_string_entered = FALSE ;
        current_line->current_param_match_index = 0 ;
        current_line->num_param_values = 0 ;
        current_line->num_param_names = 0 ;
      }
      else
      {
        /*
         * If this is not the very first parameter then
         * either go back to 'RECEIVING_PARAM_STRING' or
         * to 'RECEIVING_PARAM_VAL'.
         */
        unsigned
          int
            type,
            str_len_par ;
        if ((current_line->proc_state == EXPECTING_ENTER) ||
              (current_line->proc_state ==  EXPECTING_PARAM_STRING))
        {
          /*
           * In these states, 'param_index' has been advanced
           * since a full 'param_val_pair' could be loaded
           * and closed. This means 'current_line->param_index'
           * must be larger than 1.
           */
          current_line->param_index-- ;
        }
        else
        {
          /*
           * In the state of 'EXPECTING_PARAM_VAL'
           * and in the fuzzy state of
           * 'EXPECTING_PARAM_STRING | EXPECTING_PARAM_VAL',
           * 'param_index' has been advanced
           * only if one value has already been entered
           * since a full 'param_val_pair' could then be loaded
           * and closed.
           */
          if (current_line->num_param_values)
          {
            current_line->param_index-- ;
          }
        }
        param_index = current_line->param_index ;
        type =
          current_line->
              param_val_pair[param_index].param_value.val_type ;
        if (type == VAL_NONE)
        {
          /*
           * Enter if no values have been entered for
           * the previous (or current) parameter.
           */
          current_line->proc_state = RECEIVING_PARAM_STRING ;
          current_line->param_string_entered = FALSE ;
          current_line->num_param_names-- ;
          if (param_index > 0)
          {
            type =
              current_line->
                  param_val_pair[param_index-1].param_value.val_type ;
            if (type == VAL_NONE)
            {
              /*
               * Enter if no values have been entered for
               * the previous parameter (which is now the last
               * valid parameter).
               */
              current_line->num_param_values = 0 ;
            }
            else
            {
              /*
               * Enter if some values have been entered for
               * the previous parameter (which is now the last
               * valid parameter).
               */
              current_line->num_param_values =
                current_line->
                    param_val_pair[param_index-1].in_val_index + 1 ;
            }
          }
          else
          {
            /*
             * If this is the first parameter then no values
             * have been entered for it.
             */
            current_line->num_param_values = 0 ;
          }
          current_line->
              param_val_pair[param_index].in_val_index = 0 ;
          /*
           * update to indicate how many characters have been
           * entered for last field and where it did start.
           */
          str_len_par =
            strlen(current_line->param_val_pair[
                                param_index].par_name) ;
          current_line->num_loaded_this_field = str_len_par ;
          current_line->start_index_this_field -= (str_len_par + 1) ;
          current_line->param_val_pair[
                                param_index].par_name = "" ;
        }
        else
        {
          /*
           * Enter if at least one value has been entered for
           * the previous parameter.
           */
          unsigned
            int
              first_char_index ;
          current_line->proc_state = RECEIVING_PARAM_VAL ;
          /*
           * Effectively, the following decreases 'num_param_values'
           * by one.
           */
          current_line->num_param_values =
            current_line->param_val_pair[
                            param_index].in_val_index ;

          first_char_index = index_of_last_space(current_line) + 1 ;
          current_line->num_loaded_this_field =
            current_line->line_buf_index - first_char_index ;
          current_line->start_index_this_field = first_char_index ;
          current_line->
              param_val_pair[param_index].param_value.val_type = VAL_NONE ;
          current_line->param_string_entered = TRUE ;
          current_line->current_param_match_index =
            current_line->param_val_pair[
                            param_index].param_match_index ;
        }
      }
      back_space_line(current_line) ;
      break ;
    }
    case RECEIVING_PARAM_VAL:
    {
      /*
       * Erase last typed in character which is
       * assumed to not be the first in that field.
       */
      unsigned
        int
          index ;
      index = current_line->line_buf_index - 1 ;
      /*
       * If the character to delete is a quotation mark then
       * this must a a text field. In that case, since quotation
       * marks always come in pairs, first deleted quotation
       * mark gets the system into 'quotation_mark_entered'
       * while the second gets the system out of
       * 'quotation_mark_entered'.
       */
      if (current_line->line_buf[index] == QUOTATION_MARK)
      {
        if (current_line->quotation_mark_entered)
        {
          current_line->quotation_mark_entered = FALSE ;
        }
        else
        {
          current_line->quotation_mark_entered = TRUE ;
        }
      }
      current_line->line_buf_index-- ;
      current_line->num_loaded-- ;
      current_line->num_loaded_this_field-- ;
      if (current_line->num_loaded_this_field == 0)
      {
        /*
         * Enter if the whole value has been deleted.
         */
        current_line->proc_state =
                get_blank_proc_state(current_line) ;
      }
      back_space_line(current_line) ;
      break ;
    }
    case RECEIVING_PARAM_STRING:
    {
      /*
       * Erase last typed in character which is
       * assumed to not be the first in that field.
       */
      current_line->line_buf_index-- ;
      current_line->num_loaded-- ;
      current_line->num_loaded_this_field-- ;
      if (current_line->num_loaded_this_field == 0)
      {
        /*
         * Enter if the whole parameter string has been deleted.
         */
        unsigned
          int
            type,
            param_index ;
        PARAM_VAL_PAIR
          *param_val_pair ;
        param_index = current_line->param_index ;
        if (param_index > 0)
        {
          /*
           * Check the previous 'param_val_pair' entry.
           * Update 'param_string_entered'.
           */
          param_val_pair =
            &current_line->param_val_pair[param_index - 1] ;
          current_line->param_string_entered = TRUE ;
          current_line->
            current_param_match_index =
                  param_val_pair->param_match_index ;
          type = param_val_pair->param_value.val_type ;
          if (type == VAL_NONE)
          {
            /*
             * Enter if no values have been entered for
             * the previous parameter.
             */
            current_line->num_param_values = 0 ;
          }
          else
          {
            current_line->num_param_values =
                      param_val_pair->in_val_index + 1 ;
          }
          current_line->proc_state =
                get_blank_proc_state(current_line) ;
          /*
           * Note that, from the state of RECEIVING_PARAM_STRING,
           * we should only get to either EXPECTING_PARAM_STRING
           * or (EXPECTING_PARAM_STRING | EXPECTING_PARAM_VAL)
           */
          if (current_line->proc_state == EXPECTING_PARAM_STRING)
          {
            current_line->param_string_entered = FALSE ;
          }
          else if (
            current_line->proc_state ==
                    (EXPECTING_PARAM_STRING | EXPECTING_PARAM_VAL))
          {
            /*
             * If previous parameter did not get any value
             * then decrement 'param_index' since 'param_val_pair'
             * has place for a potential PARAM_VAL.
             */
            if (current_line->num_param_values == 0)
            {
              current_line->param_index-- ;
            }
          }
        }
        else
        {
          current_line->proc_state =
                get_blank_proc_state(current_line) ;
        }
      }
      back_space_line(current_line) ;
      break ;
    }
    default:
    {
      char
        err_string[100] ;
      sal_sprintf(err_string,
        "Unexpected proc_state (%d)",current_line->proc_state) ;
      gen_err(FALSE,TRUE,(int)0,0,err_string,
                  proc_name,SVR_FTL,
                  HANDLE_BACK_SPACE_ERR_01,FALSE,0,-1,FALSE) ;
      break ;
    }
  }
  return (ret) ;
}
/*
 * OBJECT:
 * CLI error counter
 * Counts the number of syntax (or other) errors (made by
 * the user while typing in a command line and reported
 * to the user with a message usually starting with '***'
 * {
 */
static
  unsigned
    int
      Cli_err_counter = 0 ;
unsigned
  int
    get_cli_err_counter(
      void
    )
{
  return (Cli_err_counter) ;
}
void
  inc_cli_err_counter(
    void
  )
{
  Cli_err_counter++ ;
}
void
  clear_cli_err_counter(
    void
  )
{
  Cli_err_counter = 0 ;
  return ;
}
/*
 * }
 */
/*
 * OBJECT:
 * Echo - mode
 * Get/set value of CLI flag which indicates whether
 * characters received from the user are to be echoed
 * back to the sender or not.
 * {
 */
static
  unsigned
    int
      No_echo_mode = 0 ;
unsigned
  int
    is_echo_mode_on(
      void
    )
{
  return (!No_echo_mode) ;
}
void
  set_echo_mode_on(
    void
  )
{
  No_echo_mode = 0 ;
  return ;
}
void
  set_echo_mode_off(
    void
  )
{
  No_echo_mode = 1 ;
  return ;
}
/*
 * }
 */
/*****************************************************
*NAME
*  handle_next_char
*TYPE: PROC
*DATE: 20/JAN/2002
*FUNCTION:
*  handle the next character entered on the current line.
*CALLING SEQUENCE:
*  handle_next_char(current_line,in_char)
*INPUT:
*  SOC_SAND_DIRECT:
*    CURRENT_LINE **current_line_ptr.
*      Pointer to pointer to a structure of type CURRENT_LINE. This
*      structure holds all information required to
*      handle the next character and also holds the result
*      of processing this character.
*    char in_char -
*      The character typed in.
*  SOC_SAND_INDIRECT:
*    Contents of current_line.
*OUTPUT:
*  SOC_SAND_DIRECT:
*    None.
*  SOC_SAND_INDIRECT:
*    Updated current_line structure. Possibly updated
*    value of current_line pointer.
*REMARKS:
*  NONE.
*SEE ALSO:
*****************************************************/
void
  handle_next_char(
    CURRENT_LINE **current_line_ptr,
    char         in_char
  )
{
  const char
    *proc_name ;
  static
    unsigned long
      num_chars_handled ;
  CURRENT_LINE
    *current_line ;
  char
    found_names_list[400*40] ;
  int
    err ;
  proc_name = "handle_next_char" ;
  clear_num_lines_printed() ;
  current_line = *current_line_ptr ;
  num_chars_handled++ ;
  
  switch (in_char)
  {
    case ENTER_ECHO_MODE_OFF:
    {
      if (is_echo_mode_on())
      {
        /*
         * If user requested to stop echo mode while echo mode was on,
         * ring bell and display message announcing 'no echo from now on'
         */
        send_char_to_screen(BELL_CHAR) ;
        send_string_to_screen(
            "\r\n"
            "==> NO ECHO from this point on. To get echo back, hit \'ctrl-f\'\r\n\n",
            FALSE) ;
        set_echo_mode_off() ;
      }
      goto exit ;
    }
    case ENTER_ECHO_MODE_ON:
    {
      if (!is_echo_mode_on())
      {
        /*
         * If user requested to return to echo mode while echo mode was off,
         * ring bell and display message announcing 'echo from now on'
         */
        send_char_to_screen(BELL_CHAR) ;
        msg_and_redisplay(
            "\r\n"
            "==> ECHO ON from this point on. To stop echo, hit \'ctrl-e\'\r\n\n",
            current_line) ;
        set_echo_mode_on() ;
        redisplay_line(current_line) ;
      }
      goto exit ;
    }
    case FLUSH_STD_OUTPUT:
    {
      send_char_to_screen(BELL_CHAR) ;
      msg_and_redisplay(
            "\r\n"
            "==> Standard output flushed by the user\r\n\n",
            current_line) ;
      redisplay_line(current_line) ;
      goto exit ;
    }
    default:
    {
      break ;
    }
  }
  if (!is_line_mode_set())
  {
    /*
     * Here handle SINGLE-CHARACTER mode.
     */
    /*
     * If system is in 'quotation_mark_entered' state then
     * convert space into the special '$' character and, thus,
     * make sure space is handled as a standard ascii character.
     */
    if (current_line->quotation_mark_entered)
    {
      if (in_char == SPECIAL_DOLLAR)
      {
        in_char = '^' ;
      }
      else if (in_char == BLANK_SPACE)
      {
        in_char = SPECIAL_DOLLAR ;
      }
    }
    switch (in_char)
    {
      case DOWN_ARROW:
      {
        show_next_history_line(current_line_ptr) ;
        break ;
      }
      case UP_ARROW:
      {
        show_prev_history_line(current_line_ptr) ;
        break ;
      }
      case DEBUG_INFO:
      {
        /*
         * Print selected debug information. Debug aid only.
         */
        /*
         * This character is not counted as standard CLI characters.
         */
        num_chars_handled-- ;
        display_line_info(current_line) ;
        redisplay_line(current_line) ;
        break ;
      }
      case BACK_SPACE:
      case DEL_KEY:
      {
        if (current_line->line_buf_index == 0)
        {
          /*
           * Enter if cursor is at the beginning of the line and
           * user is trying top delete to the left. Impossible.
           * Formerly we displayed an error message, now we ring the bell.
           */
          send_char_to_screen(BELL_CHAR) ;
          /*
          msg_and_redisplay(
              "\r\n"
              "*** Can not back space: Beginning of line...\r\n",
              current_line) ;
          */
          inc_cli_err_counter() ;
        }
        else
        {
          handle_back_space(current_line) ;
        }
        break ;
      }
      case CARRIAGE_RETURN:
      case LINE_FEED:
      {
        /*
         * Here process input line and display prompt on next
         * clean line
         */
        CURRENT_LINE
          *line_to_process ;
        /*
         * If last character is 'back space' (might happen if
         * more parameter strings or parameter data items could
         * be added) then get rid of it by pretending 'back-space'
         * has been hit).
         */
        {
          unsigned
            long
              line_buf_index ;

          if (current_line->line_buf_index)
          {
            line_buf_index = current_line->line_buf_index - 1 ;
            if (current_line->line_buf[line_buf_index] == BLANK_SPACE)
            {
              handle_back_space(current_line) ;
            }
          }
        }
        /*
         * Save pointer to current line (which is the line
         * to process) in 'line_to_process' and, then, store
         * 'current_line' in 'history' fifo and replace
         * it with a fresh pointer.
         * 'History' commands do not go into fifo.
         */
        line_to_process = current_line ;
        soc_sand_os_task_lock() ;
        put_history_fifo(current_line_ptr) ;
        current_line = *current_line_ptr ;
        soc_sand_os_task_unlock() ;

        /*
         * Make sure this task is not deleted half way through.
         * This is important for handling of the chip set
         * and also for handling of dynamic memory (e.g. history
         * fifo).
         */
        switch (line_to_process->proc_state)
        {
          case RECEIVING_PASSWORD:
          {
            process_line(line_to_process,current_line_ptr) ;
            /*
             * Display prompt and clear line.
             */
            current_line = *current_line_ptr ;
            redisplay_line(current_line) ;
            break ;
          }
          case EXPECTING_PASSWORD:
          {
            /*
             * If CR is encountered while expecting password then issue warninmg.
             */
            msg_and_redisplay(
              "\r\n"
              "*** Expecting password...\r\n",
              current_line) ;
            inc_cli_err_counter() ;
            break ;
          }
          case EXPECTING_SUBJECT:
          {
            /*
             * If CR is encountered while expecting subject then just redisplay
             * prompt.
             */
            /*
             * Actually send to screen only if echo mode is on.
             */
            if (is_echo_mode_on())
            {
              send_array_to_screen("\r\n",2) ;
            }
            redisplay_line(current_line) ;
            break ;
          }
          case EXPECTING_ENTER:
          case EXPECTING_PARAM_STRING:
          case (EXPECTING_PARAM_STRING | EXPECTING_PARAM_VAL):
          {
            process_line(line_to_process,current_line_ptr) ;
            /*
             * Display prompt and clear line.
             */
            current_line = *current_line_ptr ;
            redisplay_line(current_line) ;
            break ;
          }
          case EXPECTING_PARAM_VAL:
          {
            /*
             * <ENTER> hit while parameter value is being expected.
             */
            char
              err_msg[160+MAX_SIZE_OF_SYMBOL] ;
            PARAM
              *parameter_ptr ;
            parameter_ptr =
              &line_to_process->line_subject->
                allowed_params[line_to_process->current_param_match_index] ;
            sal_sprintf(err_msg,
              "\r\n\n"
              "*** Cannot process line. This parameter expects values...\r\n\n"
              "List of values for this parameter (\'%s\'):",
              line_to_process->
                parameters[line_to_process->num_param_names - 1]->par_name) ;
            inc_cli_err_counter() ;
            send_string_to_screen(err_msg,TRUE) ;
            send_string_to_screen(
                parameter_ptr->vals_detailed_help,TRUE) ;
            clear_n_redisplay_line(current_line) ;
            break ;
          }
          case RECEIVING_PARAM_VAL:
          {
            /*
             * <ENTER> hit while parameter value is being received.
             */
            err = space_receiving_param_val(line_to_process,TRUE) ;
            if (!err)
            {
              /*
               * Since a parameter is expected, try to process line.
               */
              process_line(line_to_process,current_line_ptr) ;
              current_line = *current_line_ptr ;
            }
            else
            {
              /*
               * Error condition. Just display clear line.
               */
              init_current_line(current_line) ;
            }
            redisplay_line(current_line) ;
            break ;
          }
          case RECEIVING_PARAM_STRING:
          {
            /*
             * <ENTER> hit while parameter string is being received.
             */
            err = space_receiving_param_string(line_to_process,TRUE) ;
            if (!err)
            {
              /*
               * Since a parameter is expected, try to process line.
               */
              process_line(line_to_process,current_line_ptr) ;
              current_line = *current_line_ptr ;
            }
            else
            {
              /*
               * Error condition. Just display clear line.
               */
              init_current_line(current_line) ;
            }
            redisplay_line(current_line) ;
            break ;
          }
          case RECEIVING_SUBJECT:
          {
            err =
              space_while_receiving_subject(
                            line_to_process,
                            TRUE
              ) ;
            if (!err)
            {
              process_line(line_to_process,current_line_ptr) ;
              current_line = *current_line_ptr ;
            }
            else
            {
              /*
               * Error condition. Just display clear line.
               */
              init_current_line(current_line) ;
            }
            redisplay_line(current_line) ;
            break ;
          }
          default:
          {
            char
              err_string[100] ;
            sal_sprintf(err_string,
              "*** Unexpected proc_state (%d)",current_line->proc_state) ;
            inc_cli_err_counter() ;
            gen_err(FALSE,TRUE,(int)0,0,err_string,
                        proc_name,SVR_FTL,
                        HANDLE_NEXT_CHAR_ERR_02,FALSE,0,-1,FALSE) ;
            break ;
          }
        }
        /*
         * Little patch:
         * The line that goes into the 'history' buffer must
         * be set to its original state:
         * Within line processing, the flag 'current_line->param_string_entered'
         * may have been set 'false' to indicate no more param
         * it out of the history values are expected. However, when we
         * restart the line by taking buffer, we want to be in the very
         * same original state.
         * This is specifically for (EXPECTING_PARAM_STRING | EXPECTING_PARAM_VAL)
         * state since we want to be able to enter more values.
         */
        if (line_to_process->proc_state == (EXPECTING_PARAM_STRING | EXPECTING_PARAM_VAL))
        {
          if (!(line_to_process->param_string_entered))
          {
            line_to_process->param_string_entered  = TRUE ;
          }
        }
        break ;
      }
      case ESC:
      case CLEAR_TYPE_AHEAD:
      {
        /*
         * Indicate to user that periodic display has been stopped
         * and type ahead buffer has been flushed.
         * Action has been done by calling task.
         */
        if (is_display_periodic_on())
        {
          msg_and_redisplay(
              "\r\n"
              "==> Periodic display stopped (and type ahead buffer manually flushed).\r\n",
              current_line) ;
          set_display_periodic_off() ;
        }
        else
        {
          msg_and_redisplay(
              "==> Type ahead buffer manually flushed.",
              current_line) ;
        }
        clear_n_redisplay_line(current_line) ;

#if LINK_PSS_LIBRARIES
        if (test_fap10m_field_is_test_started())
        {
          test_fap10m_field_stop();
        }
#endif
        break ;
      }
      case CLEAR_LINE:
      {
        /*
         * Clean line of all input
         */
        /*
         * Display clear line with prompt.
         */
        clear_n_redisplay_line(current_line) ;
        break ;
      }
      case EXIT_TERMINAL:
      {
        /* ask ui_proc to finish */
        S_ui_proc_continue = FALSE; 

        break;

      }
      case QUESTION_MARK:
      {
        /*
         * Help
         */
        /*
         * First, echo the character.
         */
        /*
         * Actually send to screen only if echo mode is on.
         */
        if (is_echo_mode_on())
        {
          send_char_to_screen(in_char) ;
        }
        /*
         * This character is not counted as standard CLI characters.
         */
        num_chars_handled-- ;
        switch (current_line->proc_state)
        {
          case EXPECTING_ENTER:
          {
            msg_and_redisplay(
              "<CR>",
              current_line) ;
            inc_cli_err_counter() ;
            break ;
          }
          case EXPECTING_SUBJECT:
          {
            /*
             * Display all subjects with short help.
             */
            const char
              *info_msg ;
            info_msg =
                  "\r\n\n"
                  "Expecting SUBJECT..." ;
            send_string_to_screen(info_msg,TRUE) ;
            all_subjs_short_help(current_line,TRUE) ;
            redisplay_line(current_line) ;
            break ;
          }
          case RECEIVING_SUBJECT:
          {
            /*
             * Display detailed help on all matching subjects.
             */
            print_detailed_help(current_line,TRUE) ;
            redisplay_line(current_line) ;
            break ;
          }
          case EXPECTING_PARAM_STRING:
          {
            /*
             * Display all subjects with short help.
             */
            all_subjs_short_help(current_line,FALSE) ;
            redisplay_line(current_line) ;
            break ;
          }
          case (EXPECTING_PARAM_STRING | EXPECTING_PARAM_VAL):
          {
            /*
             * Display all subjects with short help.
             */
            PARAM
              *parameter_ptr ;
            char
              err_msg[100+MAX_SIZE_OF_SYMBOL] ;
            parameter_ptr =
              &current_line->line_subject->
                  allowed_params[current_line->current_param_match_index] ;
            {
              /*
               * Display help for variable
               */
              sal_sprintf(err_msg,
                  "\r\n\n"
                  "EITHER expecting VALUE for this PARAMETER (\'%s\')...",
                  parameter_ptr->par_name) ;
              send_string_to_screen(err_msg,TRUE) ;
              sal_sprintf(err_msg,
                    "\r\n\n"
                    "List of acceptable values for this parameter (\'%s\'):",
                    parameter_ptr->par_name) ;
              send_string_to_screen(err_msg,TRUE) ;
              send_string_to_screen(parameter_ptr->vals_detailed_help,TRUE) ;
            }
            {
              /*
               * Display help for parameter
               */
              sal_sprintf(err_msg,
                    "\r\n\n"
                    "OR expecting a new PARAMETER...") ;
              send_string_to_screen(err_msg,TRUE) ;
              all_subjs_short_help(current_line,FALSE) ;
              sal_sprintf(err_msg,
                    "List of values for this parameter (\'%s\'):",
                    parameter_ptr->par_name) ;
              send_string_to_screen(err_msg,FALSE) ;
              msg_and_redisplay(
                    parameter_ptr->vals_detailed_help,
                    current_line) ;
            }
            break ;
          }
          case EXPECTING_PARAM_VAL:
          case RECEIVING_PARAM_VAL:
          {
            /*
             * Display all values for this parameter with help.
             */
            PARAM
              *parameter_ptr ;
            char
              err_msg[80+MAX_SIZE_OF_SYMBOL] ;
            parameter_ptr =
              &current_line->line_subject->
                  allowed_params[current_line->current_param_match_index] ;
            if (current_line->proc_state == EXPECTING_PARAM_VAL)
            {
              sal_sprintf(err_msg,
                  "\r\n\n"
                  "Expecting VALUE for this PARAMETER (\'%s\')...",
                  parameter_ptr->par_name) ;
              send_string_to_screen(err_msg,TRUE) ;
              sal_sprintf(err_msg,
                    "\r\n\n"
                    "List of acceptable values for this parameter (\'%s\'):",
                    parameter_ptr->par_name) ;
              send_string_to_screen(err_msg,TRUE) ;
              msg_and_redisplay(
                      parameter_ptr->vals_detailed_help,
                      current_line) ;
            }
            else
            {
              if ((parameter_ptr->num_ip_vals != 0)     ||
                  (parameter_ptr->num_text_vals != 0)   ||
                  (is_last_field_numeric(current_line))
                )
              {
                sal_sprintf(err_msg,
                      "\r\n\n"
                      "List of acceptable values for this parameter (\'%s\'):",
                      parameter_ptr->par_name) ;
                send_string_to_screen(err_msg,TRUE) ;
                msg_and_redisplay(
                        parameter_ptr->vals_detailed_help,
                        current_line) ;
              }
              else
              {
                sal_sprintf(err_msg,
                    "\r\n\n"
                    "List of acceptable values for this parameter (\'%s\'):",
                    parameter_ptr->par_name) ;
                send_string_to_screen(err_msg,TRUE) ;
                help_for_match(
                    &current_line->line_buf[current_line->start_index_this_field],
                    current_line->num_loaded_this_field,
                    parameter_ptr) ;
                send_array_to_screen("\r\n",2) ;
                redisplay_line(current_line) ;
              }
            }
            break ;
          }
          case RECEIVING_PARAM_STRING:
          {
            /*
             * Display detailed help on all matching subjects.
             */
            print_detailed_help(current_line,FALSE) ;
            redisplay_line(current_line) ;
            break ;
          }
          default:
          {
            char
              err_string[100] ;
            sal_sprintf(err_string,
              "Unexpected proc_state (%d)",current_line->proc_state) ;
            gen_err(FALSE,TRUE,(int)0,0,err_string,
                      proc_name,SVR_FTL,
                      HANDLE_NEXT_CHAR_ERR_04,FALSE,0,-1,FALSE) ;
            break ;
          }
        }
        break ;
      }
      case TAB:
      case BLANK_SPACE:
      {
        switch (current_line->proc_state)
        {
          case EXPECTING_ENTER:
          {
            msg_and_redisplay(
              "<CR>",
              current_line) ;
            inc_cli_err_counter() ;
            break ;
          }
          case EXPECTING_SUBJECT:
          {
            /*
             * Display all subjects with short help.
             */
            msg_and_redisplay(
                "*** Expecting subject, not blank space...\r\n",
                current_line) ;
            inc_cli_err_counter() ;
            break ;
          }
          case EXPECTING_PARAM_STRING:
          {
            /*
             * Space expecting parameter string.
             */
            inc_cli_err_counter() ;
            break ;
          }
          case EXPECTING_PARAM_VAL:
          {
            /*
             * Space expecting parameter value.
             */
            inc_cli_err_counter() ;
            break ;
          }
          case (EXPECTING_PARAM_VAL | EXPECTING_PARAM_STRING):
          {
            /*
             * Space expecting parameter value or parameter string.
             */
            inc_cli_err_counter() ;
            break ;
          }
          case RECEIVING_SUBJECT:
          {
            /*
             * Move from subject to first parameter.
             */
            err = space_while_receiving_subject(current_line,FALSE) ;
            if (err)
            {
              /*
               * We should not display again -- 'space_while_receiving_subject' is doing the
               *  rediplay.
               *
               redisplay_line(current_line) ;
               */
            }
            break ;
          }
          case RECEIVING_PARAM_STRING:
          {
            /*
             * Move to parameter value (or to next parameter).
             */
            err = space_receiving_param_string(current_line,FALSE) ;
            if (err)
            {
             /*
              * We should not display again -- 'space_receiving_param_string' is doing the
              *  rediplay.
              *
              redisplay_line(current_line) ;
              */
            }
            break ;
          }
          case RECEIVING_PARAM_VAL:
          {
            /*
             * Move to next parameter (or to next parameter value).
             */
            err = space_receiving_param_val(current_line,FALSE) ;
            if (err)
            {
              /*
               * We should not display again -- 'space_receiving_param_val' is doing the
               *  rediplay.
               *
               redisplay_line(current_line) ;
               */
            }
            break ;
          }
          default:
          {
            char
              err_string[100] ;
            sal_sprintf(err_string,
              "Unexpected proc_state (%d)",current_line->proc_state) ;
            gen_err(FALSE,TRUE,(int)0,0,err_string,
                      proc_name,SVR_FTL,
                      HANDLE_NEXT_CHAR_ERR_05,FALSE,0,-1,FALSE) ;
            break ;
          }
        }
        break ;
      }
      default:
      {
        /*
         * Handle all other characters (non special control).
         */
        /*
         * If character is non printable then issue warning and
         * redisplay line.
         */
#if !DUNE_BCM
        if (!isgraph((unsigned int)in_char))
        {
          char
            err_msg[80] ;
          /*
           * Enter if input character is not printable. White
           * space, like \0 or \t, are also illegal here.
           * Issue warning and return to exactly same state
           * and place.
           */
          sal_sprintf(
              err_msg,
              "\r\n"
              "*** Illegal input character (0x%02X)\r\n",
              (unsigned int)in_char) ;
          msg_and_redisplay(
              err_msg,
              current_line) ;
          inc_cli_err_counter() ;
          break ;
        }
#endif
        if (in_char == SPECIAL_DOLLAR)
        {
          /*
           * In 'quotation_mark_entered' state, this character should
           * be converted back to BLANK_SPACE.
           */
          if (current_line->quotation_mark_entered)
          {
            in_char = BLANK_SPACE ;
          }
          else
          {
            char
              err_msg[80] ;
            /*
             * Enter if input character is not legal
             * for this CLI.
             * Example: '$' is used for naming numeric
             *          values. See ui_rom_defi.h
             * Issue warning and return to exactly same state
             * and place.
             */
            sal_sprintf(
                err_msg,
                "\r\n"
                "*** Illegal input character (%c)\r\n",
                in_char) ;
            msg_and_redisplay(
                err_msg,
                current_line) ;
            inc_cli_err_counter() ;
            break ;
          }
        }
        /*
         * First, echo the character.
         */
        /*
         * Actually send to screen only if echo mode is on.
         */
        if (is_echo_mode_on())
        {
          send_char_to_screen(in_char) ;
        }
        if ((current_line->num_loaded +
                strlen(current_line->prompt_string) >=
                          current_line->max_chars_on_line))
        {
          /*
           * Enter if there is no more place on the line.
           * Issue warning and return to exactly same state
           * and place.
           */
          msg_and_redisplay(
              "\r\n"
              "*** Too many characters on this line: Take some out.\r\n",
              current_line) ;
          inc_cli_err_counter() ;
          break ;
        }
        switch (current_line->proc_state)
        {
          case EXPECTING_ENTER:
          {
            msg_and_redisplay(
              "<CR>",
              current_line) ;
            inc_cli_err_counter() ;
            break ;
          }
          case EXPECTING_SUBJECT:
          case RECEIVING_SUBJECT:
          {
            int
              num_subjs ;
            unsigned int
              match_index,
              full_match,
              common_len ;
            char
              *subject_name ;
            current_line->
              line_buf[current_line->line_buf_index] = in_char ;
            current_line->line_buf_index++ ;
            current_line->num_loaded++ ;
            /*
             * Redundant for 'subject' but consistent with
             * other cases.
             */
            current_line->num_loaded_this_field++ ;
            /*
             * Since this is not 'insert' mode, 'num_loaded' contains
             * the number of characters in current subject.
             */
            /*
             * Check whether there is a subject starting with
             * these letters.
             */
            num_subjs =
              search_params(
                  &current_line->line_buf[
                        current_line->start_index_this_field],
                  current_line->num_loaded_this_field,
                  TRUE,
                  0,&match_index,&full_match,
                  &subject_name,&common_len,
                  (char *)&Subjects_list[0],
                  get_num_subjs(),
                  sizeof(Subjects_list[0]),
                  OFFSETOF(SUBJECT,subj_name),found_names_list,
                  FALSE,0,0,
                  (char **)0,0,0,0
                  ) ;
            if (num_subjs == 0)
            {
              char
                err_msg[160+MAX_SIZE_OF_SYMBOL] ;
              char
                tmp_str[MAX_SIZE_OF_SYMBOL] ;
              memcpy(tmp_str,&current_line->line_buf[
                        current_line->start_index_this_field],
                          current_line->num_loaded_this_field) ;
              tmp_str[current_line->num_loaded_this_field] = 0 ;
              /*
               * Erase last typed in character.
               */
              current_line->line_buf_index-- ;
              current_line->num_loaded-- ;
              current_line->num_loaded_this_field-- ;
              sal_sprintf(err_msg,
                "\r\n\n"
                "*** Expecting SUBJECT. No subject,"
                " starting with \'%s\', was found.\r\n\n"
                "List of subjects in system:",tmp_str) ;
              send_string_to_screen(err_msg,FALSE) ;
              msg_and_redisplay(
                  get_subj_list_ptr(),
                  current_line) ;
              inc_cli_err_counter() ;
            }
            else
            {
              /*
               * At least one subject, starting with these letters,
               * has been found.
               */
              current_line->proc_state = RECEIVING_SUBJECT ;
            }
            break ;
          }
          case EXPECTING_PARAM_STRING:
          case RECEIVING_PARAM_STRING:
          {
            int
              num_params ;
            unsigned int
              current_ordinal,
              match_index,
              full_match,
              common_len ;
            char
              *param_name ;
            unsigned
              long
                current_mutex[NUM_ELEMENTS_MUTEX_CONTROL] ;
            current_line->
              line_buf[current_line->line_buf_index] = in_char ;
            current_line->line_buf_index++ ;
            current_line->num_loaded++ ;
            current_line->num_loaded_this_field++ ;
            /*
             * Since this is not 'insert' mode, 'num_loaded' contains
             * the number of characters in current subject.
             */
            /*
             * Check whether there is a subject starting with
             * these letters.
             */
            get_current_mutex_control(current_line,current_mutex) ;
            current_ordinal =
              get_current_ordinal(current_line,current_mutex,0) ;
            num_params =
              search_params(
                  &current_line->line_buf[
                        current_line->start_index_this_field],
                  current_line->num_loaded_this_field,
                  TRUE,
                  0,&match_index,&full_match,
                  &param_name,&common_len,
                  (char *)(current_line->line_subject->allowed_params),
                  current_line->line_subject->num_allowed_params,
                  sizeof(*(current_line->line_subject->allowed_params)),
                  OFFSETOF(PARAM,par_name),found_names_list,TRUE,
                  current_mutex,
                  OFFSETOF(PARAM,mutex_control),
                  (char **)&current_line->parameters[0],
                  current_line->num_param_names,
                  current_ordinal,
                  OFFSETOF(PARAM,ordinal)
                  ) ;
            if (num_params == 0)
            {
              char
                err_msg[160+MAX_SIZE_OF_SYMBOL] ;
              char
                tmp_str[MAX_SIZE_OF_SYMBOL] ;
              memcpy(tmp_str,&current_line->line_buf[
                        current_line->start_index_this_field],
                          current_line->num_loaded_this_field) ;
              tmp_str[current_line->num_loaded_this_field] = 0 ;
              /*
               * Erase last typed in character.
               */
              current_line->line_buf_index-- ;
              current_line->num_loaded-- ;
              current_line->num_loaded_this_field-- ;
              sal_sprintf(err_msg,
                "\r\n\n"
                "*** Expecting PARAMETER. No parameter, starting with \'%s\',"
                " was found.\r\n\n"
                "List of acceptable parameters for this subject:",tmp_str) ;
              inc_cli_err_counter() ;
              send_string_to_screen(err_msg,FALSE) ;
              search_params(
                  "",0,
                  TRUE,
                  0,&match_index,&full_match,
                  &param_name,&common_len,
                  (char *)(current_line->line_subject->allowed_params),
                  current_line->line_subject->num_allowed_params,
                  sizeof(*(current_line->line_subject->allowed_params)),
                  OFFSETOF(PARAM,par_name),found_names_list,
                  TRUE,
                  current_mutex,
                  OFFSETOF(PARAM,mutex_control),
                  (char **)&current_line->parameters[0],
                  current_line->num_param_names,
                  current_ordinal,
                  OFFSETOF(PARAM,ordinal)
                  ) ;
              msg_and_redisplay(
                  found_names_list,
                  current_line) ;
            }
            else
            {
              /*
               * At least one parameter, starting with these letters,
               * has been found.
               * The state must now be RECEIVING_PARAM_STRING.
               */
              current_line->proc_state = RECEIVING_PARAM_STRING ;
            }
            break ;
          }
          case (EXPECTING_PARAM_VAL | EXPECTING_PARAM_STRING):
          {
            if (current_line->scheduled_num_char == num_chars_handled)
            {
              /*
               * Enter if this is an identifying character to
               * select either new parameter or just a value.
               */
              if (in_char == '1')
              {
                PARAM_VAL_PAIR
                  *param_val_pair ;
                param_val_pair =
                  &current_line->param_val_pair[current_line->param_index] ;
                param_val_pair->in_val_index = 0 ;
                param_val_pair->param_value.val_type = VAL_NONE ;
                current_line->param_index++ ;
                /*
                 * Redundant but safe.
                 */
                param_val_pair =
                  &current_line->param_val_pair[current_line->param_index] ;
                param_val_pair->par_name = "" ;
                param_val_pair->in_val_index = 0 ;
                param_val_pair->param_value.val_type = VAL_NONE ;
                current_line->proc_state = EXPECTING_PARAM_STRING ;
                redisplay_line(current_line) ;
                break ;
              }
              else if (in_char == '2')
              {
                current_line->proc_state = EXPECTING_PARAM_VAL ;
                redisplay_line(current_line) ;
                break ;
              }
            }
            if (current_line->num_param_values == 0)
            {
              /*
               * Note that since 'num_param_values' is '0',
               * no value has been entered for this paramater.
               */
              int
                num_params ;
              unsigned int
                match_index,
                full_match,
                common_len ;
              char
                *param_name ;
              int
                num_params_x ;
              unsigned int
                match_index_x,
                full_match_x,
                common_len_x ;
              char
                *param_name_x ;
              PARAM
                *parameter_ptr ;
              unsigned int
                num_vals ;
              current_line->
                line_buf[current_line->line_buf_index] = in_char ;
              current_line->line_buf_index++ ;
              current_line->num_loaded++ ;
              current_line->num_loaded_this_field++ ;
              parameter_ptr =
                  &current_line->line_subject->
                      allowed_params[current_line->current_param_match_index] ;
              num_vals = parameter_ptr->num_allowed_vals ;
              /*
               * Since this is not 'insert' mode, 'num_loaded' contains
               * the number of characters in current subject.
               */
              /*
               * If there are IP address values for this field then no
               * other types are expected (i.e. IP address values do not
               * mix with others).
               */
              if (parameter_ptr->num_ip_vals != 0)
              {
                char
                  *err_msg ;
                unsigned long
                  ip_addr ;
                if (!is_field_legitimate_ip(current_line,&err_msg,TRUE,&ip_addr))
                {
                  /*
                   * Something is wrong with this IP field. Clear
                   * last character.
                   */
                  /*
                   * Erase last typed in character.
                   */
                  current_line->line_buf_index-- ;
                  current_line->num_loaded-- ;
                  current_line->num_loaded_this_field-- ;
                  send_array_to_screen("\r\n",2) ;
                  msg_and_redisplay(err_msg,current_line) ;
                }
                else
                {
                  /*
                   * Start receiving IP address value.
                   * The state must now be RECEIVING_PARAM_VAL.
                   */
                  current_line->proc_state = RECEIVING_PARAM_VAL ;
                }
              }
              /*
               * If there are free text values for this field then no
               * other types are expected (i.e. free text does not
               * mix with numeric of symbolic).
               */
              else if (parameter_ptr->num_text_vals != 0)
              {
                if (in_char == QUOTATION_MARK)
                {
                  /*
                   * Start receiving free text value.
                   * The state must now be RECEIVING_PARAM_VAL.
                   */
                  current_line->proc_state = RECEIVING_PARAM_VAL ;
                  current_line->quotation_mark_entered = TRUE ;
                }
                else if (is_char_legitimate_text(in_char))
                {
                  /*
                   * Start receiving free text value.
                   * The state must now be RECEIVING_PARAM_VAL.
                   */
                  current_line->proc_state = RECEIVING_PARAM_VAL ;
                }
                else
                {
                  /*
                   * Enter if this is not an acceptable character
                   * for free text.
                   */
                  const char
                    *err_msg ;
                  /*
                   * Erase last typed in character.
                   */
                  current_line->line_buf_index-- ;
                  current_line->num_loaded-- ;
                  current_line->num_loaded_this_field-- ;
                  err_msg =
                    "\r\n\n"
                    "*** This is not an acceptable free text character\r\n"
                    "    (alphanumeric and underline only)...\r\n" ;
                  inc_cli_err_counter() ;
                  msg_and_redisplay(
                      err_msg,
                      current_line) ;
                }
              }
              else
              {
                if (is_char_numeric_starter(in_char))
                {
                  /*
                   * This is the beginning of a numeric value field.
                   */
                  if (parameter_ptr->num_numeric_vals == 0)
                  {
                    /*
                     * Enter if there are no numeric values for
                     * this parameter.
                     */
                    const char
                      *err_msg ;
                    /*
                     * Erase last typed in character.
                     */
                    current_line->line_buf_index-- ;
                    current_line->num_loaded-- ;
                    current_line->num_loaded_this_field-- ;
                    err_msg =
                      "\r\n\n"
                      "*** No numeric values allowed for this parameter.\r\n\n"
                      "List of values for this parameter:" ;
                    inc_cli_err_counter() ;
                    send_string_to_screen(err_msg,FALSE) ;
                    msg_and_redisplay(
                        parameter_ptr->vals_detailed_help,
                        current_line) ;
                  }
                  else
                  {
                    /*
                     * At least one numeric parameter has been found.
                     * The state must now be RECEIVING_PARAM_VAL.
                     */
                    current_line->proc_state = RECEIVING_PARAM_VAL ;
                  }
                }
                else
                {
                  /*
                   * This is the beginning of a symbol.
                   * We need to find out whether this symbol is a
                   * value for current parameter or a new parameter.
                   */
                  /*
                   * Check whether there is a symbolic field in the
                   * list of values attached to the parameter
                   * currently active.
                   */
                  unsigned
                    long
                      current_mutex[NUM_ELEMENTS_MUTEX_CONTROL] ;
                  unsigned
                    int
                      current_ordinal ;
                  num_params =
                    search_params(
                        &current_line->line_buf[
                            current_line->start_index_this_field],
                        current_line->num_loaded_this_field,
                        TRUE,
                        0,&match_index,&full_match,
                        &param_name,&common_len,
                        (char *)(parameter_ptr->allowed_vals),
                        num_vals,
                        sizeof(*(parameter_ptr->allowed_vals)),
                        OFFSETOF(PARAM_VAL_RULES,symb_val),
                        found_names_list,
                        FALSE,
                        0,0,
                        (char **)0,0,0,0
                        ) ;
                  get_current_mutex_control(current_line,current_mutex) ;
                  current_ordinal =
                    get_current_ordinal(current_line,current_mutex,0) ;
                  num_params_x =
                    search_params(
                        &current_line->line_buf[
                              current_line->start_index_this_field],
                        current_line->num_loaded_this_field,
                        TRUE,
                        0,&match_index_x,&full_match_x,
                        &param_name_x,&common_len_x,
                        (char *)(current_line->line_subject->allowed_params),
                        current_line->line_subject->num_allowed_params,
                        sizeof(*(current_line->line_subject->allowed_params)),
                        OFFSETOF(PARAM,par_name),found_names_list,
                        TRUE,
                        current_mutex,
                        OFFSETOF(PARAM,mutex_control),
                        (char **)&current_line->parameters[0],
                        current_line->num_param_names,
                        current_ordinal,
                        OFFSETOF(PARAM,ordinal)
                        ) ;
                  if ((num_params == 0) && (num_params_x == 0))
                  {
                    char
                      err_msg[160+MAX_SIZE_OF_SYMBOL] ;
                    char
                      tmp_str[MAX_SIZE_OF_SYMBOL] ;
                    memcpy(tmp_str,&current_line->line_buf[
                              current_line->start_index_this_field],
                                current_line->num_loaded_this_field) ;
                    tmp_str[current_line->num_loaded_this_field] = 0 ;
                    /*
                     * Erase last typed in character.
                     */
                    current_line->line_buf_index-- ;
                    current_line->num_loaded-- ;
                    current_line->num_loaded_this_field-- ;
                    sal_sprintf(err_msg,
                      "\r\n\n"
                      "*** No symbolic value or parameter, starting"
                      " with \'%s\', was found.\r\n\n"
                      "List of values for this parameter:",tmp_str) ;
                    inc_cli_err_counter() ;
                    send_string_to_screen(err_msg,TRUE) ;
                    send_string_to_screen(parameter_ptr->vals_detailed_help,TRUE) ;
                    sal_sprintf(err_msg,
                      "\r\n"
                      "List of parameters for this subject:") ;
                    send_string_to_screen(err_msg,FALSE) ;
                    msg_and_redisplay(
                        current_line->line_subject->params_ascii_list,
                        current_line) ;
                  }
                  else
                  {
                    /*
                     * Enter if either a symbolic value or a parameter
                     * has been found (or both!).
                     */
                    if ((num_params > 0) && (num_params_x == 0))
                    {
                      /*
                       * At least one symbolic value has been found
                       * while no parameter has been found.
                       * The state must now be RECEIVING_PARAM_VAL.
                       */
                      current_line->proc_state = RECEIVING_PARAM_VAL ;
                    }
                    else if ((num_params == 0) && (num_params_x > 0))
                    {
                      /*
                       * At least one parameter has been found
                       * while no symbolic value has been found.
                       * The state must now be RECEIVING_PARAM_STRING.
                       * Also, we need to advance to the next
                       * entry in 'param_val_pairs' array and do
                       * all 'blank space' actions which have been
                       * posponed because of the fuzzy state.
                       */
                      PARAM_VAL_PAIR
                        *param_val_pair ;
                      current_line->proc_state = RECEIVING_PARAM_STRING ;
                      param_val_pair =
                        &current_line->param_val_pair[current_line->param_index] ;
                      param_val_pair->in_val_index = 0 ;
                      param_val_pair->param_value.val_type = VAL_NONE ;
                      current_line->param_index++ ;
                      current_line->param_string_entered = FALSE ;
                      /*
                       * Redundant but safe.
                       */
                      param_val_pair =
                        &current_line->param_val_pair[current_line->param_index] ;
                      param_val_pair->par_name = "" ;
                      param_val_pair->in_val_index = 0 ;
                      param_val_pair->param_value.val_type = VAL_NONE ;
                    }
                    else
                    {
#if INCLUDE_ENHANCED_CLI
/* { */
                      /*
                       * At least one parameter has been found
                       * and also at least one symbolic value has been found.
                       * Request the user to be specific by typing a
                       * special character
                       */
                     char
                        err_msg[500+MAX_SIZE_OF_SYMBOL] ;
                      /*
                       * Erase last typed in character.
                       */
                      current_line->line_buf_index-- ;
                      current_line->num_loaded-- ;
                      current_line->num_loaded_this_field-- ;
                      current_line->scheduled_num_char = num_chars_handled + 1 ;
                      sal_sprintf(err_msg,
                "\r\n\n"
                "*** The system encountered ambiguity between another parameter\r\n"
                "    name and a symbolic value for this parameter (\'%s\').\r\n"
                "    If you wish to start a new parameter, hit \'1\' followed\r\n"
                "    by the new parameter\'s name\r\n"
                "    Otherwise, simply hit \'2\' followed by the symbolic value.\r\n",
                                parameter_ptr->par_name) ;
                      inc_cli_err_counter() ;
                      msg_and_redisplay(
                          err_msg,
                          current_line) ;
/* } */
#else
/* { */
                      /*
                       * At least one parameter has been found
                       * and also at least one symbolic value has been found.
                       * For this version, prefer the symbolic value.
                       */
                      current_line->proc_state = RECEIVING_PARAM_VAL ;
/* } */
#endif
                    }
                  }
                }
              }
            }
            else
            {
              /*
               * Enter if 'num_param_values' is non zero:
               * Some values have been entered for this paramater.
               * This means that either numeric values can be
               * expected or a parameter string. Symbolic values are not
               * acceptable here. Also, since the first numeric
               * value has been entered, this parameter accepts
               * numeric values and there is no need to recheck.
               */
              int
                num_params_x ;
              unsigned int
                match_index_x,
                full_match_x,
                common_len_x ;
              char
                *param_name_x ;
              PARAM
                *parameter_ptr ;
              current_line->
                line_buf[current_line->line_buf_index] = in_char ;
              current_line->line_buf_index++ ;
              current_line->num_loaded++ ;
              current_line->num_loaded_this_field++ ;
              parameter_ptr =
                    &current_line->line_subject->
                        allowed_params[current_line->current_param_match_index] ;
              /*
               * Since this is not 'insert' mode, 'num_loaded' contains
               * the number of characters in current subject.
               */
              if (is_char_numeric_starter(in_char))
              {
                /*
                 * This is the beginning of a numeric value field.
                 */
                /*
                 * No need to heck whether there is a numeric
                 * field in the list of values attached to the parameter
                 * currently active.
                 */
                /*
                 * Issue a warning if this is the last value allowed for
                 * this parameter and more than one value is allowed.
                 */
                unsigned
                  long
                    max_num_repeated ;
                max_num_repeated =
                  parameter_ptr->allowed_vals[parameter_ptr->numeric_index].
                                val_descriptor.val_num_descriptor.max_num_repeated ;
                if (max_num_repeated > 1)
                {
                  if (current_line->num_param_values >= (max_num_repeated - 1))
                  {
                  }
                }
                current_line->proc_state = RECEIVING_PARAM_VAL ;
              }
              else
              {
                /*
                 * This is the beginning of a symbol.
                 */
                /*
                 * Check whether there is a matching parameter
                 * in the list of values attached to the subject
                 * currently active.
                 */
                unsigned
                  long
                    current_mutex[NUM_ELEMENTS_MUTEX_CONTROL] ;
                unsigned
                  int
                    current_ordinal ;
                get_current_mutex_control(current_line,current_mutex) ;
                current_ordinal =
                  get_current_ordinal(current_line,current_mutex,1) ;
                num_params_x =
                  search_params(
                      &current_line->line_buf[
                            current_line->start_index_this_field],
                      current_line->num_loaded_this_field,
                      TRUE,
                      0,&match_index_x,&full_match_x,
                      &param_name_x,&common_len_x,
                      (char *)(current_line->line_subject->allowed_params),
                      current_line->line_subject->num_allowed_params,
                      sizeof(*(current_line->line_subject->allowed_params)),
                      OFFSETOF(PARAM,par_name),
                      found_names_list,
                      TRUE,
                      current_mutex,
                      OFFSETOF(PARAM,mutex_control),
                      (char **)&current_line->parameters[0],
                      current_line->num_param_names,
                      current_ordinal,
                      OFFSETOF(PARAM,ordinal)
                      ) ;
                if (num_params_x == 0)
                {
                  char
                    err_msg[160+MAX_SIZE_OF_SYMBOL] ;
                  char
                    tmp_str[MAX_SIZE_OF_SYMBOL] ;
                  memcpy(tmp_str,&current_line->line_buf[
                            current_line->start_index_this_field],
                              current_line->num_loaded_this_field) ;
                  tmp_str[current_line->num_loaded_this_field] = 0 ;
                  /*
                   * Erase last typed in character.
                   */
                  current_line->line_buf_index-- ;
                  current_line->num_loaded-- ;
                  current_line->num_loaded_this_field-- ;
                  sal_sprintf(err_msg,
                    "\r\n\n"
                    "*** No parameter, starting"
                    " with \'%s\', was found.\r\n\n"
                    "List of acceptable parameters for this subject:",tmp_str) ;
                  inc_cli_err_counter() ;
                  send_string_to_screen(err_msg,FALSE) ;
                  search_params(
                      "",
                      0,
                      TRUE,
                      0,&match_index_x,&full_match_x,
                      &param_name_x,&common_len_x,
                      (char *)(current_line->line_subject->allowed_params),
                      current_line->line_subject->num_allowed_params,
                      sizeof(*(current_line->line_subject->allowed_params)),
                      OFFSETOF(PARAM,par_name),found_names_list,TRUE,
                      current_mutex,
                      OFFSETOF(PARAM,mutex_control),
                      (char **)&current_line->parameters[0],
                      current_line->num_param_names,
                      current_ordinal,
                      OFFSETOF(PARAM,ordinal)
                      ) ;
                  msg_and_redisplay(
                      found_names_list,
                      current_line) ;
                }
                else
                {
                  /*
                   * Enter if at least one parameter has been found.
                   */
                  /*
                   * The state must now be RECEIVING_PARAM_STRING.
                   * We do not need to advance to the next
                   * entry in 'param_val_pairs' array since this started
                   * as the second value for the same parameter.
                   */
                  PARAM_VAL_PAIR
                    *param_val_pair ;
                  current_line->proc_state = RECEIVING_PARAM_STRING ;
                  param_val_pair =
                    &current_line->param_val_pair[current_line->param_index] ;
                  current_line->param_string_entered = FALSE ;
                  /*
                   * Redundant but safe.
                   */
                  param_val_pair->in_val_index = 0 ;
                  param_val_pair->param_value.val_type = VAL_NONE ;
                  param_val_pair->par_name = "" ;
                }
              }
            }
            break ;
          }
          case EXPECTING_PARAM_VAL:
          {
            int
              num_params ;
            unsigned int
              match_index,
              full_match,
              common_len ;
            char
              *param_name ;
            PARAM
              *parameter_ptr ;
            unsigned int
              num_vals ;
            current_line->
              line_buf[current_line->line_buf_index] = in_char ;
            current_line->line_buf_index++ ;
            current_line->num_loaded++ ;
            current_line->num_loaded_this_field++ ;
            parameter_ptr =
                &current_line->line_subject->
                    allowed_params[current_line->current_param_match_index] ;
            num_vals = parameter_ptr->num_allowed_vals ;
            /*
             * Since this is not 'insert' mode, 'num_loaded' contains
             * the number of characters in current subject.
             */
            /*
             * If there are ip address values for this field then no
             * other types are expected (i.e. ip address does not
             * mix with numeric, text or symbolic).
             */
            if (parameter_ptr->num_ip_vals != 0)
            {
              char
                *err_msg ;
              unsigned long
                ip_addr ;
              if (is_field_legitimate_ip(current_line,&err_msg,TRUE,&ip_addr))
              {
                /*
                 * Start receiving ip address value.
                 * The state must now be RECEIVING_PARAM_VAL.
                 */
                current_line->proc_state = RECEIVING_PARAM_VAL ;
              }
              else
              {
                /*
                 * Enter if this is not an acceptable character
                 * for free text.
                 */
                /*
                 * Erase last typed in character.
                 */
                current_line->line_buf_index-- ;
                current_line->num_loaded-- ;
                current_line->num_loaded_this_field-- ;
                send_array_to_screen("\r\n",2) ;
                msg_and_redisplay(
                    err_msg,
                    current_line) ;
              }
            }
            /*
             * If there are free text values for this field then no
             * other types are expected (i.e. free text does not
             * mix with numeric, ip or symbolic).
             */
            else if (parameter_ptr->num_text_vals != 0)
            {
              if (in_char == QUOTATION_MARK)
              {
                /*
                 * Start receiving free text value.
                 * The state must now be RECEIVING_PARAM_VAL.
                 */
                current_line->proc_state = RECEIVING_PARAM_VAL ;
                current_line->quotation_mark_entered = TRUE ;
              }
              else if (is_char_legitimate_text(in_char))
              {
                /*
                 * Start receiving free text value.
                 * The state must now be RECEIVING_PARAM_VAL.
                 */
                current_line->proc_state = RECEIVING_PARAM_VAL ;
              }
              else
              {
                /*
                 * Enter if this is not an acceptable character
                 * for free text.
                 */
                const char
                  *err_msg ;
                /*
                 * Erase last typed in character.
                 */
                current_line->line_buf_index-- ;
                current_line->num_loaded-- ;
                current_line->num_loaded_this_field-- ;
                err_msg =
                  "\r\n\n"
                  "*** This is not an acceptable free text character\r\n"
                  "    (alphanumeric and underline only)...\r\n" ;
                inc_cli_err_counter() ;
                msg_and_redisplay(
                    err_msg,
                    current_line) ;
              }
            }
            else
            {
              if (is_char_numeric_starter(in_char))
              {
                /*
                 * This is the beginning of a numeric value field.
                 */
                /*
                 * Check whether there is a numeric field in the
                 * list of values attached to the parameter
                 * currently active.
                 */
                if (parameter_ptr->num_numeric_vals == 0)
                {
                  const char
                    *err_msg ;
                  /*
                   * Erase last typed in character.
                   */
                  current_line->line_buf_index-- ;
                  current_line->num_loaded-- ;
                  current_line->num_loaded_this_field-- ;
                  err_msg =
                    "\r\n\n"
                    "*** No numeric values allowed for this parameter.\r\n\n"
                    "List of values for this parameter:" ;
                  inc_cli_err_counter() ;
                  send_string_to_screen(err_msg,FALSE) ;
                  msg_and_redisplay(
                      parameter_ptr->vals_detailed_help,
                      current_line) ;
                }
                else
                {
                  /*
                   * At least one numeric integer value has been found.
                   * The state must now be RECEIVING_PARAM_VAL.
                   */
                  current_line->proc_state = RECEIVING_PARAM_VAL ;
                }
              }
              else
              {
                /*
                 * This is the beginning of a symbolic value.
                 */
                if (parameter_ptr->num_symbolic_vals == 0)
                {
                  const char
                    *err_msg ;
                  /*
                   * Erase last typed in character.
                   */
                  current_line->line_buf_index-- ;
                  current_line->num_loaded-- ;
                  current_line->num_loaded_this_field-- ;

                  err_msg =
                    "\r\n\n"
                    "*** This parameter accepts no symbolic values.\r\n\n"
                    "List of values for this parameter:" ;
                  inc_cli_err_counter() ;
                  send_string_to_screen(err_msg,FALSE) ;
                  msg_and_redisplay(
                      parameter_ptr->vals_detailed_help,
                      current_line) ;
                }
                else
                {
                  /*
                   * Check whether there is a symbolic field in the
                   * list of values attached to the parameter
                   * currently active.
                   */
                  num_params =
                    search_params(
                        &current_line->line_buf[
                            current_line->start_index_this_field],
                        current_line->num_loaded_this_field,
                        TRUE,
                        0,&match_index,&full_match,
                        &param_name,&common_len,
                        (char *)(parameter_ptr->allowed_vals),
                        num_vals,
                        sizeof(*(parameter_ptr->allowed_vals)),
                        OFFSETOF(PARAM_VAL_RULES,symb_val),
                        found_names_list,
                        FALSE,
                        0,0,(char **)0,0,0,0
                        ) ;
                  if (num_params == 0)
                  {
                    char
                      err_msg[160+MAX_SIZE_OF_SYMBOL] ;
                    char
                      tmp_str[MAX_SIZE_OF_SYMBOL] ;
                    memcpy(tmp_str,&current_line->line_buf[
                              current_line->start_index_this_field],
                                current_line->num_loaded_this_field) ;
                    tmp_str[current_line->num_loaded_this_field] = 0 ;
                    /*
                     * Erase last typed in character.
                     */
                    current_line->line_buf_index-- ;
                    current_line->num_loaded-- ;
                    current_line->num_loaded_this_field-- ;
                    sal_sprintf(err_msg,
                      "\r\n\n"
                      "*** No symbolic value, starting with \'%s\',"
                      " was found.\r\n\n"
                      "List of values for this parameter:",tmp_str) ;
                    inc_cli_err_counter() ;
                    send_string_to_screen(err_msg,FALSE) ;
                    msg_and_redisplay(
                        parameter_ptr->vals_detailed_help,
                        current_line) ;
                  }
                  else
                  {
                    /*
                     * At least one symbolic value has been found.
                     * The state must now be RECEIVING_PARAM_VAL.
                     */
                    current_line->proc_state = RECEIVING_PARAM_VAL ;
                  }
                }
              }
            }
            break ;
          }
          case RECEIVING_PARAM_VAL:
          {
            PARAM
              *parameter_ptr ;
            current_line->
              line_buf[current_line->line_buf_index] = in_char ;
            current_line->line_buf_index++ ;
            current_line->num_loaded++ ;
            current_line->num_loaded_this_field++ ;
            parameter_ptr =
                &current_line->line_subject->
                    allowed_params[current_line->current_param_match_index] ;
            /*
             * Since this is not 'insert' mode, 'num_loaded' contains
             * the number of characters in current subject.
             */
            /*
             * If there are ip address values for this field then no
             * other types are expected (i.e. IP address values do not
             * mix with numeric, text or symbolic).
             */
            if (parameter_ptr->num_ip_vals != 0)
            {
              char
                *err_msg ;
              unsigned long
                ip_addr ;
              if (!is_field_legitimate_ip(current_line,&err_msg,TRUE,&ip_addr))
              {
                /*
                 * Something is wrong with this IP field. Clear
                 * last character.
                 */
                /*
                 * Erase last typed in character.
                 */
                current_line->line_buf_index-- ;
                current_line->num_loaded-- ;
                current_line->num_loaded_this_field-- ;
                send_array_to_screen("\r\n",2) ;
                msg_and_redisplay(err_msg,current_line) ;
              }
              else
              {
                /*
                 * This text field is OK. Prepare for next character.
                 */
              }
            }
            /*
             * If there are free text values for this field then no
             * other types are expected (i.e. free text does not
             * mix with numeric, ip or symbolic).
             */
            else if (parameter_ptr->num_text_vals != 0)
            {
              char
                *err_msg ;
              if ((in_char == QUOTATION_MARK) && current_line->quotation_mark_entered)
              {
                /*
                 * A quotation mark is legitimate only if it is the
                 * second quotation mark in this field in which
                 * case it is taken to indicate end of text field.
                 */
                current_line->quotation_mark_entered = FALSE ;
                if ((current_line->num_loaded +
                      strlen(current_line->prompt_string) >=
                                current_line->max_chars_on_line))
                {
                  /*
                   * Enter if there is no more place on the line.
                   * Only carriage return can now be entered.
                   */
                  current_line->proc_state = EXPECTING_ENTER ;
                }
                else
                {
                  /*
                   * If there is place on this line then consider the
                   * second quotation as two characters: Quotation mark
                   * followed by space.
                   * Add space and behave accordingly:
                   * Move to next parameter (or to next parameter value).
                   */
                  err = space_receiving_param_val(current_line,FALSE) ;
                  if (err)
                  {
                   /*
                    * We should not display again -- 'space_receiving_param_val' is doing the
                    *  rediplay.
                    *
                    redisplay_line(current_line) ;
                    */
                  }
                }
              }
              else if (!is_field_legitimate_text(current_line,&err_msg,TRUE))
              {
                /*
                 * Something is wrong with this text field. Clear
                 * last character.
                 */
                /*
                 * Erase last typed in character.
                 */
                current_line->line_buf_index-- ;
                current_line->num_loaded-- ;
                current_line->num_loaded_this_field-- ;
                send_array_to_screen("\r\n",2) ;
                msg_and_redisplay(err_msg,current_line) ;
              }
              else
              {
                /*
                 * This text field is OK. Prepare for next character.
                 */
              }
            }
            else if (is_last_field_numeric(current_line))
            {
              /*
               * Field is numeric.
               */
              char
                *err_msg ;
              if (!is_field_numeric_starter(current_line,&err_msg))
              {
                /*
                 * Something is wrong with this numeric field. Clear
                 * last character.
                 */
                /*
                 * Erase last typed in character.
                 */
                current_line->line_buf_index-- ;
                current_line->num_loaded-- ;
                current_line->num_loaded_this_field-- ;
                send_array_to_screen("\r\n",2) ;
                msg_and_redisplay(err_msg,current_line) ;
              }
              else
              {
                /*
                 * This numeric field is OK. Prepare for next character.
                 */
              }
            }
            else
            {
              /*
               * This is the handling of a symbolic value.
               */
              int
                num_params ;
              unsigned int
                match_index,
                full_match,
                common_len ;
              char
                *param_name ;
              PARAM
                *parameter_ptr ;
              unsigned int
                num_vals ;
              parameter_ptr =
                  &current_line->line_subject->
                      allowed_params[current_line->current_param_match_index] ;
              num_vals = parameter_ptr->num_allowed_vals ;
              /*
               * Check whether there is a symbolic field in the
               * list of values attached to the parameter
               * currently active.
               */
              num_params =
                search_params(
                    &current_line->line_buf[
                        current_line->start_index_this_field],
                    current_line->num_loaded_this_field,
                    TRUE,
                    0,&match_index,&full_match,
                    &param_name,&common_len,
                    (char *)(parameter_ptr->allowed_vals),
                    num_vals,
                    sizeof(*(parameter_ptr->allowed_vals)),
                    OFFSETOF(PARAM_VAL_RULES,symb_val),
                    found_names_list,
                    FALSE,
                    0,0,(char **)0,0,0,0
                    ) ;
              if (num_params == 0)
              {
                char
                  err_msg[160+MAX_SIZE_OF_SYMBOL] ;
                char
                  tmp_str[MAX_SIZE_OF_SYMBOL] ;
                memcpy(tmp_str,&current_line->line_buf[
                          current_line->start_index_this_field],
                            current_line->num_loaded_this_field) ;
                tmp_str[current_line->num_loaded_this_field] = 0 ;
                /*
                 * Erase last typed in character.
                 */
                current_line->line_buf_index-- ;
                current_line->num_loaded-- ;
                current_line->num_loaded_this_field-- ;
                sal_sprintf(err_msg,
                  "\r\n\n"
                  "*** No symbolic value, starting with \'%s\', was found.\r\n\n"
                  "List of values for this parameter:",tmp_str) ;
                inc_cli_err_counter() ;
                send_string_to_screen(err_msg,FALSE) ;
                msg_and_redisplay(
                  (strlen(parameter_ptr->vals_detailed_help) < 500)?parameter_ptr->vals_detailed_help:parameter_ptr->param_detailed_help,
                    current_line) ;
              }
            }
            break ;
          }
          default:
          {
            char
              err_string[100] ;
            sal_sprintf(err_string,
              "Unexpected proc_state (%d)",current_line->proc_state) ;
            gen_err(FALSE,TRUE,(int)0,0,err_string,
                      proc_name,SVR_FTL,
                      HANDLE_NEXT_CHAR_ERR_06,FALSE,0,-1,FALSE) ;
            break ;
          }
        }
        break ;
      }
    }
  }
  else
  {
    /*
     * Here handle line mode.
     */
    switch (in_char)
    {
      case DOWN_ARROW:
      {
        /*
         * This character is not counted as standard CLI characters.
         */
        num_chars_handled-- ;
        show_next_history_line(current_line_ptr) ;
        break ;
      }
      case UP_ARROW:
      {
        /*
         * This character is not counted as standard CLI characters.
         */
        num_chars_handled-- ;
        show_prev_history_line(current_line_ptr) ;
        break ;
      }
      case DEBUG_INFO:
      {
        /*
         * Print selected debug information. Debug aid only.
         */
        /*
         * This character is not counted as standard CLI characters.
         */
        num_chars_handled-- ;
        display_line_info(current_line) ;
        redisplay_line(current_line) ;
        break ;
      }
      case BACK_SPACE:
      case DEL_KEY:
      {
        if (current_line->line_buf_index == 0)
        {
          /*
           * Enter if cursor is at the beginning of the line and
           * user is trying top delete to the left. Impossible.
           */
          msg_and_redisplay(
              "\r\n"
              "*** Can not back space: Beginning of line...\r\n",
              current_line) ;
          inc_cli_err_counter() ;
          break ;
        }
        current_line->line_buf_index-- ;
        current_line->num_loaded-- ;
        back_space_line(current_line) ;
        break ;
      }
      case CARRIAGE_RETURN:
      case LINE_FEED:
      {
        /*
         * Here process input line and display prompt on next
         * clean line
         */
        CURRENT_LINE
          *line_to_process ;
        if (current_line->num_loaded== 0)
        {
          /*
           * If no characters have been loaded then just print
           * another clean line, ready for input.
           */
          send_array_to_screen("\r\n",2) ;
          redisplay_line(current_line) ;
          break ;
        }
        /*
         * Save pointer to current line (which is the line
         * to process) in 'line_to_process' and, then, store
         * 'current_line' in 'history' fifo and replace
         * it with a fresh pointer.
         * 'History' commands do not go into fifo.
         */
        line_to_process = current_line ;
        soc_sand_os_task_lock() ;
        put_history_fifo(current_line_ptr) ;
        current_line = *current_line_ptr ;
        soc_sand_os_task_unlock() ;
        /*
         * Make sure this task is not deleted half way through.
         * This is important for handling of the chip set
         * and also for handling of dynamic memory (e.g. history
         * fifo).
         */
        process_line(line_to_process,current_line_ptr) ;
        /*
         * Display prompt and clear line.
         */
        current_line = *current_line_ptr ;
#if SHOW_PROMPT_LINE_AFTER_ERROR_DURING_LINE_MODE
        redisplay_line(current_line) ;
#else
        clear_n_redisplay_line(current_line) ;
#endif
        break ;
      }
      case ESC:
      case CLEAR_TYPE_AHEAD:
      {
        /*
         * Indicate to user that periodic display has been stopped
         * and type ahead buffer has been flushed.
         * Action has been done by calling task.
         */
        msg_and_redisplay(
            "\r\n"
            "==> Periodic display stopped (and type ahead buffer manually flushed).\r\n",
            current_line) ;
        clear_n_redisplay_line(current_line) ;
        break ;
      }
      case CLEAR_LINE:
      {
        /*
         * Clean line of all input
         */
        /*
         * Display clear line with prompt.
         */
        clear_n_redisplay_line(current_line) ;
        break ;
      }
      default:
      {
#if !DUNE_BCM
        if (
            (in_char != ' ') &&
            (!isgraph((unsigned int)in_char))
          )
        {
          char
            err_msg[80] ;
          /*
           * Enter if input character is not printable. White
           * space, like \0 or \t, are also illegal here.
           * Issue warning and return to exactly same state
           * and place.
           */
          sal_sprintf(
              err_msg,
              "\r\n"
              "*** Illegal input character (0x%02X)\r\n",
              (unsigned int)in_char) ;
          inc_cli_err_counter() ;
          msg_and_redisplay(
              err_msg,
              current_line) ;
          break ;
        }
#endif
        /*
         * Echo the character.
         */
        /*
         * Actually send to screen only if echo mode is on.
         */
        if (is_echo_mode_on())
        {
          send_char_to_screen(in_char) ;
        }
        if ((current_line->num_loaded +
                strlen(current_line->prompt_string) >=
                          current_line->max_chars_on_line))
        {
          /*
           * Enter if there is no more place on the line.
           * Issue warning and return to exactly same state
           * and place.
           */
          msg_and_redisplay(
              "\r\n"
              "*** Too many characters on this line: Take some out.\r\n",
              current_line) ;
          inc_cli_err_counter() ;
          break ;
        }
        current_line->line_buf[current_line->line_buf_index] = in_char ;
        current_line->line_buf_index++ ;
        current_line->num_loaded++ ;
        break ;
      }
    }
  }
exit:
  return ;
}
/*****************************************************
*NAME
*  display_cli_prompt
*TYPE: PROC
*DATE: 08/APR/2003
*FUNCTION:
*  Disply CLI prompt. To be used by external printing
*  tasks that wish to end by leaving CLI prompt on the
*  screen.
*CALLING SEQUENCE:
*  display_cli_prompt()
*INPUT:
*  SOC_SAND_DIRECT:
*    None.
*  SOC_SAND_INDIRECT:
*    Contents of current_line (probably empty).
*OUTPUT:
*  SOC_SAND_DIRECT:
*    None.
*  SOC_SAND_INDIRECT:
*    Updated screen.
*REMARKS:
*  NONE.
*SEE ALSO:
*****************************************************/
void
  display_cli_prompt(
    void
  )
{
  handle_next_char(&Current_line_ptr,CARRIAGE_RETURN) ;
  return ;
}

/*****************************************************
*NAME
*  sys_getch
*TYPE:
*  PROCEDURE
*DATE:
*  24/MARCH/2002
*FUNCTION:
*  Wait for the next character on standard input.
*  Equivalent to 'getch' but adapted to this system.
*CALLING SEQUENCE:
*  sys_getch()
*INPUT:
*  SOC_SAND_DIRECT:
*    None.
*  SOC_SAND_INDIRECT:
*    This system's IO spec.
*OUTPUT:
*  SOC_SAND_DIRECT:
*    char in_char -
*      Detected input character.
*  SOC_SAND_INDIRECT:
*    None.
*REMARKS:
*  NONE.
*SEE ALSO:
*****************************************************/
#if !DUNE_BCM
#ifdef __VXWORKS__
/* { */
 char
  sys_getch(
    void
  )
{
  char
    *proc_name = "sys_getch" ;
  char
    ret ;
  STATUS
    status ;
  int
    in_char_count,
    nof_lines=0,
    user_fd,
    unread_count ;
  static
    unsigned
      long
        cli_last_delay_time = 0 ;
  unsigned
    long
      current_time ;
  /*
   * In 'file input mode', read characters from assigned file,
   * which is assumed to have been opened.
   */
  if (is_file_input_mode())
  {
    user_fd = get_cli_input_fd() ;
  }
  else
  {
    user_fd = get_user_if_file_descriptor() ;
  }
  while (TRUE)
  {
    status = ioctl(user_fd,FIONREAD,(int)&unread_count) ;
    if ((status == ERROR) || (unread_count != 0))
    {
      /*
       * If, after getting semaphore, the pipe is empty then
       * just keep cycling: Data has been taken by some other task.
       * This may happen when other task is in 'print_services'
       * mode (see start_print_services()).
       */
      take_screen_semaphore() ;
      status = ioctl(user_fd,FIONREAD,(int)&unread_count) ;
      if (unread_count == 0)
      {
        give_screen_semaphore() ;
        continue ;
      }
      in_char_count = read(user_fd,&ret,1) ;
      give_screen_semaphore() ;
      gen_err(TRUE,TRUE,(int)in_char_count,ERROR,
                    "read - ",proc_name,
                        SVR_FTL,SYS_GETCH_ERR_01,FALSE,0,-1,FALSE) ;
      if (is_file_input_mode())
      {
        /*
         * In 'file input mode', delay the system every so many cr/lf
         * (using fixed time delay) to avoid watchdog expiry.
         */
        if ((ret == '\r') || (ret == '\n'))
        {
          nof_lines++;
          /*
           * Get current time in units of 10ms.
           */
          current_time = get_watchdog_time(TRUE) ;
          if(
             ((current_time - cli_last_delay_time) > NUM_10MS_BETWEEN_DELAY) ||
             (nof_lines > 10)
            )
          {
            d_taskDelay(NUM_TICKS_IN_CLI_DELAY) ;
            cli_last_delay_time = current_time ;
            nof_lines = 0;
          }
        }
      }
      break ;
    }
    else
    {
      /*
       * If there are no characters to read then react,
       * depending on current mode:
       * In 'file input mode', this indicates end of file has been reached,
       *   so close the file and get out of this mode.
       * In 'standard', this indicates there is no input from the user,
       *   just sleep a little while and poll again.
       */
      if (is_file_input_mode())
      {
        close_cli_input_fd() ;
        clear_file_input_mode() ;
        user_fd = get_user_if_file_descriptor() ;
        if(Activating_file)
        {
          Activating_file=FALSE;
          break;
        }
      }
      else
      {
        d_taskDelay(4) ;
      }
    }
  }
  return (ret) ;
}
/* } */
#elif defined(__DUNE_SSF__)
/* { */
char
  sys_getch(
    void
  )
{
  char
     c = '\0';

  while (TRUE)
  {
    if(S_ui_proc_continue == FALSE)
    {
      c = '\0';
      break;
    }

    if(get_char(&Ui_char_queue, &c))
    {
      if (c != '\0')
      {
        break;
      }
    }

    sal_msleep(0.1);
  }

  return c;
}
/* } */
#endif

#else
/* { */
char
  sys_getch(
    void
  )
{
  char
      in_char ;
    unsigned long
      in_char_long=0;
    in_char = d_getch() ;
    in_char_long = in_char & 0xFF;
    if(in_char_long == 0xE0 || in_char == 0 || in_char == 0x5b)
    {
      in_char = (char)d_getch() ;
    }

    return in_char;
 }
/* } */
#endif

#if !DUNE_BCM
#if !(defined(LINUX) || defined(UNIX))
/*****************************************************
*NAME
*  sys_flush_getch
*TYPE:
*  PROCEDURE
*DATE:
*  06/APRIL/2003
*FUNCTION:
*  Flush standard input. Only good when history
* keys are not important.
*CALLING SEQUENCE:
*  sys_flush_getch()
*INPUT:
*  SOC_SAND_DIRECT:
*    None.
*  SOC_SAND_INDIRECT:
*    This system's IO spec.
*OUTPUT:
*  SOC_SAND_DIRECT:
*    None.
*  SOC_SAND_INDIRECT:
*    Flushed pipe.
*REMARKS:
*  NONE.
*SEE ALSO:
*****************************************************/
void
  sys_flush_getch(
    void
  )
{
  int
    user_fd ;
  char
    *proc_name ;
  proc_name = "sys_flush_getch" ;
  /*
   * In 'file input mode', read characters from assigned file,
   * which is assumed to have been opened.
   */
  if (is_file_input_mode())
  {
    user_fd = get_cli_input_fd() ;
  }
  else
  {
    user_fd = get_user_if_file_descriptor() ;
  }
  d_ioctl(user_fd,FIOFLUSH,0) ;
  return ;
}

static unsigned int
  Startup_from_file_flag = 0;

/*} LINUX */
#endif
#endif

/*****************************************************
*NAME
*  ui_proc
*TYPE: TASK
*DATE: 16/JAN/2002
*FUNCTION:
*  User interface main task.
*CALLING SEQUENCE:
*  ui_proc()
*INPUT:
*  SOC_SAND_DIRECT:
*    None.
*  SOC_SAND_INDIRECT:
*    Various globals. Can not be specific here.
*OUTPUT:
*  SOC_SAND_DIRECT:
*    None.
*  SOC_SAND_INDIRECT:
*    User interface task active.
*REMARKS:
*  NONE.
*SEE ALSO:
*****************************************************/
void
  ui_proc(
    void
  )
{
#if !DUNE_BCM
#if !(defined(LINUX) || defined(UNIX))
/* { */
  unsigned char run_cli_script_flag;
/*} LINUX */
#endif
#endif

  {
    /*
     * Allocate memory and initialize history fifo.
     */
    init_history_fifo() ;
    Current_line_ptr->proc_state = STARTER_STATE ;
  }
  /*
   * Abort_print flag and num_lines_printed MUST be cleared
   * every place the prompt is displayed.
   * The abort print flag is an indication that the user does
   * not wish a long print, which has been stopped at a page
   * mark, to continue.
   */
  clear_abort_print_flag() ;
  clear_num_lines_printed() ;
  /*
   * Make sure periodic flag is off, to start with.
   * Periodiv flag is an indication that continuous, repetitive
   * printing, on the same line, is active.
   */
#if !DUNE_BCM
  set_periodic_off() ;
#endif

#if !DUNE_BCM
#else
  S_ui_proc_continue = TRUE;
  S_ui_proc_finished = FALSE;
#endif

  /*
   * Display prompt.
   */
  send_string_to_screen("\r                    \r",FALSE) ;
  send_string_to_screen(Current_line_ptr->prompt_string,FALSE) ;
  /*
   * Wait a little to get dying Vx shell prompt out (if any)
   */
  send_array_to_screen("\r",1) ;
  send_string_to_screen(Current_line_ptr->prompt_string,FALSE) ;
  if (is_recovery_msg_on())
  {
    set_recovery_msg_off() ;
    send_string_to_screen(
      "\r\n\n"
      "*** System has just recovered from fatal exeption\r\n"
      "*** (e.g. illegal address access).",TRUE
    );
    /*
     * Display prompt.
     */
    send_array_to_screen("\r\n",2) ;
    send_string_to_screen(Current_line_ptr->prompt_string,FALSE) ;
  }

#if !DUNE_BCM
#if !(defined(LINUX) || defined(UNIX))
/* { */
  get_run_cli_script_at_stratup( &run_cli_script_flag );
  if(run_cli_script_flag)
  {
    if (0 == Startup_from_file_flag)
    {
      Startup_from_file_flag = 1;

      if(0 != utils_dune_load_and_rup_startup_script() )
      {
        send_string_to_screen(
          "\r\n\n*** Error running startup script", TRUE
        );

      }
    }
  }
/* } */
#endif
#endif

  /*
   * The main loop.
   */
  while (TRUE)
  {
    char
      in_char ;

#if !DUNE_BCM
#else
  /* if global flag saying UI to continue is not set - exit.
   * The flag is set to FALSE by restart_ui_task
   */
  if(S_ui_proc_continue == FALSE)
  {
    in_char = '\0';
    send_string_to_screen(&in_char,TRUE) ;
    S_ui_proc_finished = TRUE;
    return;
  }  
#endif
#if defined(__VXWORKS__) || defined(__DUNE_SSF__)
/* { */
    in_char = sys_getch() ;

/* } */
#else
   {
    unsigned long
      in_char_long=0;
    in_char = d_getch() ;
    in_char_long = in_char & 0xFF;
    if (in_char == 27) /* '[' char is the second in sequence */
    {
      in_char = d_getch();
      if (in_char == 91) {
        int in_char_tmp = d_getch();
        switch (in_char_tmp)
        {
          case 65:
            in_char = UP_ARROW;
            break;
          case 66:
            in_char = DOWN_ARROW;
            break;
          default:
            in_char = 91; /* process ESC if unknown */
        }
      }
    }
    else if(in_char_long == 0xE0 || in_char == 0)
    {
      in_char = d_getch() ;
    }
   }
#endif
#ifdef __DUNE_HRP__
/* { */
/* handle up/down arrows */
  if (in_char == 27)  /* ESC key starts arrow sequence */
  {
    in_char = d_getch();
    if (in_char == 91) /* '[' char is the second in sequence */
    {
      in_char = d_getch();
      switch (in_char)
      {
        case 65:
          in_char = UP_ARROW;
          break;
        case 66:
          in_char = DOWN_ARROW;
          break;
        default:
          in_char = 27; /* process ESC if unknown */
      }
    }
    else
    {
      in_char = 27; /* process ESC if unknown */
    }
  }
/* } */
#endif

  handle_next_char(&Current_line_ptr,(char)in_char) ;

  }
#if  defined(__DUNE_SSF__)
  S_ui_proc_finished = TRUE;
#endif
  return ;
}

void
  ui_activate_file(
    void
  )
{
  char in_char;
  Activating_file = TRUE;
  while(is_file_input_mode())
  {
    in_char = sys_getch() ;
    /*
     * Here process input characters and respond to screen.
     */
    if(Activating_file)
    {
      handle_next_char(&Current_line_ptr,in_char) ;
    }
    else
    {
      return;
    }
  }
  return;
}


#if defined(__VXWORKS__) && !defined(__DUNE_GTO_BCM_CPU__)
/* { */
/*****************************************************
*NAME
*  dummy_console
*TYPE: TASK
*DATE: 18/MARCH/2003
*FUNCTION:
*  Collector of user input from local console
*  while user interface main task is on Telnet.
*CALLING SEQUENCE:
*  dummy_console()
*INPUT:
*  SOC_SAND_DIRECT:
*    None.
*  SOC_SAND_INDIRECT:
*    Various globals. Can not be specific here.
*OUTPUT:
*  SOC_SAND_DIRECT:
*    None.
*  SOC_SAND_INDIRECT:
*    User interface on local console while Telnet
*    is active.
*REMARKS:
*  NONE.
*SEE ALSO:
*  restart_ui_task()
*****************************************************/
void
  dummy_console(
    void
  )
{
  int
    err,
    terminal_options,
    waiting_for_timeout,
    dummy_console_fd ;
  STATUS
    status ;
  char
    *msg[4],
    *write_err,
    *proc_name ;
  unsigned
    long
      poll_timeout_ticks ;
  unsigned
    int
      msg_len[4] ;
  proc_name = "dummy_console" ;
  /*
   * Get file descriptor of local terminal.
   * In this system, it is one file.
   */
  dummy_console_fd = get_console_fd() ;
  terminal_options = ioctl(dummy_console_fd,FIOGETOPTIONS,0) ;
  terminal_options &=
      ~(OPT_MON_TRAP | OPT_ECHO | OPT_ABORT | OPT_LINE | OPT_TANDEM) ;
  status = ioctl(dummy_console_fd,FIOSETOPTIONS,terminal_options) ;
  gen_err(TRUE,TRUE,(int)status,ERROR,
            "dummy_console - failed to set terminal options",
            proc_name,SVR_FTL,
            DUMMY_CONSOLE_ERR_01,FALSE,0,-1,FALSE) ;
  {
    /*
     * Calculate number of ticks to wait between pollings of
     * input queue.
     */
    unsigned
      long
        poll_timeout_10ms,
        nano_per_tick ;
    nano_per_tick = 1000000000 / CLOCKS_PER_SEC ;
    /*
     * Timeout value, in units of 10 milliseconds
     */
    poll_timeout_10ms = 10 ;
    poll_timeout_ticks = ((poll_timeout_10ms * 1000000) / (nano_per_tick / 10)) ;
  }
  msg[0] =
    "\r\n"
    "==> System is in Telnet mode. Hit \'space\' to shorten Telnet\r\n"
    "    inactivity timeout or any other key to ignore this line.\r\n"
    "    (To get the same message on out-of-band Telnet, use port 26). ====> " ;
  msg_len[0] = strlen(msg[0]) ;
  msg[1] =
    "\r\n\n"
    "==> Non-space character hit.\r\n" ;
  msg_len[1] = strlen(msg[1]) ;
  msg[2] =
    "\r\n\n"
    "==> \'space\' hit! Telnet inactivity timeout for current user shortened!!!\r\n"
    "    If current user does not hit any key for a short while. Control will\r\n"
    "    return to local console.\r\n" ;
  msg_len[2] = strlen(msg[2]) ;
  msg[3] =
    "\r\n"
    "==> \'space\' already hit and Telnet inactivity timeout\r\n"
    "    for current user already shortened!!!\r\n"
    "    If current user does not hit any key for a short while,\r\n"
    "    control will return to local console.\r\n" ;
  msg_len[3] = strlen(msg[3]) ;
  write_err = "dummy_console - failed to write to local console" ;
  err =
    d_write(dummy_console_fd,msg[0],msg_len[0],FALSE) ;
  gen_err(TRUE,TRUE,(int)err,ERROR,
            write_err,
            proc_name,SVR_FTL,
            DUMMY_CONSOLE_ERR_02,FALSE,0,-1,FALSE) ;
  /*
   * The main loop.
   */
  waiting_for_timeout = FALSE ;
  while (is_telnet_active())
  {
    int
      in_char_count ;
    char
      in_char ;
    int
      unreadCount ;
    unsigned
      char
        val ;
    /*
     * Get the next character from local console.
     */
    status = ioctl((int)dummy_console_fd,FIONREAD,(int)&unreadCount) ;
    if (unreadCount != 0)
    {
      in_char_count = read(dummy_console_fd,&in_char,1) ;
      if (!waiting_for_timeout)
      {
        if (in_char == ' ')
        {
          err =
            d_write(dummy_console_fd,msg[2],msg_len[2],FALSE) ;
          gen_err(TRUE,TRUE,(int)err,ERROR,
              write_err,
              proc_name,SVR_FTL,
              DUMMY_CONSOLE_ERR_03,FALSE,0,-1,FALSE) ;
          if (is_display_periodic_on())
          {
            val = (unsigned char)DEFAULT_TELNET_ACTIVITY_TIMEOUT_CONT ;
          }
          else
          {
            val = (unsigned char)DEFEULT_TELNET_ACTIVITY_TIMEOUT_STD;
          }
          d_printf(
            "\r\n\n"
            "==> Telnet inactivity timeout shortened by an external user....\r\n") ;
          set_telnet_activity_time_mark() ;
          set_telnet_timeout_val((unsigned long)val) ;
          waiting_for_timeout = TRUE ;
        }
        else
        {
          err =
            d_write(dummy_console_fd,msg[1],msg_len[1],FALSE) ;
          gen_err(TRUE,TRUE,(int)err,ERROR,
              write_err,
              proc_name,SVR_FTL,
              DUMMY_CONSOLE_ERR_03,FALSE,0,-1,FALSE) ;
          err =
            d_write(dummy_console_fd,msg[0],msg_len[0],FALSE) ;
          gen_err(TRUE,TRUE,(int)err,ERROR,
              write_err,
              proc_name,SVR_FTL,
              DUMMY_CONSOLE_ERR_03,FALSE,0,-1,FALSE) ;
        }
      }
      else
      {
        err =
          d_write(dummy_console_fd,msg[3],msg_len[3],FALSE) ;
        gen_err(TRUE,TRUE,(int)err,ERROR,
            write_err,
            proc_name,SVR_FTL,
            DUMMY_CONSOLE_ERR_03,FALSE,0,-1,FALSE) ;
      }
    }
    else
    {
      /*
       * No character has been found on queue.
       * wait some time and poll again
       */
      d_taskDelay(poll_timeout_ticks) ;
      continue ;
    }
  }
  return ;
}
/*
 * Object: Dummy Telnet handling
 * Dummy Telnet handles user input on TCP port 26 while
 * standard Telnet is active.
 * {
 *
 */
static
  int
    Dummy_telnet_fd = ERROR ;
static
  int
    Dummy_telnet_socket = ERROR ;
/*
 * Server's socket address
 */
static
  struct sockaddr_in
    Dummy_telnet_server_addr ;
/*
 * Client's socket address
 */
static
  struct sockaddr_in
    Dummy_telnet_client_addr ;
/*
 * Size of socket address structure
 */
static
  int
    Dummy_telnet_sock_addr_size = sizeof(struct sockaddr_in) ;
int
  get_dummy_telnet_sock_addr_size(
    void
  )
{
  return (Dummy_telnet_sock_addr_size) ;
}
struct sockaddr_in
  *get_dummy_telnet_server_addr(
    void
  )
{
  return(&Dummy_telnet_server_addr) ;
}
struct sockaddr_in
  *get_dummy_telnet_client_addr(
    void
  )
{
  return(&Dummy_telnet_client_addr) ;
}
int
  get_dummy_telnet_fd(
    void
  )
{
  return ((int)Dummy_telnet_fd) ;
}
void
  set_dummy_telnet_fd(
    int fd
  )
{
  Dummy_telnet_fd = fd ;
  return ;
}
int
  get_dummy_telnet_socket(
    void
  )
{
  return ((int)Dummy_telnet_socket) ;
}
void
  set_dummy_telnet_socket(
    int socket
  )
{
  Dummy_telnet_socket = socket ;
  return ;
}
/*
 * }
 */
/*****************************************************
*NAME
*  dummy_telnet
*TYPE: TASK
*DATE: 18/MARCH/2003
*FUNCTION:
*  Collector of user input from telnet on port 26
*  while user interface main task is on Telnet.
*CALLING SEQUENCE:
*  dummy_telnet()
*INPUT:
*  SOC_SAND_DIRECT:
*    None.
*  SOC_SAND_INDIRECT:
*    Various globals. Can not be specific here.
*OUTPUT:
*  SOC_SAND_DIRECT:
*    None.
*  SOC_SAND_INDIRECT:
*    User interface on dummy telnet while Telnet
*    is active.
*REMARKS:
*  NONE.
*SEE ALSO:
*  restart_ui_task()
*****************************************************/
void
  dummy_telnet(
    void
  )
{
  int
    err,
    waiting_for_timeout,
    dummy_telnet_fd ;
  STATUS
    status ;
  char
    *msg[4],
    *write_err,
    *proc_name ;
  unsigned
    long
      poll_timeout_ticks ;
  unsigned
    int
      activity_count,
      msg_len[4] ;
  struct sockaddr_in
    *server_addr ;
  struct sockaddr_in
    *client_addr ;
  int
    sock_addr_size,
    socket_fd ;
  proc_name = "dummy_telnet" ;
  /*
   * set up the local address
   */
  sock_addr_size = get_dummy_telnet_sock_addr_size() ;
  server_addr = get_dummy_telnet_server_addr() ;
  bzero((char *)server_addr,sock_addr_size) ;
  server_addr->sin_family = AF_INET ;
  server_addr->sin_len = (unsigned char)sock_addr_size ;
  server_addr->sin_port = htons(DUMMY_TELNET_TCP_PORT) ;
  server_addr->sin_addr.s_addr = htonl(INADDR_ANY) ;
  /*
   * create a TCP-based socket
   */
  socket_fd = socket(AF_INET,SOCK_STREAM,0) ;
  if (socket_fd == ERROR)
  {
    gen_err(FALSE,TRUE,(int)socket_fd,ERROR,
          "dummy_telnet - \'socket\' fail",
          proc_name,SVR_WRN,
          DUMMY_TELNET_ERR_01,FALSE,0,-1,FALSE) ;
    goto exit ;
  }
  set_dummy_telnet_socket(socket_fd) ;
  /*
   * bind socket to local address
   */
  err = bind(socket_fd,(struct sockaddr *)server_addr,sock_addr_size) ;
  if (err == ERROR)
  {
    gen_err(FALSE,TRUE,(int)err,ERROR,
          "dummy_telnet - \'bind\' fail",
          proc_name,SVR_WRN,
          DUMMY_TELNET_ERR_02,FALSE,0,-1,FALSE) ;
    goto exit ;
  }
  /*
   * create queue for client connection requests.
   * Allow one user only!
   */
  err = listen(socket_fd,1) ;
  if (err == ERROR)
  {
    gen_err(FALSE,TRUE,(int)err,ERROR,
          "dummy_telnet - \'listen\' fail",
          proc_name,SVR_WRN,
          DUMMY_TELNET_ERR_03,FALSE,0,-1,FALSE) ;
    goto exit ;
  }
  msg[0] =
    "\r\n"
    "==> System is in Telnet mode. Within 10 seconds, hit \'space\' to shorten\r\n"
    "    Telnet inactivity timeout or any other key to ignore this line ======> " ;
  msg_len[0] = strlen(msg[0]) ;
  msg[1] =
    "\r\n\n"
    "==> Non-space character hit.\r\n" ;
  msg_len[1] = strlen(msg[1]) ;
  msg[2] =
    "\r\n\n"
    "==> \'space\' hit! Telnet inactivity timeout for current user shortened!!!\r\n"
    "    If current user does not hit any key for a short while. Control will\r\n"
    "    return to local terminal.\r\n" ;
  msg_len[2] = strlen(msg[2]) ;
  msg[3] =
    "\r\n"
    "==> \'space\' already hit and Telnet inactivity timeout\r\n"
    "    for current user already shortened!!!\r\n"
    "    If current Telnet user does not hit any key for a short\r\n"
    "    while, control will return to local terminal.\r\n" ;
  msg_len[3] = strlen(msg[3]) ;
  write_err = "dummy_telnet - failed to write to dummy telnet" ;
  {
    /*
     * Calculate number of ticks to wait between pollings of
     * input queue.
     */
    unsigned
      long
        poll_timeout_10ms,
        nano_per_tick ;
    nano_per_tick = 1000000000 / CLOCKS_PER_SEC ;
    /*
     * Timeout value, in units of 10 milliseconds
     */
    poll_timeout_10ms = 10 ;
    poll_timeout_ticks = ((poll_timeout_10ms * 1000000) / (nano_per_tick / 10)) ;
  }
  /*
   * Now accept new connect requests and process them
   */
  waiting_for_timeout = FALSE ;
  while (is_telnet_active())
  {
    client_addr = get_dummy_telnet_client_addr() ;
    dummy_telnet_fd =
      accept(socket_fd,(struct sockaddr *)client_addr,&sock_addr_size) ;
    if (dummy_telnet_fd == ERROR)
    {
      gen_err(FALSE,TRUE,(int)dummy_telnet_fd,ERROR,
          "dummy_telnet - \'accept\' fail",
          proc_name,SVR_WRN,
          DUMMY_TELNET_ERR_04,FALSE,0,-1,FALSE) ;
      goto exit ;
    }
    set_dummy_telnet_fd(dummy_telnet_fd) ;
    err =
      d_write(dummy_telnet_fd,msg[0],msg_len[0],FALSE) ;
    gen_err(TRUE,TRUE,(int)err,ERROR,
            write_err,
            proc_name,SVR_WRN,
            DUMMY_TELNET_ERR_05,FALSE,0,-1,FALSE) ;
    /*
     * Get base time for 10 seconds timeout.
     */
    activity_count = 100 ;
    while (TRUE)
    {
      int
        in_char_count ;
      char
        in_char ;
      int
        unreadCount ;
      unsigned
        char
          val ;
      /*
       * Get the next character from local telnet.
       */
      status = ioctl((int)dummy_telnet_fd,FIONREAD,(int)&unreadCount) ;
      if (unreadCount != 0)
      {
        activity_count = 100 ;
        in_char_count = read(dummy_telnet_fd,&in_char,1) ;
        if (!waiting_for_timeout)
        {
          if (in_char == ' ')
          {
            err =
              d_write(dummy_telnet_fd,msg[2],msg_len[2],FALSE) ;
            gen_err(TRUE,TRUE,(int)err,ERROR,
                write_err,
                proc_name,SVR_WRN,
                DUMMY_TELNET_ERR_05,FALSE,0,-1,FALSE) ;
            if (is_display_periodic_on())
            {
              val = (unsigned char)DEFAULT_TELNET_ACTIVITY_TIMEOUT_CONT ;
            }
            else
            {
              val = (unsigned char)DEFEULT_TELNET_ACTIVITY_TIMEOUT_STD;
            }
            d_printf(
              "\r\n\n"
              "==> Telnet inactivity timeout shortened by an external user....\r\n") ;
            set_telnet_activity_time_mark() ;
            set_telnet_timeout_val((unsigned long)val) ;
            waiting_for_timeout = TRUE ;
          }
          else
          {
            err =
              d_write(dummy_telnet_fd,msg[1],msg_len[1],FALSE) ;
            gen_err(TRUE,TRUE,(int)err,ERROR,
                write_err,
                proc_name,SVR_WRN,
                DUMMY_TELNET_ERR_05,FALSE,0,-1,FALSE) ;
            err =
              d_write(dummy_telnet_fd,msg[0],msg_len[0],FALSE) ;
            gen_err(TRUE,TRUE,(int)err,ERROR,
                write_err,
                proc_name,SVR_WRN,
                DUMMY_TELNET_ERR_05,FALSE,0,-1,FALSE) ;
          }
        }
        else
        {
          err =
            d_write(dummy_telnet_fd,msg[3],msg_len[3],FALSE) ;
          gen_err(TRUE,TRUE,(int)err,ERROR,
              write_err,
              proc_name,SVR_WRN,
              DUMMY_TELNET_ERR_03,FALSE,0,-1,FALSE) ;
        }
      }
      else
      {
        /*
         * No character has been found on queue.
         * wait some time and poll again
         */
        activity_count-- ;
        if (activity_count <= 0)
        {
#if !DUNE_BCM
          close(dummy_telnet_fd) ;
#else
          assert(1);
#endif
          break ;
        }
        d_taskDelay(poll_timeout_ticks) ;
        continue ;
      }
    }
  }
exit:
  shutdown(get_dummy_telnet_socket(),2) ;
  return ;
}
/*****************************************************
*NAME
*  restart_ui_task
*TYPE: TASK
*DATE: 23/JAN/2002
*FUNCTION:
*  Delete ui_task (if it is alive), clear its pipe
*  and restart it. This makes sure is graciously halts
*  whatever action it was involved in.
*CALLING SEQUENCE:
*  restart_ui_task()
*INPUT:
*  SOC_SAND_DIRECT:
*    None.
*  SOC_SAND_INDIRECT:
*    Various globals. Can not be specific here.
*OUTPUT:
*  SOC_SAND_DIRECT:
*    None.
*  SOC_SAND_INDIRECT:
*    User prep task active.
*REMARKS:
*  NONE.
*SEE ALSO:
*****************************************************/
void
  restart_ui_task(
    void
  )
{
  /*
   *   IMPORTANT:
   *   The user interafce task must make the execution of
   *   command lines atomic in the sense of task deletion (i.e.
   *   It must execute command lines under 'taskSafe'). This insures
   *   that detetion here does not get us into an undefined state.
   * Before restarting the user interface task, the pipe
   * between it and this task is cleared. This means that this task
   * always starts at the beginning of a line.
   */
  int
    user_fd,
    user_if_task_id ;
  int
    dummy_console_fd,
    dummy_console_task_id,
    dummy_telnet_task_id ;
  char
    *proc_name ;
  proc_name = "restart_ui_task" ;
  /*
   * Delete dummy local console task only if it is alive.
   * This task collects user keys as hit by the user when telnet
   * is active.
   */
  if (is_task_alive(DUMMY_CONSOLE_TASK_ID))
  {
    /*
     * Enter here only if dummy local console task is alive.
     */
    dummy_console_task_id = get_task_id(DUMMY_CONSOLE_TASK_ID) ;
    d_taskDelete(dummy_console_task_id) ;
  }
  /*
   * Delete dummy Telnet task only if it is alive.
   * This task collects user keys on Telnet port 26 as hit
   * by the user when standard Telnet is active (port 23).
   */
  if (is_task_alive(DUMMY_TELNET_TASK_ID))
  {
    /*
     * Enter here only if dummy Telnet task is alive.
     */
    dummy_telnet_task_id = get_task_id(DUMMY_TELNET_TASK_ID) ;
    d_taskDelete(dummy_telnet_task_id) ;
    /*
     * Shut down associated socket (if not shut down already).
     */
    shutdown(get_dummy_telnet_socket(),2) ;
  }
  if (is_telnet_active())
  {
    /*
     * If Telnet is active then clear the pipe (just
     * to make sure) and restart
     * 1. dummy console to respond to user request for
     *    stopping standard Telnet.
     * 2. dummy Telnet to respond to user request for
     *    stopping standard Telnet.
     */
    {
      dummy_console_fd = get_console_fd() ;
      d_ioctl(dummy_console_fd,FIOFLUSH,0) ;
      /*
       * Spawn the dummy console task (which gets its input
       * from  local console.
       */
      d_taskSpawn(
          DUMMY_CONSOLE_TASK_NAME,DUMMY_CONSOLE_TASK_PRIORITY,
            0,STACK_SIZE,(FUNCPTR)dummy_console,
            0,0,0,0,0,0,0,0,0,0,DUMMY_CONSOLE_TASK_ID) ;
    }
    {
      /*
       * Spawn the dummy Telnet task (which gets its input
       * from Telnet on port 26).
       */
      d_taskSpawn(
          DUMMY_TELNET_TASK_NAME,DUMMY_TELNET_TASK_PRIORITY,
            0,STACK_SIZE,(FUNCPTR)dummy_telnet,
            0,0,0,0,0,0,0,0,0,0,DUMMY_TELNET_TASK_ID) ;
    }
  }
  /*
   * Delete user interface task only if it is alive.
   */
  if (is_task_alive(USER_INTERFACE_TASK_ID))
  {
    /*
     * Enter here only if user interface task is alive.
     */
    user_if_task_id = get_task_id(USER_INTERFACE_TASK_ID) ;
    d_taskDelete(user_if_task_id) ;
    /*
     * Clear the pipe...
     */
    user_fd = get_user_if_file_descriptor() ;
    d_ioctl(user_fd,FIOFLUSH,0) ;
    /*
     * Return memory allocated within this task (if there is any).
     */
    /*
     * Spawn the user interface task (which gets its input
     * from a special pipe, "/pipe/userIf").
     * This is a relatively high priority task but lower
     * than user_prep_task which fills in that pipe.
     */
    d_taskSpawn(
        USER_INTERFACE_TASK_NAME,USER_INTERFACE_TASK_PRIORITY,
          0,USER_INTERFACE_TASK_STACK_SIZE,(FUNCPTR)ui_proc,
          0,0,0,0,0,0,0,0,0,0,USER_INTERFACE_TASK_ID) ;
  }
  else
  {
    /*
     * User interface task does not exist. Clear the pipe (just
     * to make sure) and restart it.
     */
    user_fd = get_user_if_file_descriptor() ;
    d_ioctl(user_fd,FIOFLUSH,0) ;
    /*
     * Spawn the user interface task (which gets its input
     * from a special pipe, "/pipe/userIf").
     * This is a relatively high priority task but lower
     * than user_prep_task which fills in that pipe.
     */
    /*
     * Small delay to allow shell prompt to be printed. After that,
     * we take over the prompt.
     */
    d_taskDelay(20) ;
    d_taskSpawn(
        USER_INTERFACE_TASK_NAME,USER_INTERFACE_TASK_PRIORITY,
          0,USER_INTERFACE_TASK_STACK_SIZE,(FUNCPTR)ui_proc,
          0,0,0,0,0,0,0,0,0,0,USER_INTERFACE_TASK_ID) ;
  }
  return ;
}
/* } */
#endif /*__VXWORKS__*/

#ifdef __DUNE_SSF__
/* { */
/*****************************************************
*NAME
*  restart_ui_task
*TYPE: TASK
*DATE: 23/JAN/2002
*FUNCTION:
*  Delete ui_task (if it is alive), clear its pipe
*  and restart it. This makes sure is graciously halts
*  whatever action it was involved in.
*CALLING SEQUENCE:
*  restart_ui_task()
*INPUT:
*  SOC_SAND_DIRECT:
*    None.
*  SOC_SAND_INDIRECT:
*    Various globals. Can not be specific here.
*OUTPUT:
*  SOC_SAND_DIRECT:
*    None.
*  SOC_SAND_INDIRECT:
*    User prep task active.
*REMARKS:
*  NONE.
*SEE ALSO:
*****************************************************/
void
  restart_ui_task(
    void
  )
{

  if(S_ui_proc_finished == FALSE)
  {
    /* wait for ending processing */
    while(char_buff_is_empty(&Ui_char_queue) == FALSE)
    {
      sal_msleep(1);
    }

    /* ask ui_proc to finish */
    S_ui_proc_continue = FALSE;
    /* send \0 to ui_proc to unblock sys_getch */
    put_char(&Ui_char_queue, '\0');

    /* wait for ui_proc to finish */
    S_ui_tid = -1;
    while(S_ui_proc_finished == FALSE)
    {
      sal_msleep(1);
    }
  }


  S_ui_proc_continue = TRUE;
  S_ui_proc_finished = FALSE;

#if !DUNE_BCM
  utils_aux_input_start();
#endif

  if (Ui_char_queue_is_init)
  {
    char_buff_delete(&Ui_char_queue);
  }
  char_buff_create(UI_CHAR_BUFF_SIZE, &Ui_char_queue);
  Ui_char_queue_is_init = 1;

  /*
   * Small delay to allow shell prompt to be printed. After that,
   * we take over the prompt.
   */
  sal_msleep(0.02) ;
  S_ui_tid = d_taskSpawn(
      USER_INTERFACE_TASK_NAME,USER_INTERFACE_TASK_PRIORITY,
        0,USER_INTERFACE_TASK_STACK_SIZE,(FUNCPTR)ui_proc,
        0,0,0,0,0,0,0,0,0,0,USER_INTERFACE_TASK_ID) ;

  return;
}
/* } */
#endif /*__DUNE_SSF__*/

/*****************************************************
*NAME
*  translate_escape_seq
*TYPE: TASK
*DATE: 23/JULY/2002
*FUNCTION:
*  Check input sequence of characters collected at
*  standard IO and translate, if possible, to
*  another seuence (one character, in the most
*  common case).
*CALLING SEQUENCE:
*  translate_escape_seq(
*           in_chars_array,in_chars_count,
*           loop_on)
*INPUT:
*  SOC_SAND_DIRECT:
*    char *in_chars_array -
*      Pointer to an array of input characters (starting
*      with ESC. To be compared with supported
*      escape sequences).
*      This array also serves as output if a full
*      escape sequence has been detected and
*      translated. In this case, 'loop_on' is loaded
*      by 'false' and 'in_chars_array' is loaded
*      by the characters which are the translation
*      of the escape sequence.
*    unsigned int *in_chars_count -
*      Number of characters in 'in_chars_array'.
*      This variable also serves as output if a full
*      escape sequence has been detected and
*      translated. In this case, 'loop_on' is loaded
*      by 'false' and this variable is loaded by
*      the number of characters on the newly loaded
*      'in_chars_array'.
*    unsigned int *loop_on -
*      Flag. This procedure loads pointed variable
*      with 'true' if characters, collected so far,
*      form the begining of an escape sequence.
*      If a full escape sequence is found, this
*      variable will be loaded by 'false' and
*      both 'in_chars_array' and 'in_chars_count'
*      may be updated.
*      If this procedure decides that input sequence
*      can not be a recognized escape sequence, this
*      variable will be loaded by 'false'.
*      When 'flase' is retuned, calling procedure is
*      supposed to use whatever is in 'in_chars_array'
*      and empty it.
*  SOC_SAND_INDIRECT:
*    in_chars_array, in_chars_count.
*OUTPUT:
*  SOC_SAND_DIRECT:
*    int -
*      If non-zero then some error has been encountered.
*  SOC_SAND_INDIRECT:
*    in_chars_array, in_chars_count.
*REMARKS:
*  Since only 2 escape sequences are supported, handling
*  of interpretation of escape sequences is primitive.
*SEE ALSO:
*****************************************************/
int
  translate_escape_seq(
    char         *in_chars_array,
    unsigned int *in_chars_count,
    unsigned int *loop_on
  )
{
  int
    ret ;
  unsigned
    int
      in_count,
      full_compare,
      sequence_len ;
  static
    const char
      *up_arrow_sequence = "\x1B\x5B\x41" ;
  static
    const char
      *down_arrow_sequence = "\x1B\x5B\x42" ;
  ret = 0 ;
  *loop_on = FALSE ;
  in_count = *in_chars_count ;
  sequence_len = strlen(up_arrow_sequence) ;
  if (sequence_len <= in_count)
  {
    full_compare = TRUE ;
  }
  else
  {
    full_compare = FALSE ;
    sequence_len = in_count ;
  }
  if (memcmp(in_chars_array,up_arrow_sequence,sequence_len) == 0)
  {
    /*
     * Full or partial match for up arrow.
     */
    if (full_compare)
    {
      *in_chars_count = 1 ;
      in_chars_array[0] = UP_ARROW ;
      goto tese_exit ;
    }
    else
    {
      *loop_on = TRUE ;
    }
  }
  sequence_len = strlen(down_arrow_sequence) ;
  if (sequence_len <= in_count)
  {
    full_compare = TRUE ;
  }
  else
  {
    full_compare = FALSE ;
    sequence_len = in_count ;
  }
  if (memcmp(in_chars_array,down_arrow_sequence,sequence_len) == 0)
  {
    /*
     * Full or partial match for up arrow.
     */
    if (full_compare)
    {
      *in_chars_count = 1 ;
      in_chars_array[0] = DOWN_ARROW ;
      goto tese_exit ;
    }
    else
    {
      *loop_on = TRUE ;
    }
  }
tese_exit:
  return (ret) ;
}

#if defined(__VXWORKS__) && !defined(__DUNE_GTO_BCM_CPU__)
/* { */

/*****************************************************
*NAME
*  ui_prep_proc
*TYPE: TASK
*DATE: 16/JAN/2002
*FUNCTION:
*  Collector of user input for user interface main task.
*CALLING SEQUENCE:
*  ui_prep_proc()
*INPUT:
*  SOC_SAND_DIRECT:
*    None.
*  SOC_SAND_INDIRECT:
*    Various globals. Can not be specific here.
*OUTPUT:
*  SOC_SAND_DIRECT:
*    None.
*  SOC_SAND_INDIRECT:
*    User prep task active.
*REMARKS:
*  NONE.
*SEE ALSO:
*****************************************************/
void
  ui_prep_proc(
    void
  )
{
  int
    error_value,
    user_fd,
    standard_fd ;
  STATUS
    status ;
  char
    *proc_name ;
  unsigned
    long
      poll_timeout_ticks ;
  unsigned
    int
      ui,
      error_count,
      escape_seq_timeout_10ms,
      escape_seq_on,
      loop_on,
      in_chars_flush,
      in_chars_index,
      in_chars_len ;
  unsigned
    long
      time_now,
      escape_seq_time_last_char ;
  char
    in_chars_array[10] ;
  proc_name = "ui_prep_proc" ;
  escape_seq_time_last_char = 0;
  {
    /*
     * When this task starts, it deletes the shell and takes over.
     * Then, it restarts the shell at a lower priority, waiting
     * on a different port.
     * There may be a simpler way of doing it: Delete the shell and,
     * whenever a shell service is required, open it to carry
     * out a script from a file. See example at usrScript.c
     * However, this may pose a problem with Telnet.
     */
    int
      shell_task_id ;
    /*
     * Handle shell task only if it is alive.
     */
    shell_task_id = d_restart_shell() ;
    if (shell_task_id != ERROR)
    {
      /*
       * Make sure shell task gets its input from our
       * custom pipe.
       */
      d_ioTaskStdSet(shell_task_id,0,get_shell_file_descriptor()) ;
    }
  }
  {
    /*
     * When this task starts, it deletes the user interface task
     * and, then, it restarts it.
     *   IMPORTANT:
     *   The user interafce task must make the execution of
     *   command lines atomic in the sense of task deletion (i.e.
     *   It must execute command lines under 'taskSafe'). This insures
     *   that detetion here does not get us into an undefined state.
     * Before restarting the user interface task, the pipe
     * between it and this task is cleared. This means that this task
     * always starts at the beginning of a line.
     */
    restart_ui_task() ;
  }
  /*
   * Timer for this task.
   */
  ui_tmr_new_n_kick() ;
  /*
   * Set standard_fd to a value which will assure its proper setting
   * in the folowing loop.
   */
  error_count = 10 ;
  for (ui = 0 ; ui < error_count ; ui++)
  {
    /*
     * Make sure this task gets every single character as
     * it is typed (without echo).
     */
    int
      terminal_options ;
    /*
     * Get file descriptor of STD_IN/STD_OUT/STD_ERR.
     * In this system, it is one file.
     */
    standard_fd = d_ioGlobalStdGet(STD_IN) ;
    terminal_options = ioctl(standard_fd,FIOGETOPTIONS,0) ;
    terminal_options &=
        ~(OPT_ECHO | OPT_MON_TRAP | OPT_ABORT | OPT_LINE) ;
    status = ioctl(standard_fd,FIOSETOPTIONS,terminal_options) ;
    if (status == ERROR)
    {
      char
        err_string[100] ;
      sal_sprintf(err_string,"ui_prep_proc - ioctrl for fd=%lu",
                (unsigned long)standard_fd) ;
      gen_err(FALSE,FALSE,(int)FALSE,FALSE,
          err_string,proc_name,
            SVR_MSG,UI_PREP_PROC_ERR_06,FALSE,0,-1,FALSE) ;
      error_value = errnoGet() ;
      /*
       * If file descriptor has changed on the fly (telnet
       * started or ended while file action was half way through)
       * then just ignore the character.
       */
      if (error_value == S_iosLib_INVALID_FILE_DESCRIPTOR)
      {
        /* If, half way through, standard IO file descriptors
         * change (like in telnet connection/disconnection) then
         *
         * send signal which will eventually restart this procedure.
         * Delay is added to allow the system to respond to the signal.
         */
        /*
         * Make sure standard IO is blocked.
         */
        set_std_io_blocked(TRUE) ;
        /*
         * Reaching here may be due to 'telnet inactivity' so, before
         * sending the signal, make sure 'telnet inactivity' is not
         * detected again within 'ui_proc_signal()'
         */
        set_telnet_activity_time_mark() ;
        status = raise(SIGALRM) ;
        gen_err(TRUE,TRUE,(int)status,ERROR,
              "ui_prep_proc - raise",proc_name,SVR_FTL,
              UI_PREP_PROC_ERR_07,FALSE,0,-1,FALSE) ;
        /*
         * Just wait long enough for signal to be captured and, consequently,
         * restart this task.
         */
        d_taskDelay(300) ;
      }
      else
      {
        /*
         * Fatal error while handling standard IO.
         * There is no return from this procedure call.
         */
        gen_err(FALSE,FALSE,(int)FALSE,FALSE,
            err_string,proc_name,
              SVR_FTL,UI_PREP_PROC_ERR_08,FALSE,0,-1,FALSE) ;
      }
      continue ;
    }
    set_current_ui_fd(standard_fd) ;
    break ;
  }
  if (ui >= error_count)
  {
    gen_err(FALSE,TRUE,(int)status,ERROR,
              "ui_prep_proc - restart fail",proc_name,SVR_FTL,
              UI_PREP_PROC_ERR_09,FALSE,0,-1,FALSE) ;
  }
  /*
   * Make sure standard IO is not blocked.
   * Std IO gets blocked when the system finds that its
   * file descriptor has been changed (telnet connection
   * up or down).
   */
  set_std_io_blocked(FALSE) ;
  /*
   * The main loop.
   */
  user_fd = get_user_if_file_descriptor() ;
  {
    /*
     * Calculate number of ticks to wait between pollings of
     * input queue.
     */
    unsigned
      long
        poll_timeout_10ms,
        nano_per_tick ;
    nano_per_tick = 1000000000 / CLOCKS_PER_SEC ;
    /*
     * Timeout value, in units of 10 milliseconds
     */
    poll_timeout_10ms = 5 ;
    poll_timeout_ticks = ((poll_timeout_10ms * 1000000) / (nano_per_tick / 10)) ;
  }
  {
    /*
     * Initialize escape sequence handling.
     */
    escape_seq_on = FALSE ;
    in_chars_flush = FALSE ;
    in_chars_index = 0 ;
    /*
     * Timeout value, in units of 10 milliseconds
     */
    escape_seq_timeout_10ms = 10 ;
  }
  while (TRUE)
  {
    int
      out_char_count,
      in_char_count ;
    char
      in_char ;
    int
      unreadCount ;
    unsigned int
      use_aux_input;
    static
      unsigned int
        Last_time_user_is_101 = FALSE;
    static
      unsigned int
        Major_fd;

    use_aux_input = FALSE;
    in_char_count = 0;

    /*
     * Get the next character from standard input.
     */
    status = ioctl((int)ioGlobalStdGet(STD_IN),FIONREAD,(int)&unreadCount) ;

    if (unreadCount == 0)
    {
      /*
       * Try to get from telnet 101, only if we do not have something from the STD_IN
       */
#if !DUNE_BCM
      if (get_char(get_aux_input_queue(), &in_char))
      {
        use_aux_input = TRUE;
        in_char_count = 1;
        if (Last_time_user_is_101 == FALSE)
        {
          /*
           * Need to switch the STD_OUT
           */
          Last_time_user_is_101 = TRUE;
          Major_fd = d_ioGlobalStdGet(STD_OUT);
          d_ioGlobalStdSet(STD_OUT, get_aux_input_fd());
        }
      }
#endif
    }

    if ((status == ERROR) || (unreadCount != 0) || use_aux_input)
    {
      if ( use_aux_input == TRUE)
      {
        /*
         * 'in_char' is already loaded.
         */
      }
      else
      {
        in_char_count = read(d_ioGlobalStdGet(STD_IN),&in_char,1) ;
        if (Last_time_user_is_101 == TRUE)
        {
          /*
           * Need to switch the STD_OUT
           */
          Last_time_user_is_101 = FALSE;
          d_ioGlobalStdSet(STD_OUT, Major_fd);
        }
      }

      in_chars_array[in_chars_index] = in_char ;
      in_chars_index++ ;
      set_telnet_activity_time_mark() ;
      if (gen_err(TRUE,TRUE,(int)in_char_count,ERROR,
          "ui_prep_proc - read 1",
                proc_name,
                SVR_MSG,UI_PREP_PROC_ERR_03,FALSE,0,-1,FALSE))
      {
        error_value = errnoGet() ;
        /*
         * If file descriptor has changed on the fly (telnet
         * started or ended while file action was half way through)
         * then just ignore the character.
         */
        if (error_value == S_iosLib_INVALID_FILE_DESCRIPTOR)
        {
          /* If, half way through, standard IO file descriptors
           * change (like in telnet connection/disconnection) then
           *
           * send signal which will eventually restart this procedure.
           * Delay is added to allow the system to respond to the signal.
           * It is expected that this task is restarted before delay
           * expires.
           */
          /*
           * Make sure standard IO is blocked.
           */
          set_std_io_blocked(TRUE) ;
          status = raise(SIGALRM) ;
          gen_err(TRUE,TRUE,(int)status,ERROR,
                "ui_prep_proc - raise",proc_name,SVR_FTL,
                UI_PREP_PROC_ERR_04,FALSE,0,-1,FALSE) ;
          d_taskDelay(300) ;
          /*
           * Just making sure...
           */
          in_chars_index = 0 ;
          continue ;
        }
        else
        {
          /*
           * Fatal error while handling standard IO.
           * There is no return from this procedure call.
           */
          gen_err(TRUE,TRUE,(int)status,ERROR,
                "ui_prep_proc - read 2",proc_name,
                SVR_FTL,UI_PREP_PROC_ERR_05,FALSE,0,-1,FALSE) ;
        }
      }
    }
    else
    {
      /*
       * No character has been found on queue.
       * In the regular case, wait some time and try
       * again.
       * In active escape sequence case, wait some time
       * after last character and, then, send whatever has
       * been accumulated for processing.
       */
      if (in_chars_index >= sizeof(in_chars_array))
      {
        /*
         * If local storage of input chars has overflown
         * then process characters received so far.
         */
        in_chars_flush = TRUE ;
        escape_seq_on = FALSE ;
      }
      else
      {
        if (!escape_seq_on)
        {
          d_taskDelay(poll_timeout_ticks) ;
          continue ;
        }
        else
        {
          time_now = get_watchdog_time(TRUE) ;
          if ((time_now - escape_seq_time_last_char) < escape_seq_timeout_10ms)
          {
            /*
             * Escape sequence timeout has not expired yet.
             */
            d_taskDelay(poll_timeout_ticks) ;
            continue ;
          }
          else
          {
            /*
             * Escape sequence timeout has expired. Process
             * characters collected so far.
             */
            in_chars_flush = TRUE ;
          }
        }
      }
    }
    /*
     * If processing of all characters in buffer is required then
     * do not try to handle escape sequences.
     */
    if (!in_chars_flush)
    {
      if (escape_seq_on)
      {
        escape_seq_time_last_char = get_watchdog_time(TRUE) ;
        translate_escape_seq(in_chars_array,&in_chars_index,&loop_on) ;
        if (loop_on)
        {
          continue ;
        }
      }
      else
      {
        if ((in_char == ESC) && (in_chars_index == 1))
        {
          escape_seq_on = TRUE ;
          escape_seq_time_last_char = get_watchdog_time(TRUE) ;
          in_chars_flush = FALSE ;
          continue ;
        }
      }
    }
    /*
     * Here process typed in characters.
     */
    escape_seq_on = FALSE ;
    in_chars_flush = FALSE ;
    in_chars_len = in_chars_index ;
    for (in_chars_index = 0 ; in_chars_index < in_chars_len ; in_chars_index++)
    {
      in_char = in_chars_array[in_chars_index] ;
      switch (in_char)
      {
        case ESC:
        case CLEAR_TYPE_AHEAD:
        {
          /*
           * Clear type ahead buffer and
           * stop periodic display.
           */
          d_ioctl(user_fd,FIOFLUSH,0) ;
          set_periodic_off() ;
          /*
           * If system is in 'file input mode', for CLI, close
           * input file and get out of this mode.
           */
          if (is_file_input_mode())
          {
            close_cli_input_fd() ;
            clear_file_input_mode() ;
          }
          if (!is_echo_mode_on())
          {
            /*
             * If user requested 'escape' while echo mode was off,
             * get back to 'echo mode on'
             */
            set_echo_mode_on() ;
          }
          break ;
        }
        case FLUSH_STD_OUTPUT:
        {
          /*
           * Stop long printings.
           */
          set_abort_print_flag() ;
          if (!is_echo_mode_on())
          {
            /*
             * If user requested 'flush' while echo mode was off,
             * get back to 'echo mode on'
             */
            set_echo_mode_on() ;
          }
          break ;
        }
        default:
        {
          break ;
        }
      }
      {
        /*
         * Send the character to user interface task (use
         * custom pipe).
         * Note: Send input character only if no input file has been
         * (temporarily) assigned to CLI. If one has been assigned
         * then just flush input characters. Input from file is done in
         * sys_getch().
         */
        if (!is_file_input_mode())
        {
          out_char_count = d_write(user_fd,&in_char,1,FALSE) ;
        }
      }
    }
    in_chars_index = 0 ;
  }
  return ;
}
/* } */
#elif defined(__DUNE_SSF__)
/* { */
void
  ui_prep_proc(
    void
  )
{
  STATUS
    status=0;
  const char
    *proc_name ;
  unsigned
    long
      poll_timeout_ticks ;
  unsigned
    int
      escape_seq_timeout_10ms,
      escape_seq_on,
      loop_on,
      in_chars_flush,
      in_chars_index,
      in_chars_len;
  unsigned
    long
      time_now,
      escape_seq_time_last_char ;
  char
    in_chars_array[10] ;
  static int
    is_first_char = 1;

  proc_name = "ui_prep_proc" ;
  escape_seq_time_last_char = 0;

  restart_ui_task();

  {
    /*
     * Calculate number of ticks to wait between pollings of
     * input queue.
     */
    unsigned
      long
        poll_timeout_10ms,
        nano_per_tick ;
        nano_per_tick = 1000000000 / D_TICKS_PER_SEC ;

    /*
     * Timeout value, in units of 10 milliseconds
     */
    poll_timeout_10ms = 5 ;
    poll_timeout_ticks = 5; /* ((poll_timeout_10ms * 1000000) / (nano_per_tick / 10)) */;
  }
  {
    /*
     * Initialize escape sequence handling.
     */
    escape_seq_on = FALSE ;
    in_chars_flush = FALSE ;
    in_chars_index = 0 ;
    /*
     * Timeout value, in units of 10 milliseconds
     */
    escape_seq_timeout_10ms = 10 ;
  }
  while (TRUE)
  {
    int
      in_char_count ;
    char
      in_char=0;
#if !DUNE_BCM
    static
      int
        Major_fd = -1;
#endif

    in_char_count = 0;

    /*
     * Try to get from telnet aux
     */
#if !DUNE_BCM
    if (get_char(get_aux_input_queue(), &in_char))
    {
      in_char_count = 1;
      if (is_first_char)
      {
        /* save in Major_fd stdout */
        Major_fd = dup(STDOUT_FILENO);

        /* redirect stdout to aux_input_fd */
        dup2(get_aux_input_fd(),STDOUT_FILENO);

        is_first_char = 0;
      }
    }
#endif

    if ((status == ERROR) || (in_char_count != 0))
    {
      in_chars_array[in_chars_index] = in_char ;
      in_chars_index++ ;
      set_telnet_activity_time_mark() ;
    }
    else
    {
      /*
       * No character has been found on queue.
       * In the regular case, wait some time and try
       * again.
       * In active escape sequence case, wait some time
       * after last character and, then, send whatever has
       * been accumulated for processing.
       */
      if (in_chars_index >= sizeof(in_chars_array))
      {
        /*
         * If local storage of input chars has overflown
         * then process characters received so far.
         */
        in_chars_flush = TRUE ;
        escape_seq_on = FALSE ;
      }
      else
      {
        if (!escape_seq_on)
        {
          d_taskDelay(poll_timeout_ticks) ;
          continue ;
        }
        else
        {
          time_now = get_watchdog_time(TRUE) ;
          if ((time_now - escape_seq_time_last_char) < escape_seq_timeout_10ms)
          {
            /*
             * Escape sequence timeout has not expired yet.
             */
            d_taskDelay(poll_timeout_ticks) ;
            continue ;
          }
          else
          {
            /*
             * Escape sequence timeout has expired. Process
             * characters collected so far.
             */
            in_chars_flush = TRUE ;
          }
        }
      }
    }
    /*
     * If processing of all characters in buffer is required then
     * do not try to handle escape sequences.
     */
    if (!in_chars_flush)
    {
      if (escape_seq_on)
      {
        escape_seq_time_last_char = get_watchdog_time(TRUE) ;
        translate_escape_seq(in_chars_array,&in_chars_index,&loop_on) ;
        if (loop_on)
        {
          continue ;
        }
      }
      else
      {
        if ((in_char == ESC) && (in_chars_index == 1))
        {
          escape_seq_on = TRUE ;
          escape_seq_time_last_char = get_watchdog_time(TRUE) ;
          in_chars_flush = FALSE ;
          continue ;
        }
      }
    }
    /*
     * Here process typed in characters.
     */
    escape_seq_on = FALSE ;
    in_chars_flush = FALSE ;
    in_chars_len = in_chars_index ;
    for (in_chars_index = 0 ; in_chars_index < in_chars_len ; in_chars_index++)
    {
      in_char = in_chars_array[in_chars_index] ;
      switch (in_char)
      {
        case ESC:
        case CLEAR_TYPE_AHEAD:
        {
          /*
           * Clear type ahead buffer and
           * stop periodic display.
           */
          /* d_ioctl(user_fd,FIOFLUSH,0) ; */
          set_periodic_off() ;
          /*
           * If system is in 'file input mode', for CLI, close
           * input file and get out of this mode.
           */
          if (!is_echo_mode_on())
          {
            /*
             * If user requested 'escape' while echo mode was off,
             * get back to 'echo mode on'
             */
            set_echo_mode_on() ;
          }
          break ;
        }
        case FLUSH_STD_OUTPUT:
        {
          /*
           * Stop long printings.
           */
          set_abort_print_flag() ;
          if (!is_echo_mode_on())
          {
            /*
             * If user requested 'flush' while echo mode was off,
             * get back to 'echo mode on'
             */
            set_echo_mode_on() ;
          }
          break ;
        }
        case RESTART_TERMINAL:
          /* wait for ending processing */
          while(char_buff_is_empty(&Ui_char_queue) == FALSE)
          {
            sal_msleep(1);
          }

          /* ask ui_proc to finish */
          S_ui_proc_continue = FALSE;
          /* send \0 to ui_proc to unblock sys_getch */
          put_char(&Ui_char_queue, '\0');

          /* wait for ui_proc to finish */
          S_ui_tid = -1;
          while(S_ui_proc_finished == FALSE)
          {
            sal_msleep(1);
          }

#if !DUNE_BCM
          /* redirect output back to stdout */
          dup2(Major_fd, STDOUT_FILENO);
          close(Major_fd);
#else
          assert(1);
#endif

          /* restart ui_proc */
          restart_ui_task();
          is_first_char = 1;
          break;
        default:
        {
          break ;
        }
      }
      {
        if (in_char != RESTART_TERMINAL)
        {
          /*
           * Send the character to user interface task
           */

           put_char(&Ui_char_queue, in_char);
        }
      }
    }
    in_chars_index = 0 ;
  }

}
/* } */
#else /*SAND_LOW_LEVEL_SIMULATION*/
/* { */
void
  ui_prep_proc(
    void
  )
{
  ui_proc();
}
/* } __VXWORKS__ */
#endif

/*****************************************************
*NAME
*  validate_ui_interpreter
*TYPE: PROC
*DATE: 16/JAN/2002
*FUNCTION:
*  Before initializing the command line module,
*   this function should be called, to validate
*   the input.
*CALLING SEQUENCE:
*  validate_ui_interpreter()
*INPUT:
*  SOC_SAND_DIRECT:
*    None.
*  SOC_SAND_INDIRECT:
*    Varios globals. Can not be specific here.
*OUTPUT:
*  SOC_SAND_DIRECT:
*    None.
*  SOC_SAND_INDIRECT:
*    Initialized CLI interpreter.
*REMARKS:
*  NONE.
*SEE ALSO:
*****************************************************/
 void
  validate_ui_interpreter(
    void
  )
{
  /*
   * Initialize the ui interpreter machine.
   */
  unsigned
    int
      num_params,
      ui,
      uj,
      uk,
      nof_subjects,
      pass;
  /* subject are sometimes defines as consts */
  const SUBJECT
    *subject_ptr ;
  PARAM
    *parameter_ptr ;
  const char
    *proc_name ;
  proc_name = "validate_ui_interpreter" ;

  /*
   * Clear indication on recovery from fatal exception
   * (starting state).
   */
  set_recovery_msg_off() ;
  subject_ptr = (const SUBJECT *)&Subjects_list_rom[0] ;
  /*
   * Assume there are less than MAX_SUBJECTS_IN_UI subjects in the system.
   */
  pass = FALSE;
  for (ui = 0 ; ui < MAX_SUBJECTS_IN_UI ; ui++)
  {
    if (subject_ptr->subject_id == SUBJECT_END_OF_LIST)
    {
      pass = TRUE ;
      break;
    }
    subject_ptr++ ;
  }
  if(!pass)
  {
    d_printf(
      "%s found Error: Too many subjects.\n\r",
      proc_name
    );
  }

  nof_subjects = ui ;
  /*
   * Get info about list of subjects.
   */
  subject_ptr = (const SUBJECT *)&Subjects_list_rom[0] ;

  for (ui = 0 ; ui < nof_subjects ; ui++,subject_ptr++)
  {
    parameter_ptr = subject_ptr->allowed_params ;

    pass = FALSE;
    for (uj = 0 ; uj < MAX_PARAMS_PER_SUBJECT ; uj++,parameter_ptr++)
    {
      if (parameter_ptr->param_id == PARAM_END_OF_LIST)
      {
        pass = TRUE;
        break;
      }
    }
    if(!pass)
    {
      d_printf(
        "%s found Error: Too many paramters. (Subject %s)\n\r",
        proc_name,
        subject_ptr->subj_name
      );
    }

    num_params = uj ;
    parameter_ptr = &(subject_ptr->allowed_params)[0];
    for (uj = 0 ; uj < num_params ; uj++, parameter_ptr++)
    {
      PARAM_VAL_RULES
        *param_val_rules;
      unsigned
        int
          num_vals,
          num_numeric_vals ;

      param_val_rules = parameter_ptr->allowed_vals ;
      /*
       * Assume there are less than MAX_VALUES_PER_PARAM values for each parameter
       */
      pass = FALSE;
      for (uk = 0; uk < MAX_VALUES_PER_PARAM ; uk++,param_val_rules++)
      {
        if (param_val_rules->val_type == VAL_END_OF_LIST)
        {
          pass = TRUE;
          break;
        }
      }
      if(!pass)
      {
        d_printf(
          "%s found Error: Too many values.\n\r"
          "(Subject %s, parameter %s)\n\r",
          proc_name,
          subject_ptr->subj_name,
          parameter_ptr->par_name
        );
      }
      num_vals = uk ;
      num_numeric_vals = 0 ;
      param_val_rules = parameter_ptr->allowed_vals ;
      for (uk = 0 ; uk < num_vals ; uk++,param_val_rules++)
      {
        switch (param_val_rules->val_type & VAL_TYPES_MASK)
        {
          case VAL_NUMERIC:
          {
            num_numeric_vals++ ;
            /*
             * There can only be one numeric value per one parameter!
             */
            if (num_numeric_vals > 1)
            {
              d_printf(
                "%s found Error: More than one numeric value.\n\r"
                "(Subject %s, parameter %s)\n\r",
                proc_name,
                subject_ptr->subj_name,
                parameter_ptr->par_name
              );
            }
            break ;
          }
          case VAL_SYMBOL:
            break ;
          case VAL_TEXT:
          {
            /*
             * There can only be one free text value per one parameter and
             * other parameter values are not allowed in that case!
             */
            if (parameter_ptr->num_allowed_vals > 1)
            {
              d_printf(
                "%s found Error: More than one value in addition to free text value.\n\r"
                "(Subject %s, parameter %s)\n\r",
                proc_name,
                subject_ptr->subj_name,
                parameter_ptr->par_name
              );
            }
            break ;
          }
          case VAL_IP:
          {
            if (parameter_ptr->num_allowed_vals > 1)
            {
              d_printf(
                "%s found Error: More than one value in addition to IP address value.\n\r"
                "(Subject %s, parameter %s)\n\r",
                proc_name,
                subject_ptr->subj_name,
                parameter_ptr->par_name
              );
            }
            break ;
          }
          default:
          {
            d_printf(
              "%s found Error: Illegal parameter value (%d).\n\r"
              "(Subject %s, parameter %s)\n\r",
              proc_name,
              param_val_rules->val_type,
              subject_ptr->subj_name,
              parameter_ptr->par_name
            );
          }
        }
      }
      if ((parameter_ptr->default_care & HAS_DEFAULT_MASK) == HAS_DEFAULT)
      {
        if ((parameter_ptr->default_val->val_type & VAL_TEXT) == VAL_TEXT)
        {
          if(strlen(parameter_ptr->default_val->value.val_text) >=
             MAX_SIZE_OF_TEXT_VAR
            )
          {
            d_printf(
              "%s found Error: Too many values.\n\r"
              "(Subject %s, parameter %s)\n\r",
              proc_name,
              subject_ptr->subj_name,
              parameter_ptr->par_name
            );
          }
        }
      }
    }
  }
  return ;
}


#if !(defined(LINUX) || defined(UNIX)) && !defined(__DUNE_GTO_BCM_CPU__)
/* { */
void
   ui_triger_page_mode(
 void
)
{
  unsigned
    char
      page_mode;
  int
    err ;

  /*
   * Enable/disable page mode. Take it from NVRAM.
   */
  err = page_mode_from_nv(&page_mode) ;
  if (err)
  {
    page_mode = TRUE ;
  }
  if (page_mode)
  {
    set_page_mode() ;
  }
  else
  {
    clear_page_mode() ;
  }
}
#endif /* LINUX */

unsigned int
  ui_skip_this_subject(int subject_id)
{
  unsigned int
    do_skip = FALSE;
  D_BOARD_CHIP_TYPE
    board_device_type;

  board_get_device_type(
    &board_device_type
  );
  switch(subject_id)
  {
    case SUBJECT_FMF_SERVICES_ID:
    case SUBJECT_SIM_SERVICES_ID:
    case SUBJECT_VX_SHELL_ID:
    case SUBJECT_GENERAL_ID:
    case SUBJECT_NVRAM_ID:
    case SUBJECT_MEM_ID:
    case SUBJECT_HELP_ID:
    case SUBJECT_HISTORY_ID:
    case SUBJECT_SNMP_ID:
    case SUBJECT_SAND_SERVICES_ID:
    case SUBJECT_CSYS_SERVICES_ID:
    case SUBJECT_SKT_ID:
    case SUBJECT_END_OF_LIST:
    case SUBJECT_DHRP_SERVICES_ID:
    case SUBJECT_DHRP_API_SERVICES_ID:
      break;

    case SUBJECT_PSS_SERVICES_ID:
    case SUBJECT_FAP10M_SERVICES_ID:
    case SUBJECT_LINE_CARD_SERVICES_ID:
    case SUBJECT_DEMO_SERVICES_ID:
    case SUBJECT_GOBI_DEMO_SERVICES_ID:
    case SUBJECT_NEGEV_DEMO_SERVICES_ID:
      if(board_device_type == D_BOARD_CHIP_TYPE_FAP10M_A ||
         board_device_type == D_BOARD_CHIP_TYPE_FAP10M_B
        )
      {
        do_skip = FALSE;
      }
      else
      {
        do_skip = TRUE;
      }
    break;
    case SUBJECT_GFA_SERVICES_ID:
      if(board_device_type == D_BOARD_CHIP_TYPE_FAP20V_A ||
         board_device_type == D_BOARD_CHIP_TYPE_FAP20V_B
        )
      {
        do_skip = FALSE;
      }
      else
      {
        do_skip = TRUE;
      }
    break;

    case SUBJECT_TGS_SERVICES_ID:
      if(board_device_type == D_BOARD_CHIP_TYPE_FAP20V_A ||
         board_device_type == D_BOARD_CHIP_TYPE_FAP20V_B ||
         board_device_type == D_BOARD_CHIP_TYPE_FAP21V )
      {
        do_skip = FALSE;
      }
      else
      {
        do_skip = TRUE;
      }
      break;

    case SUBJECT_DIAG_SERVICES_ID:
      do_skip = FALSE;
      break;

    case SUBJECT_TEVB_SERVICES_ID:
    case SUBJECT_TIMNA_ACCESS_SERVICES_ID:
    case SUBJECT_TIMNA_API_SERVICES_ID:
#if LINK_TIMNA_LIBRARIES
    if(!tevb_is_timna_connected())
    {
      do_skip = TRUE;
    }
#else
      do_skip = TRUE;
#endif
      break;

    case SUBJECT_PPD_API_SERVICES_ID:
    case SUBJECT_PB_PP_ACC_SERVICES_ID:
    case SUBJECT_PB_PP_ACC2_SERVICES_ID:
#if LINK_T20E_LIBRARIES
      if(!tevb_is_timna_connected())
      {
        do_skip = TRUE;
      }
#endif
      break;

    case SUBJECT_T20E_ACC_SERVICES_ID:
    case SUBJECT_OAM_ACC_SERVICES_ID:
    case SUBJECT_OAM_API_SERVICES_ID:      
#if LINK_T20E_LIBRARIES
    if(tevb_is_timna_connected())
    {
      do_skip = FALSE;
    }
    else
    {
      do_skip = TRUE;
    }
#endif
    break;

    case SUBJECT_FAP21V_API_SERVICES_ID:
    case SUBJECT_FAP21V_ACC_SERVICES_ID:
    case SUBJECT_FAP21V_GFA_SERVICES_ID:
    case SUBJECT_FAP21V_APP_SERVICES_ID:
#if LINK_FAP21V_LIBRARIES
      if(board_device_type == D_BOARD_CHIP_TYPE_FAP21V
#if LINK_TIMNA_LIBRARIES
        && !tevb_is_timna_connected()
#endif
        )
        {
          do_skip = FALSE;
        }
        else
        {
          do_skip = TRUE;
        }
#else
        do_skip = TRUE;
#endif
      break;

    case SUBJECT_PETRA_GFA_SERVICES_ID:
    case SUBJECT_PETRA_API_SERVICES_ID:
    case SUBJECT_TMD_API_SERVICES_ID:
    case SUBJECT_PETRA_ACC_SERVICES_ID:
    case SUBJECT_SWP_PETRA_SERVICES_ID:
    case SUBJECT_PTG_SERVICES_ID:
#if LINK_PETRA_LIBRARIES && !defined(__DUNE_SSF__)
      if ((board_device_type == D_BOARD_CHIP_TYPE_PETRA) ||
          (board_device_type == D_BOARD_CHIP_TYPE_PB))
      {
        do_skip = FALSE;
      }
      else
      {
        do_skip = TRUE;
      }
#elif defined(__DUNE_SSF__)
      do_skip = FALSE;
#else
      do_skip = TRUE;
#endif
      break;
    case SUBJECT_PB_API_SERVICES_ID:
      do_skip = TRUE;
      break;

    case SUBJECT_PETRA_PP_API_SERVICES_ID:
    case SUBJECT_PETRA_PP_ACC_SERVICES_ID:
    case SUBJECT_PETRA_PP_SWP_SERVICES_ID:
      do_skip = TRUE;
#if LINK_PETRA_LIBRARIES
#if PETRA_PP
      do_skip = FALSE;
#endif
#endif
      break;
    case SUBJECT_PCP_API_SERVICES_ID:
      do_skip = TRUE;
      break;
    case SUBJECT_PETRA_PP_GSA_SERVICES_ID:
      do_skip = TRUE;
#if LINK_PETRA_LIBRARIES
#if PETRA_PP
      do_skip = FALSE;
#endif
#endif
      break;

    case SUBJECT_PETRA_GSA_SERVICES_ID:
      do_skip = TRUE;
#if LINK_PETRA_LIBRARIES
      do_skip = FALSE;
#endif
      break;

    case SUBJECT_FAP20V_SWEEP_APP_SERVICES_ID:
#if LINK_FAP20V_LIBRARIES
      if(board_device_type == D_BOARD_CHIP_TYPE_FAP20V_A)
      {
        do_skip = FALSE;
      }
      else
      {
        do_skip = TRUE;
      }
    break;

    case SUBJECT_FAP20V_SWEEP_APP_B_SERVICES_ID:
      if(board_device_type == D_BOARD_CHIP_TYPE_FAP20V_B)
      {
        do_skip = FALSE;
      }
      else
      {
        do_skip = TRUE;
      }
#endif
    break;

    case SUBJECT_FE200_SERVICES_ID:
#if LINK_FE200_LIBRARIES
      if(board_device_type == D_BOARD_CHIP_TYPE_FE200)
      {
        do_skip = FALSE;
      }
      else
      {
        do_skip = TRUE;
      }
#endif
    break;

    case SUBJECT_FAP20V_SERVICES_ID:
#if LINK_FAP20V_LIBRARIES
      if(board_device_type == D_BOARD_CHIP_TYPE_FAP20V_A
#if LINK_TIMNA_LIBRARIES
        && !tevb_is_timna_connected()
#endif
        )
        {
          do_skip = FALSE;
        }
        else
        {
          do_skip = TRUE;
        }
#endif
      break;
    case SUBJECT_FAP20V_B_SERVICES_ID:
#if LINK_FAP20V_LIBRARIES
        if(board_device_type == D_BOARD_CHIP_TYPE_FAP20V_B
#if LINK_TIMNA_LIBRARIES
          && !tevb_is_timna_connected()
#endif
          )
        {
          do_skip = FALSE;
        }
        else
        {
          do_skip = TRUE;
        }
#endif
    break;
    case SUBJECT_FE600_BSP_SERVICES_ID:
#if LINK_FE600_LIBRARIES
      if(board_device_type == D_BOARD_CHIP_TYPE_FE600)
      {
        do_skip = FALSE;
      }
      else
      {
        do_skip = TRUE;
      }
#endif
    break;

    case SUBJECT_FE600_API_SERVICES_ID:
#if LINK_FE600_LIBRARIES
      if(board_device_type == D_BOARD_CHIP_TYPE_FE600)
      {
      do_skip = FALSE;
      }
      else
      {
        do_skip = TRUE;
      }
#endif
    break;

    default:
      d_printf("WARNING: ui_skip_this_subject() got unrecognized subject %d.\n\r",subject_id);
  }
  /*
   * When the board is unknown, all the UI subjects are allowed.
   */
  if(board_device_type == D_BOARD_CHIP_TYPE_LAST)
  {
    do_skip = FALSE;
  }
  return do_skip;
}

/*****************************************************
*NAME
*  init_ui_interpreter
*TYPE: PROC
*DATE: 16/JAN/2002
*FUNCTION:
*  All initializations related to command line
*  interpreter.
*CALLING SEQUENCE:
*  init_ui_interpreter()
*INPUT:
*  SOC_SAND_DIRECT:
*    None.
*  SOC_SAND_INDIRECT:
*    Various globals. Can not be specific here.
*OUTPUT:
*  SOC_SAND_DIRECT:
*    None.
*  SOC_SAND_INDIRECT:
*    Initialized CLI interpreter.
*REMARKS:
*  NONE.
*SEE ALSO:
*****************************************************/

void
   init_ui_interpreter(
 void
)
{
  /*
   * Initialize the ui interpreter machine.
   */
  unsigned
    int
      params_list_size,
      subjects_list_size,
      list_len,
      subject_len,
      params_ascii_list_len,
      num_params,
      ui,
      uj,
      uk,
      valid_subjects;
#if defined(__VXWORKS__) && !defined(__DUNE_GTO_BCM_CPU__)
  int
    err ;
#endif /* __VXWORKS__ */
  SUBJECT
    *subject_ptr ;
  PARAM
    *params_list,
    *parameter_ptr ;
  char
    *params_ascii_list ;
  const char
    *proc_name ;
  char
    *help_buffer = NULL;
  proc_name = "init_ui_interpreter" ;
#if defined(__VXWORKS__) && !defined(__DUNE_GTO_BCM_CPU__)

  /* { */
  {
    unsigned
      char
        page_lines ;
    /*
     * Set number of lines on one page from NVRAM.
     * On error, set to 20.
     */
    err = page_lines_from_nv(&page_lines) ;
    if (err)
    {
      page_lines = 20 ;
    }
    init_page_print((unsigned int)page_lines) ;

    /*
    * At start up, page mode is disabled, to enable
    *  working with page mode, with no IO output client(telnet, serial)
    */
    clear_page_mode() ;
  }
  {
    unsigned
      char
        line_mode ;
    /*
     * Enable/disable line mode. Take it from NVRAM.
     */
    err = line_mode_from_nv(&line_mode) ;
    if (err)
    {
      line_mode = FALSE ;
    }
    if (line_mode)
    {
      set_line_mode() ;
    }
    else
    {
      clear_line_mode() ;
    }
  }
/* } __VXWORKS__ */
#endif

  /*
   * Clear indication on recovery from fatal exception
   * (starting state).
   */
  set_recovery_msg_off() ;
  /*
   * Assume there are less than MAX_SUBJECTS_IN_UI subjects in the system.
   */
  Num_subjects = 0;
  for (ui = 0 ; ui < MAX_SUBJECTS_IN_UI ; ui++)
  {
    if (Subjects_list_rom[ui].subject_id == SUBJECT_END_OF_LIST)
    {
      break ;
    }
    if(!ui_skip_this_subject(Subjects_list_rom[ui].subject_id))
    {
      Num_subjects++;
    }
  }
  /*
   * Add one to size of array in order to include the last
   * element which only indicates 'end of list'.
   */
  subjects_list_size = (Num_subjects + 1) * sizeof(SUBJECT) ;
  /*
   * Work with ram pointer. This way, subjects can be added
   * dynamically, at later stages.
   */
  Subjects_list = (SUBJECT *)sal_alloc(subjects_list_size, "Dune CLI: Subjects_list") ;
/*
  memcpy(Subjects_list,Subjects_list_rom,subjects_list_size) ;
*/
  valid_subjects = 0;
  for (ui = 0 ; ui < MAX_SUBJECTS_IN_UI ; ui++)
  {
    if(!ui_skip_this_subject(Subjects_list_rom[ui].subject_id))
    {
      memcpy(
        &(Subjects_list[valid_subjects]),
        &(Subjects_list_rom[ui]),
        sizeof(SUBJECT)
      ) ;
      valid_subjects++;
    }
    if (Subjects_list_rom[ui].subject_id == SUBJECT_END_OF_LIST)
    {
      break ;
    }
  }
  /*
   * Sort subject by name. Leave last in place.
   */
  qsort(
    &Subjects_list[0],Num_subjects,
    sizeof(SUBJECT),compare_subjs_by_name) ;
  /*
   * Get info about list of subjects.
   */
  for (ui = 0, list_len = 0 ; ui < Num_subjects ; ui++)
  {
    /*
     * Add 2 for CR/LF.
     */
    list_len += sal_strlen(Subjects_list[ui].subj_name) + 2 ;
  }

  /*
   * Add space for terminating null.
   */
  Subjects_ascii_list = (char *)sal_alloc(list_len + 1, "Dune CLI: Subjects_ascii_list") ;
  subject_len = list_len + 1;
  /*
   * Length of this list is at least 1.
   * Fill with '0'. Just to make sure (only
   * one '0' is required.
   */
  sal_memset(Subjects_ascii_list,0,list_len + 1) ;
  /*
   * Now load memory pointed by Subjects_ascii_list.
   */
  subject_ptr = &Subjects_list[0] ;
  for (ui = 0, list_len = 0 ; ui < Num_subjects ; ui++)
  {
    if (subject_ptr->subject_id == SUBJECT_END_OF_LIST)
    {
      break ;
    }
    UTILS_STRCAT_SAFE(Subjects_ascii_list,subject_ptr->subj_name, subject_len) ;
    UTILS_STRCAT_SAFE(Subjects_ascii_list,"\r\n", subject_len) ;
    subject_ptr++ ;
  }
  /*
   * Now do the same for parameters list which is attached
   * to each subject.
   */

  subject_ptr = &Subjects_list[0] ;
  for (ui = 0 ; ui < Num_subjects ; ui++,subject_ptr++)
  {
    parameter_ptr = subject_ptr->allowed_params ;
    subject_ptr->num_params_with_default = 0 ;
    subject_ptr->num_params_without_default = 0 ;
    /*
     * Assume there are less than MAX_PARAMS_PER_SUBJECT
     * parameters for this subject
     */
    for (uj = 0 ; uj < MAX_PARAMS_PER_SUBJECT ; uj++,parameter_ptr++)
    {
      if (parameter_ptr->param_id == PARAM_END_OF_LIST)
      {
        break ;
      }
      if ((parameter_ptr->default_care & HAS_DEFAULT_MASK) == HAS_DEFAULT)
      {
        subject_ptr->num_params_with_default++ ;
      }
      else
      {
        subject_ptr->num_params_without_default++ ;
      }
    }
    num_params = subject_ptr->num_allowed_params = uj ;
    /*
     * Add one to size of array in order to include the last
     * element which only indicates 'end of list'.
     */
    params_list_size = (num_params + 1) * sizeof(PARAM) ;
    /*
     * Work with ram pointer. This way, parameters can be added
     * dynamically, at later stages.
     */
    params_list = (PARAM *)sal_alloc(params_list_size, "Dune CLI: params_list") ;
    memcpy(params_list,subject_ptr->allowed_params,params_list_size) ;
    subject_ptr->allowed_params = params_list ;
    /*
     * Sort parameters by name. Leave last in place.
     */
    qsort(
      &params_list[0],num_params,
      sizeof(PARAM),compare_params_by_name) ;
    parameter_ptr = &params_list[0] ;
    for (uj = 0, list_len = 0 ; uj < num_params ; uj++)
    {
      /*
       * Add 2 for CR/LF.
       */
      list_len += strlen(parameter_ptr->par_name) + 2 ;
      parameter_ptr++ ;
    }
    /*
     * Now load memory pointed by params_ascii_list.
     */
    parameter_ptr = &params_list[0] ;
    if (num_params == 0)
    {
      const
        char *empty_list_str = "Empty list!\r\n";

      params_ascii_list = (char *)sal_alloc(strlen(empty_list_str) + 1, "Dune CLI: params_ascii_list");
      strcpy(params_ascii_list, empty_list_str);
    }
    else
    {
      /*
       * Add space for terminating null.
       */
      params_ascii_list_len = list_len + 1;
      params_ascii_list = (char *)sal_alloc(params_ascii_list_len, "Dune CLI: params_ascii_list") ;
      /*
       * Length of this list is at least 1.
       * Fill with '0'. Just to make sure (only
       * one '0' is required.
       */
      memset(params_ascii_list,0,params_ascii_list_len) ;
    }
    subject_ptr->params_ascii_list = params_ascii_list ;
    /*
     * Now, for each parameter, copy list of variables
     * to RAM.
     */
    parameter_ptr = &params_list[0] ;
    for (uj = 0 ; uj < num_params ; uj++, parameter_ptr++)
    {
      PARAM_VAL_RULES
        *param_val_rules,
        *param_val_list ;
      unsigned
        int
          vals_list_size,
          num_vals ;
      char
        *vals_detailed_help;
      param_val_rules = parameter_ptr->allowed_vals ;
      /*
       * Assume there are less than MAX_VALUES_PER_PARAM values for each parameter
       */
      for (uk = 0, list_len = 0 ; uk < MAX_VALUES_PER_PARAM ; uk++,param_val_rules++)
      {
        if (param_val_rules->val_type == VAL_END_OF_LIST)
        {
          break ;
        }
      }
      num_vals = parameter_ptr->num_allowed_vals = uk ;
      /*
       * Add one to size of array in order to include the last
       * element which only indicates 'end of list'.
       */
      vals_list_size = (num_vals + 1) * sizeof(PARAM_VAL_RULES) ;
      /*
       * Work with ram pointer. This way, values can be added
       * dynamically, at later stages.
       */
      param_val_list = (PARAM_VAL_RULES *)sal_alloc(vals_list_size, "param_val_list") ;
      memcpy(param_val_list,parameter_ptr->allowed_vals,vals_list_size) ;
      /*
       * Sort values by name. Leave last in place.
       */
      qsort(
        &param_val_list[0],num_vals,
        sizeof(PARAM_VAL_RULES),compare_param_vals_by_name) ;
      parameter_ptr->allowed_vals = param_val_list ;
      /*
       * Assume that help, on all variables related to this
       * parameter, take less than MAX_CHAR_PER_HELP_PER_VALUE characters per value.
       */
      help_buffer = (char *)sal_alloc(MAX_VALUES_PER_PARAM * MAX_CHAR_PER_HELP_PER_VALUE, "help_buffer") ;
      help_buffer[0] = 0 ;
      param_val_rules = parameter_ptr->allowed_vals ;
      if (num_vals == 0)
      {
        UTILS_STRCAT_SAFE(help_buffer,">>> There are no values for this parameter!\r\n", MAX_VALUES_PER_PARAM * MAX_CHAR_PER_HELP_PER_VALUE);
      }
      /*
       * Just making sure...
       */
      parameter_ptr->num_numeric_vals = 0 ;
      parameter_ptr->num_symbolic_vals = 0 ;
      parameter_ptr->num_text_vals = 0 ;
      parameter_ptr->num_ip_vals = 0 ;
      for (uk = 0 ; uk < num_vals ; uk++,param_val_rules++)
      {
        char
          index_str[20] ;
        sal_sprintf(index_str,"%02u) ",(unsigned short)(uk + 1)) ;
        UTILS_STRCAT_SAFE(help_buffer,index_str, MAX_VALUES_PER_PARAM * MAX_CHAR_PER_HELP_PER_VALUE);
        switch (param_val_rules->val_type & VAL_TYPES_MASK)
        {
          case VAL_NUMERIC:
          {
            VAL_NUM_DESCRIPTOR
              *val_num_descriptor ;
            parameter_ptr->num_numeric_vals++ ;
            /*
             * There can only be one numeric value per one parameter!
             */
            if (parameter_ptr->num_numeric_vals > 1)
            {
              char
                err_msg[80] ;
              sal_sprintf(err_msg,
                  "More than one numeric value for parameter %s",
                  parameter_ptr->par_name) ;
              gen_err(FALSE,TRUE,(int)0,ERROR,
                    err_msg,proc_name,SVR_FTL,
                      INIT_UI_INTERPRETER_ERR_02,FALSE,0,-1,FALSE) ;
              break ;
            }
            parameter_ptr->numeric_index = uk ;
            UTILS_STRCAT_SAFE(help_buffer," Numeric variable.", MAX_VALUES_PER_PARAM * MAX_CHAR_PER_HELP_PER_VALUE);
            val_num_descriptor = &param_val_rules->val_descriptor.val_num_descriptor ;
            if (val_num_descriptor->val_numeric_attributes & POSITIVE_VALUE)
            {
              UTILS_STRCAT_SAFE(help_buffer," Positive.", MAX_VALUES_PER_PARAM * MAX_CHAR_PER_HELP_PER_VALUE);
            }
            if (val_num_descriptor->val_numeric_attributes & NEGATIVE_VALUE)
            {
              UTILS_STRCAT_SAFE(help_buffer," Negative.", MAX_VALUES_PER_PARAM * MAX_CHAR_PER_HELP_PER_VALUE);
            }
            if (val_num_descriptor->val_numeric_attributes & HAS_MIN_VALUE)
            {
              char
                numeric[UI_NUMERIC_STR_SIZE] ;
              UTILS_STRCAT_SAFE(help_buffer," Min val:", MAX_VALUES_PER_PARAM * MAX_CHAR_PER_HELP_PER_VALUE);
              sal_sprintf(numeric," %ld (0x%lX).",
                  val_num_descriptor->val_min,
                  (unsigned long)(val_num_descriptor->val_min)) ;
              UTILS_STRCAT_SAFE(help_buffer,numeric, MAX_VALUES_PER_PARAM * MAX_CHAR_PER_HELP_PER_VALUE);
            }
            else
            {
              UTILS_STRCAT_SAFE(help_buffer," No minimum.", MAX_VALUES_PER_PARAM * MAX_CHAR_PER_HELP_PER_VALUE);
            }
            if (val_num_descriptor->val_numeric_attributes & HAS_MAX_VALUE)
            {
              char
                numeric[UI_NUMERIC_STR_SIZE] ;
              UTILS_STRCAT_SAFE(help_buffer," Max val:", MAX_VALUES_PER_PARAM * MAX_CHAR_PER_HELP_PER_VALUE);

              sal_sprintf(numeric," %ld (0x%lX).",
                      val_num_descriptor->val_max,
                      (unsigned long)val_num_descriptor->val_max) ;
              UTILS_STRCAT_SAFE(help_buffer,numeric, MAX_VALUES_PER_PARAM * MAX_CHAR_PER_HELP_PER_VALUE);
            }
            else
            {
              UTILS_STRCAT_SAFE(help_buffer," No maximum.", MAX_VALUES_PER_PARAM * MAX_CHAR_PER_HELP_PER_VALUE);
            }

            if (val_num_descriptor->max_num_repeated)
            {
              char
                numeric[UI_NUMERIC_STR_SIZE] ;
              numeric[0] = '\0';
              if (val_num_descriptor->max_num_repeated == 1)
              {
                if(get_ui_do_long_error_printing())
                {
                  sal_sprintf(numeric," May be repeated once.") ;
                }
              }
              else
              {
                sal_sprintf(numeric," May be repeated %lu times.",val_num_descriptor->max_num_repeated) ;
              }
              UTILS_STRCAT_SAFE(help_buffer,numeric, MAX_VALUES_PER_PARAM * MAX_CHAR_PER_HELP_PER_VALUE);
            }
            if (val_num_descriptor->val_checker)
            {
              char
                numeric[UI_NUMERIC_STR_SIZE] ;
              UTILS_STRCAT_SAFE(help_buffer,"\r\n    Has validating procedure (at address ", MAX_VALUES_PER_PARAM * MAX_CHAR_PER_HELP_PER_VALUE);
              sal_sprintf(numeric,"0x%08lX).",(unsigned long)val_num_descriptor->val_checker) ;
              UTILS_STRCAT_SAFE(help_buffer,numeric, MAX_VALUES_PER_PARAM * MAX_CHAR_PER_HELP_PER_VALUE);
            }
            else if(get_ui_do_long_error_printing())
            {
              UTILS_STRCAT_SAFE(help_buffer,"\r\n    No validating procedure. ", MAX_VALUES_PER_PARAM * MAX_CHAR_PER_HELP_PER_VALUE);
            }
            break ;
          }
          case VAL_SYMBOL:
          {
            parameter_ptr->num_symbolic_vals++ ;
            UTILS_STRCAT_SAFE(help_buffer,"String variable: \'", MAX_VALUES_PER_PARAM * MAX_CHAR_PER_HELP_PER_VALUE);
            UTILS_STRCAT_SAFE(help_buffer,param_val_rules->symb_val, MAX_VALUES_PER_PARAM * MAX_CHAR_PER_HELP_PER_VALUE);
            UTILS_STRCAT_SAFE(help_buffer,"\' .", MAX_VALUES_PER_PARAM * MAX_CHAR_PER_HELP_PER_VALUE);
            if (strlen(param_val_rules->val_descriptor.val_symb_descriptor.plain_text_help))
            {
              UTILS_STRCAT_SAFE(help_buffer,"\r\n", MAX_VALUES_PER_PARAM * MAX_CHAR_PER_HELP_PER_VALUE);

              UTILS_STRCAT_SAFE(
                help_buffer,
                param_val_rules->val_descriptor.val_symb_descriptor.plain_text_help,
                MAX_VALUES_PER_PARAM * MAX_CHAR_PER_HELP_PER_VALUE
              );
            }
            break ;
          }
          case VAL_TEXT:
          {
            char
              numeric[UI_NUMERIC_STR_SIZE] ;
            parameter_ptr->num_text_vals++ ;
            /*
             * There can only be one free text value per one parameter and
             * other parameter values are not allowed in that case!
             */
            if (parameter_ptr->num_allowed_vals > 1)
            {
              char
                err_msg[80] ;
              sal_sprintf(err_msg,
                  "More than one value in addition to free text value for parameter %s",
                  parameter_ptr->par_name) ;
              gen_err(FALSE,TRUE,(int)0,ERROR,
                    err_msg,proc_name,SVR_FTL,
                      INIT_UI_INTERPRETER_ERR_03,FALSE,0,-1,FALSE) ;
              break ;
            }
            parameter_ptr->text_index = uk ;
            UTILS_STRCAT_SAFE(help_buffer,"Free text variable: Up to ", MAX_VALUES_PER_PARAM * MAX_CHAR_PER_HELP_PER_VALUE);
            sal_sprintf(numeric,"%d characters",
                param_val_rules->
                        val_descriptor.val_text_descriptor.max_num_chars) ;
            UTILS_STRCAT_SAFE(help_buffer,numeric, MAX_VALUES_PER_PARAM * MAX_CHAR_PER_HELP_PER_VALUE);

            if (strlen(param_val_rules->val_descriptor.val_text_descriptor.plain_text_help))
            {
              UTILS_STRCAT_SAFE(help_buffer,"\r\n", MAX_VALUES_PER_PARAM * MAX_CHAR_PER_HELP_PER_VALUE);

              UTILS_STRCAT_SAFE(
                help_buffer,
                param_val_rules->val_descriptor.val_text_descriptor.plain_text_help,
                MAX_VALUES_PER_PARAM * MAX_CHAR_PER_HELP_PER_VALUE
              );
            }
            break ;
          }
          case VAL_IP:
          {
            parameter_ptr->num_ip_vals++ ;
            /*
             * There can only be one IP address value per one parameter and
             * other parameter values are not allowed in that case!
             */
            if (parameter_ptr->num_allowed_vals > 1)
            {
              char
                err_msg[80] ;
              sal_sprintf(err_msg,
                  "More than one value in addition to IP address value for parameter %s",
                  parameter_ptr->par_name) ;
              gen_err(FALSE,TRUE,(int)0,ERROR,
                    err_msg,proc_name,SVR_FTL,
                      INIT_UI_INTERPRETER_ERR_04,FALSE,0,-1,FALSE) ;
              break ;
            }
            parameter_ptr->ip_index = uk ;
            UTILS_STRCAT_SAFE(help_buffer,"IP address variable ", MAX_VALUES_PER_PARAM * MAX_CHAR_PER_HELP_PER_VALUE);
            break ;
          }
          default:
          {
            gen_err(FALSE,TRUE,(int)0,ERROR,
                  "Illegal parameter value - ",
                          proc_name,SVR_FTL,
                          INIT_UI_INTERPRETER_ERR_01,FALSE,0,-1,FALSE) ;
            break ;
          }
        }
        UTILS_STRCAT_SAFE(help_buffer,"\r\n", MAX_VALUES_PER_PARAM * MAX_CHAR_PER_HELP_PER_VALUE);
      }
      if ((parameter_ptr->default_care & HAS_DEFAULT_MASK) == HAS_DEFAULT)
      {
        char
          buffer[80] ;
        char
          ip_addr[MAX_IP_STRING] ;

        buffer[0] = 0 ;

        UTILS_STRCAT_SAFE(help_buffer,">>Has default value: ", MAX_VALUES_PER_PARAM * MAX_CHAR_PER_HELP_PER_VALUE);
        if ((parameter_ptr->default_val->val_type & VAL_NUMERIC) == VAL_NUMERIC)
        {
          UTILS_STRCAT_SAFE(help_buffer,"Numeric, ", MAX_VALUES_PER_PARAM * MAX_CHAR_PER_HELP_PER_VALUE);
          sal_sprintf(buffer,"Value: %lu",
                    parameter_ptr->default_val->value.ulong_value) ;
        }
        else if ((parameter_ptr->default_val->val_type & VAL_SYMBOL) == VAL_SYMBOL)
        {
          UTILS_STRCAT_SAFE(help_buffer,"Symbolic, ", MAX_VALUES_PER_PARAM * MAX_CHAR_PER_HELP_PER_VALUE);
          sal_sprintf(buffer,"Value: \'%s\'",
                  parameter_ptr->default_val->value.string_value) ;
        }
        else if ((parameter_ptr->default_val->val_type & VAL_TEXT) == VAL_TEXT)
        {
          UTILS_STRCAT_SAFE(help_buffer,"Free text, ", MAX_VALUES_PER_PARAM * MAX_CHAR_PER_HELP_PER_VALUE);
          sal_sprintf(buffer,"Value: \'%s\'",
                  parameter_ptr->default_val->value.val_text) ;
        }
        else if ((parameter_ptr->default_val->val_type & VAL_IP) == VAL_IP)
        {
          UTILS_STRCAT_SAFE(help_buffer,"IP address, Value: ", MAX_VALUES_PER_PARAM * MAX_CHAR_PER_HELP_PER_VALUE);
          sprint_ip(ip_addr,parameter_ptr->default_val->value.ip_value, FALSE) ;
          UTILS_STRCAT_SAFE(buffer,ip_addr, 80);
        }
        else
        {
          sal_sprintf(buffer,"Unknown type") ;
        }
        UTILS_STRCAT_SAFE(help_buffer,buffer, MAX_VALUES_PER_PARAM * MAX_CHAR_PER_HELP_PER_VALUE);
      }
      else if(get_ui_do_long_error_printing())
      {
        UTILS_STRCAT_SAFE(help_buffer,">>> No default value for this item.", MAX_VALUES_PER_PARAM * MAX_CHAR_PER_HELP_PER_VALUE);
      }
      UTILS_STRCAT_SAFE(help_buffer,"\r\n", MAX_VALUES_PER_PARAM * MAX_CHAR_PER_HELP_PER_VALUE);
      list_len = strlen(help_buffer) ;
      vals_detailed_help = (char *)sal_alloc(list_len + 1, "vals_detailed_help") ;
      strcpy(vals_detailed_help,help_buffer) ;
      sal_free(help_buffer) ;
      help_buffer = NULL;
      parameter_ptr->vals_detailed_help = vals_detailed_help ;
    }
  }

  /*
   * Allocate memory and initialize history fifo.
   */
  init_history_fifo() ;
#if DUNE_NATIVE
#ifdef __VXWORKS__ 
  /*
   * Allocate memory and initialize memory access handling.
   */
  init_mem_db() ;
#endif /* __VXWORKS__ */
#else
#endif /* DUNE_NATIVE */

  /*
   * Make sure echo mode is on, by default. Redundant.
   */
  set_echo_mode_on() ;

#ifndef __VXWORKS__
exit:
  if(help_buffer!=NULL)
  {
    sal_free(help_buffer);
  }
#endif
  return ;
}
/*****************************************************
*NAME
*  init_ui_module
*TYPE: PROC
*DATE: 16/JAN/2002
*FUNCTION:
*  All initializations related to user interface module.
*CALLING SEQUENCE:
*  init_ui_module()
*INPUT:
*  SOC_SAND_DIRECT:
*    None.
*  SOC_SAND_INDIRECT:
*    Various globals. Can not be specific here.
*OUTPUT:
*  SOC_SAND_DIRECT:
*    None.
*  SOC_SAND_INDIRECT:
*    Initialized module. Started task.
*REMARKS:
*  NONE.
*SEE ALSO:
*****************************************************/
void
   init_ui_module(
 void
)
{
#if defined(__VXWORKS__) && !defined(__DUNE_GTO_BCM_CPU__)
  int
    err ;
  STATUS
    status ;
  char
    screen_msg[80] ;
  char
    ascii_ver_app[7],
  ascii_ver_boot[7];
#endif /* __VXWORKS__ */

#if defined(__VXWORKS__) && !defined(__DUNE_GTO_BCM_CPU__)
/* { */
  if (sizeof(BLOCK_05) > SIZEOF_EE_BLOCK_05)
  {
    gen_err(FALSE,FALSE,(int)err,0,
          "NVRAM event log file defined too big ",
          proc_name,SVR_FTL,INIT_UI_MODULE_ERR_06,TRUE,0,-1,FALSE) ;
  }
  if (NUM_EVENT_LOG_ITEMS >= BIT(BITS_IN_BYTE))
  {
    gen_err(FALSE,FALSE,(int)err,0,
          "NVRAM event log array with too many elements ",
          proc_name,SVR_FTL,INIT_UI_MODULE_ERR_07,TRUE,0,-1,FALSE) ;
  }
  /*
   * Make sure all blocks under user control are
   * loaded with meaningful values.
   */
#if !DUNE_BCM
  err = load_defaults_conditional(DEFAULTS_ALL_EQUIVALENT) ;
#else
  d_printf("skipping load_defaults_conditional\n\r");
  err = 0;
#endif
  gen_err(TRUE,FALSE,(int)err,0,
          "load_defaults_conditional ",
          proc_name,SVR_ERR,INIT_UI_MODULE_ERR_05,FALSE,0,-1,FALSE) ;
  /*
   * Load run-time image of NV ram (run-time values)
   */
#if !DUNE_BCM
  load_run_vals() ;
#else
  d_printf("skipping load_run_vals\n\r");
#endif
  {
    char
      *addr_string ;
    struct in_addr
      inet_address ;
    STATUS
      status ;
    unsigned
      long
        ip_addr ;
    unsigned
      int
        size,
        offset ;
    /*
     * As an exception, run time IP address may not be an image
     * of NV ram since IP may be taken from combination of
     * NV ram value and slot id (SEE loadBootParams()). Make sure to update
     * it from currently loaded IP address.
     * We could also use here
     *  status = ifAddrGet(BOOT_DEV_NAME"0",&interfaceAddress[0]) ;
     */
    addr_string = get_addr_string() ;
    status = inet_aton(addr_string,&inet_address) ;
    gen_err(TRUE,FALSE,(int)status,(int)OK,
          "inet_aton ",
          proc_name,SVR_ERR,INIT_UI_MODULE_ERR_08,FALSE,0,-1,FALSE) ;
    ip_addr = inet_address.s_addr ;
    offset = (unsigned int)((char *)&(((EE_AREA *)0)->
                    ee_block_02.un_ee_block_02.block_02.ip_addr)) ;
    size = sizeof(((EE_AREA *)0)->
                    ee_block_02.un_ee_block_02.block_02.ip_addr) ;
#if !DUNE_BCM
    err = set_run_vals((char *)(&ip_addr),(int)size,(int)offset) ;
#else
    err = 0;
    d_printf("skipping set_run_vals\n\r");
#endif
  }
  {
    /*
     * Initializations related to telnet timeout:
     */
#if !DUNE_BCM
    unsigned
      char
        val ;
    get_telnet_activity_timeout_std(&val) ;
    set_telnet_timeout_val((unsigned long)val) ;
    /*
     * Make sure telnet activity time base is set to prevent
     * early disconnection in case we come up with telnet on.
     */
    set_telnet_activity_time_mark() ;
#else
    d_printf("skipping telnet timeout\n\r");
#endif
  }
  {
    /*
     * For some reason (vXWorks bug ? ? ?) we have to first create
     * a dummy pipe when this application is downloaded
     * using (ld).
     */
    int
      user_fd ;
    status = pipeDevCreate("/pipe/dum",1000,2) ;
    gen_err(TRUE,TRUE,(int)status,ERROR,
          "init_ui_module - pipeDevCreate",
                  proc_name,SVR_FTL,
                  INIT_UI_MODULE_ERR_01,FALSE,0,-1,FALSE) ;
    user_fd = open("/pipe/dum",O_RDWR,0) ;
    gen_err(TRUE,TRUE,(int)user_fd,ERROR,
          "init_ui_module - open",
                  proc_name,SVR_FTL,
                  INIT_UI_MODULE_ERR_03,FALSE,0,-1,FALSE) ;
    /*
     * Create pipe to be used as a channel between user prep task
     * and user interface task. Make sure it is always open.
     * Note that max. number of messages in pipe (second parameters)
     * determines the depth of the 'type ahead buffer'. Make
     * it large to contain large command download files.
     */
    status = pipeDevCreate("/pipe/uif",50000,2) ;
    gen_err(TRUE,TRUE,(int)status,ERROR,
          "init_ui_module - pipeDevCreate",
                  proc_name,SVR_FTL,
                  INIT_UI_MODULE_ERR_01,FALSE,0,-1,FALSE) ;
    user_fd = open("/pipe/uif",O_RDWR,0) ;
    gen_err(TRUE,TRUE,(int)user_fd,ERROR,
          "init_ui_module - open",
                  proc_name,SVR_FTL,
                  INIT_UI_MODULE_ERR_03,FALSE,0,-1,FALSE) ;
#if !DUNE_BCM
    set_user_if_file_descriptor(user_fd) ;
#else
    d_printf("skipping set_user_if_file_descriptor\n\r");
#endif
  }

  {
    /*
     * Create pipe to be used as a channel between user interface
     * task and shell. Make sure it is always open.
     */
    int
      console_fd,
      shell_fd ;
    status = pipeDevCreate("/pipe/shell",100,3) ;
    gen_err(TRUE,TRUE,(int)status,ERROR,
          "init_ui_module - pipeDevCreate",
                  proc_name,SVR_FTL,
                  INIT_UI_MODULE_ERR_02,FALSE,0,-1,FALSE) ;
    shell_fd = open("/pipe/shell",O_RDWR,0) ;
    gen_err(TRUE,TRUE,(int)shell_fd,ERROR,
          "init_ui_module - pipeDevCreate",
                  proc_name,SVR_FTL,
                  INIT_UI_MODULE_ERR_04,FALSE,0,-1,FALSE) ;
#if !DUNE_BCM
    set_shell_file_descriptor(shell_fd) ;
#else
    d_printf("skipping set_shell_file_descriptor\n\r");
#endif
    /*
     * Remember the standard IO FD at startup. This is
     * necessary to get out of some telnet situations
     * (unsolicited disconnect).
     */
    console_fd = get_console_fd() ;
#if !DUNE_BCM
    set_startup_io_file_descriptor(console_fd) ;
#else
    d_printf("skipping set_startup_io_file_descriptor\n\r");
#endif
  }
/* } */
#endif
  /*
   * Initialize CLI file system.
   */
#if defined(__VXWORKS__) && !defined(__DUNE_GTO_BCM_CPU__)
/* { */
  init_cli_files() ;
/* } */
#endif
  /*
   * Validate UI input
   */
#if defined(__VXWORKS__) && !defined(__DUNE_GTO_BCM_CPU__)
  validate_ui_interpreter();
#endif
  /*
   * Initialize CLI interpreter.
   */
  init_ui_interpreter() ;
#if defined(__VXWORKS__) && !defined(__DUNE_GTO_BCM_CPU__)
/* { */
  sal_sprintf(screen_msg,
      "\r\n"
      "====== Starting CLI [VER   App %s Boot %s] ======"
      "\r\n",version_to_ascii(ascii_ver_app,get_prog_ver(),1), \
    version_to_ascii(ascii_ver_boot,get_dune_boot_version(),1)) ;
  d_printf(screen_msg) ;
/* } */ 
#endif
  return ;
}

/* Function dune_ui_module_free_bits prints the free bits that can be used to 
 * add new dune ui apis
*/

#include "soc/dpp/SAND/Utils/sand_bitstream.h"
#define SOC_SAND_NOF_BITS_IN_LONG 32

void
  dune_ui_module_free_bits()
{
      unsigned int
        subject_i,
        param_i,
        long_i,
        bit_i;
      uint32
        mutex_control[NUM_ELEMENTS_MUTEX_CONTROL] ;
      PARAM
        *param_ptr;

      send_string_to_screen("",TRUE) ;
      send_string_to_screen("Free space mark as ZERO in the bitmap",TRUE) ;
      for (subject_i=0; Subjects_list_rom[subject_i].subject_id!=SUBJECT_END_OF_LIST; ++subject_i)
      {
        /*
         * For every subject
         */
		  sal_printf("Subject Name: %s\n\r", Subjects_list_rom[subject_i].subj_name);
        soc_sand_bitstream_clear(mutex_control, NUM_ELEMENTS_MUTEX_CONTROL);

        param_ptr = Subjects_list_rom[subject_i].allowed_params;
        for (param_i=0; param_ptr->param_id != PARAM_END_OF_LIST; ++param_i, ++param_ptr)
        {
			soc_sand_bitstream_or(mutex_control, (uint32*)param_ptr->mutex_control, NUM_ELEMENTS_MUTEX_CONTROL);
        }

        for (long_i=0; long_i<NUM_ELEMENTS_MUTEX_CONTROL; ++long_i)
        {
          sal_printf("%u) {", long_i);
          if (mutex_control[long_i] == 0x0)
          {
            sal_printf(" All ");
          }
          else if (mutex_control[long_i] == 0xFFFFFFFF)
          {
            sal_printf(" None ");
          }
          else
          {
            for (bit_i=0; bit_i<SOC_SAND_NOF_BITS_IN_LONG; ++bit_i)
            {
              if ( ! ( BIT(bit_i) & mutex_control[long_i] ) )
              {
                sal_printf("%2u,", bit_i);
              }
            }
          }
          sal_printf("}\n\r");
        }
      }
      send_string_to_screen("",TRUE) ;

      for (subject_i=0; Subjects_list_rom[subject_i].subject_id!=SUBJECT_END_OF_LIST; ++subject_i)
      {
        unsigned long
          nof_params;
        nof_params = 0;

        param_ptr = Subjects_list_rom[subject_i].allowed_params;
        for (param_i=0; param_ptr->param_id != PARAM_END_OF_LIST; ++param_i, ++param_ptr)
        {
          nof_params++;
        }

        sal_printf(
          "Subject Name: %s, Number of parameters %lu \n\r",
          Subjects_list_rom[subject_i].subj_name,
          nof_params
        );
        if (nof_params>=MAX_PARAMS_PER_SUBJECT)
        {
          sal_printf(
            "Error More than Max (%u) \n\r",
            MAX_PARAMS_PER_SUBJECT
          );
        }


      }
      send_string_to_screen("",TRUE) ;
}
