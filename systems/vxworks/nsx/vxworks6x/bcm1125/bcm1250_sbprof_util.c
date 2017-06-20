/* $Id: bcm1250_sbprof_util.c,v 1.3 2011/07/21 16:14:49 yshtil Exp $ */
#include <assert.h>
#include <stdlib.h>

#include <unistd.h>

#include <sockLib.h>
#include <inetLib.h>                                                            

#define ntoh_swap2(x) hton_swap2(x)
#define ntoh_swap4(x) hton_swap4(x)
#define ntoh_swap8(x) hton_swap8(x)
                                                                                
#define PLATFORM_STRING "vxworks"
 
#include "vxWorks.h"
                                                                                
typedef unsigned long long  u8;
typedef UINT32 u4;
typedef UINT16 u2;
typedef UINT8  u1;
 
typedef long long  i8;
typedef INT32 i4;
typedef INT16 i2;
typedef INT8  i1;
 
/* VxWorks is brain-dead -- it has no 64-bit formats 8-( */
#define FMT8 "l"
 
#if defined(__MIPSEL__)
 
#define hton_swap2(x) { u2 *_p_ = (x); \
                       *_p_ = htons(*_p_); }
#define hton_swap4(x) { u4 *_p_ = (x); \
                       *_p_ = htonl(*_p_); }
#define hton_swap8(x) { u4 *_p_ = (u4 *) (x); \
                       u4 _tmp0_ = htonl(_p_[1]); \
                       u4 _tmp1_ = htonl(_p_[0]); \
                       _p_[0] = _tmp0_; \
                       _p_[1] = _tmp1_; }
 
#elif defined(__MIPSEB__)
 
#define _HOST_IS_BIG_ENDIAN
 
/* Host byte order matches network byte order */
#define hton_swap2(x)
#define hton_swap4(x)
#define hton_swap8(x)
 
#else
#error "unexpected byte order"
#endif                                                                          

#undef MALLOC
#define MALLOC(typ) ((typ *) safe_malloc(sizeof(typ), __FILE__, __LINE__))
#define AMALLOC(typ,n) \
                    ((typ *) safe_malloc((n)*sizeof(typ), __FILE__, __LINE__))
#define fatal(msg) fatal_msg((msg), __FILE__, __LINE__)
#define fatal_errno(msg) fatal_errno_msg((msg), __FILE__, __LINE__)
 
#define CALLOC(typ) ((typ *) safe_calloc(sizeof(typ), __FILE__, __LINE__))
#define ACALLOC(typ,n) \
                    ((typ *) safe_calloc((n)*sizeof(typ), __FILE__, __LINE__))
 
void fatal_msg(char *message, char *fname, unsigned int line);
void fatal_errno_msg(char *message, char *fname, unsigned int line);
/* Outputs description of errno plus message */
 
static __inline__
void *safe_malloc(unsigned int size, char *file, unsigned int line)
{
  void *res = malloc(size);
  if (res == NULL) fatal_msg("MALLOC failed", file, line);
  return res;
}
 
static __inline__
void *safe_calloc(unsigned int size, char *file, unsigned int line)
{
  void *res = calloc(1, size);
  if (res == NULL) fatal_msg("CALLOC failed", file, line);
  return res;
}                                                                               

static __inline__ u1 u1min(u1 x, u1 y) { return x < y ? x : y; }
static __inline__ u2 u2min(u2 x, u2 y) { return x < y ? x : y; }
static __inline__ u4 u4min(u4 x, u4 y) { return x < y ? x : y; }
static __inline__ u8 u8min(u8 x, u8 y) { return x < y ? x : y; }
 
static __inline__ u1 u1max(u1 x, u1 y) { return x > y ? x : y; }
static __inline__ u2 u2max(u2 x, u2 y) { return x > y ? x : y; }
static __inline__ u4 u4max(u4 x, u4 y) { return x > y ? x : y; }
static __inline__ u8 u8max(u8 x, u8 y) { return x > y ? x : y; }
 
typedef u1 bool;
#ifndef TRUE
#define TRUE (0 == 0)
#endif
#ifndef FALSE
#define FALSE (0 == 1)
#endif
 
 
static __inline__
void swap(u1 *a, u1 *b)
{
  u1 av = *a;
  u1 bv = *b;
  *a = bv;
  *b = av;
}
 
static __inline__
void bswap2(u2 *orig)
{
  u1 *p = (u1 *) orig;
  swap(p+0, p+1);
}
                                                                                static __inline__
void bswap4(u4 *orig)
{
  u1 *p = (u1 *) orig;
  swap(p+0, p+3);
  swap(p+1, p+2);
}
 
static __inline__
void bswap8(u8 *orig)
{
  u1 *p = (u1 *) orig;
  swap(p+0, p+7);
  swap(p+1, p+6);
  swap(p+2, p+5);
  swap(p+3, p+4);
}
                                                                                
typedef u1 perf_cnt_arch_t;
typedef u1 sbprof_event_t;
                                                                                
typedef enum {
  PERF_CNT_ARCH_SB1,            /* Pass 1 SB-1 */
  PERF_CNT_ARCH_AMD_ATHLON,
  PERF_CNT_ARCH_INTEL_P6,
  PERF_CNT_ARCH_SB1250,         /* SCD performance counters */
  PERF_CNT_ARCH_SB1R2,          /* Pass 2 SB-1 (PTR reg, new events, etc.) */
  PERF_CNT_ARCH_LAST
} perf_cnt_arch_val;
 
/* Maximum number of counters on a single architecture */
#define MAX_COUNTERS 4
 
#define EVENT_NONE ((event_t) -1)
#define EVENT_MAX  ((event_t) -2)
                                                                                
typedef enum {
  DP_G_PCARCH,  /* request pcarch, os, endianness, pc alignment, ... */
  DP_G_START,   /* start gathering data (describes which events to monitor) */
  DP_G_STOP,    /* stop gathering data */
  DP_G_CLOSE,   /* close connection, decided not to profile */
  DB_G_LAST
} dp_gatherer_msg;
 
/* Messages from driver to gatherer */
typedef enum {
  DP_D_PCARCH,  /* response to DP_G_PCARCH  */
  DP_D_STOPPED, /* ack to STOP request, reports event totals */
  DP_D_PID_IMAGE, /* reports an image loaded in the address space of a process
                   * Any previous for pid images are discarded.  E.g., use
                   * in Unix when a process calls exec().  Pid may or may
                   * already have images defined. */
  DP_D_PID_DEAD,/* reports the death of a process (address space) */
  DP_D_CONTEXT, /* new context for subsequent data messages
                 * cpuid + pid + ... */
  DP_D_EVENTS,  /* pc samples for events */
  DP_D_LAST
} dp_driver_msg;
 
#define PROF_DEFAULT_PORT 1250
 
typedef enum {
  OS_LINUX,
  OS_NETBSD,
  OS_ICOS,
  OS_VXWORKS,
  OS_AMECFE,
  OS_LAST
} os_t;                                                                         

typedef int comm_t; 
 
void fatal_msg(char *message, char *fname, unsigned int line)
{
  fprintf(stderr, "%s file %s line %d\n", message, fname, line);
  exit(1);
}
 
void fatal_errno_msg(char *message, char *fname, unsigned int line)
{
  perror(message);
  fprintf(stderr, "fatal error; file %s line %d\n", fname, line);
  exit(1);
}                                                                               

comm_t create_listener(u2 port)
{
  int s;
  struct sockaddr_in sa;
  memset(&sa, 0, sizeof(sa));
  s = socket(AF_INET, SOCK_STREAM, 0);
  if (s == ERROR) fatal_errno("socket failed");
  sa.sin_family = AF_INET;
  sa.sin_port = htons(port);
  sa.sin_len = sizeof(sa);
  sa.sin_addr.s_addr = htonl(INADDR_ANY);
 
  if (bind(s, (struct sockaddr *) &sa, sizeof(sa)) == ERROR)
    fatal_errno("bind failed");
  if (listen(s, 0) == ERROR)
    fatal_errno("listen failed");
  return s;
}                                                                               

void comm_send(comm_t comm, u1 *buf, u4 len)
{
  while (len > 0) {
    ssize_t n = write(comm, (void *) buf, (size_t) len);
    if (n == -1) {
      if (errno == EINTR || errno == EAGAIN) {
        continue;
      }
      fatal_errno("write failed");
    }
    len -= n;
    buf += n;
  }
}
 
void comm_recv(comm_t comm, u1 *buf, u4 len)
{
  while (len > 0) {
    ssize_t n = read(comm, (void *) buf, (size_t) len);
    if (n == 0) fatal("unexpected EOF");
    if (n == (ssize_t) -1) {
      if (errno == EINTR && errno == EAGAIN) {
        continue;
      }
      fatal_errno("read failed");
    }
    len -= n;
    buf += n;
  }
}                                                                               

static __inline__
void comm_close(comm_t comm)
     /* Opaque */
{
  close(comm);
}                                                                               

