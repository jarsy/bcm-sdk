/* $Id: bcm1250_sbprof.c,v 1.3 2011/07/21 16:14:49 yshtil Exp $
 *  Copyright (c) 2002
 *  Broadcom Corporation, Irvine, CA
 *
 *  BROADCOM PROPRIETARY AND CONFIDENTIAL
 *  
 *  This software is furnished under a Broadcom license agreement
 *  and may only be used in accordance with and subject to the terms 
 *  and conditions of such license. 
 *
 * Author: Mark Vandevoorde
 */

/*
 * A VxWorks Performance Counter Driver/Daemon for the BCM1250 SB-1 CPUs.
 */

/*#include "port.h"*/
/*#include "util.h"*/
#undef MALLOC	/* conflicts with inetLib.h -> .... -> mbuf.h !! */
#undef AMALLOC
#undef CALLOC
#undef ACALLOC
/*#include "event_comm.h"*/
/*#include "dataprot.h"*/
/*#include "comm.h"*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <intLib.h>
#include <errno.h>
#include <taskLib.h>
#include <semLib.h>
#include <logLib.h>

/* Rather than linking separate files, bring the other .c files here. */

/*#include "util.c"*/
/*#include "comm.c"*/
#include "bcm1250_sbprof_util.c"

#define panic(msg) do {logMsg(msg,0,1,2,3,4,5); exit(1); } while (0)


#define MAXCPUS 1
#define NEWPIDSIZE 160
/* The two definitions below must match those in pgather.c */
#define MAX_COUNTERS 4
#define MAX_SLOTS 8

typedef struct {
  u1  event;
  u1  hwcode;     /* pcarch-specific bits identifying desired event */
  u8 period;     /* 40-bit sampling period during slot */
} sbprof_mux_counter_t;

typedef struct {
  u1	n;			/* number of counters to use in slot */
  u1      use_ptr;                /* counter 1 events get pc from PTR */
  sbprof_mux_counter_t
                counter[MAX_COUNTERS];	/* data for setting up counter */
} sbprof_mux_slot_t;

typedef struct {
  sbprof_mux_slot_t slots[MAX_SLOTS];
} sbprof_mux_slots;

typedef struct {
  u8 total;	/* total number of interrupts */
  u8 dropped;	/* total interrupts when buffers were full */
  u8 value;	/* initial counter value when slot next entered */
  u4 start_period_low;	/* counter val for starting a period */
  u1  start_period_high;
  u1  event;      /* pcarch-specific event_t */
  u1  hwcode;	/* pcarch-specific code for event */
  u1  inuse;	/* Is counter X slot actually used? */
} event_counter_state_t;

typedef struct {
  u4 total;	 /* total for current run of slot */
  u4 dropped;	 /* dropped for current run of slot */
  u8 start_period;/* counter value to start a full period */
} active_counter_state_t;

struct _cpudata1 {
  /******* page-aligned boundary *********************************************/
  u4 last_pid;	/* Pid for last sample in buffer (-1 initially) */
  u1  curbuf;	/* 2 -> both buffers full at end of last interrupt
			   1 -> use buffer[1]
			   0 -> use buffer[0] */
  u1  nextbuf;	/* the index of the next buffer to be filled */
  u1  nnewpid[2]; /* number of entries set in new_pid[i] */
  u4 next;       /* index of next free entry in curbuf */
  INT32  last_event; /* index of byte just past last event DP_D_EVENTS
			   message in curbuf that needs the number of events
			   filled in.  -1 means there is no such message and
			   that such a message must be entered before
			   adding event samples.
			 */
  /* 16-byte boundary */
  volatile u4 full[2];
                        /* full[i] > 0 means buffer[i] needs emptying.
			   0 to nonzero done by sbprofintr().
			   nonzero to zero by sbprofioctl().
			   When nonzero, full[i] is the number of bytes
			   set in buffer[i].
			*/
  u4 threshold;  /* when does the current multiplexing slot expire? */
  u1 slotid;      /* index into slots[] of current multiplexing slot */
  u1 nslots;	/* number of slots */
  u1 needs_scan;
  u1 use_ptr;     /* nonzero IFF counter 1 events get PC from PTR */
  /******* 32-byte boundary *********************************************/
  active_counter_state_t cur_slot[MAX_COUNTERS];
  /******* 32-byte boundary *********************************************/
  event_counter_state_t event_counter_state[MAX_SLOTS][MAX_COUNTERS];
  /******* 32-byte boundary *********************************************/
  u1 slot_use_ptr[MAX_SLOTS];

  u4 newpid[2][NEWPIDSIZE];  /* new_pid[i][] contains indices of
				       buffer[i][] where a new_pid message
				       is encoded and needs to have the
				       image_id, base addr, and num_inst
				       fields set by the user-level daemon */
  SEM_ID semSync[2];	/* binary semaphores used to sync between interrupt
			   handler and task that drains buffers */
};

#define SBPROF_BUFSIZE (64*1024 - (sizeof(struct _cpudata1)+1)/2)

typedef struct _cpudata {
  struct _cpudata1 x;
  u1 buffer[2][SBPROF_BUFSIZE];
} cpudata_t;

typedef struct _chiprev {
  u8 system_rev;	/* system_rev register (0x1002000) */
  u4 cpuid;      /* CP0 reg $15,0 (Processor Id Rev) of a cpu */
  u1 major;       /* driver major version number */
  u1 minor;       /* driver minor version number */
  u1 pad[2];
} chiprev_t;

#if NEWPIDSIZE > 254
#error "newpidsize too big"
#endif


static void record_sample(u4, u4, u1, u1);
static void stop_intr(void);
static void unarm_counters(void);
static void slot_enter(cpudata_t *, u1, u4, u1);
static void slot_exit(cpudata_t *, u1, u8 *);

static void enter_event_hdr(cpudata_t *, u1 *);
static void enter_context(cpudata_t *, u4, u1);
static void backfill_nevents(cpudata_t *);
static int find_buffer(cpudata_t *);
static int swap_buffers(cpudata_t *);
static int report_new_context(cpudata_t *, u4, u1);

#define EVENT_HDR_MSG_SIZE 4         /* number of u1's per msg */
#define EVENT_MSG_SIZE 4             /* number of u1's per msg */
#define NEW_CONTEXT_MSG_SIZE 8      /* number of u1's per msg */
#define MAX_IMAGE_NAME 256           /* in 8-bit bytes */
#define NEW_PID_MSG_SIZE (28 + MAX_IMAGE_NAME)

/***********/
/* Globals */
/***********/

static int stopped = 1;

/* Hack to allocate page-aligned cpu data that is pinned in memory.
   We could call an allocator, but then we'd get an extra cache miss per
   interupt just to dereference the pointer. */

static void alloc_space(void);

static void alloc_space()
{
  __asm__ __volatile__ ("	.data;"
			"	.align 5;"
			"	.globl _sbprof_space;"
			"_sb1250_prof_space: .space %0;"
			"	.text;"
			: /* no output */
			: "i" (sizeof(cpudata_t) * MAXCPUS));
  /* Silence compiler warning that this procedure is never used. */
  alloc_space();
}

extern cpudata_t _sb1250_prof_space[MAXCPUS];

#define cpudata _sb1250_prof_space

static __inline__ u8 u64max(u8 a, u8 b) {
  return (a > b) ? a : b;
}

/* Writes to cp0 regs happen at pipestage 8; interrupts at pipestage 4.
   3 or 4 cycles of delay are needed between writing a cp0 reg to disable
   interrupts or a particular counter before the write takes effect. */
#define PAUSE __asm__ __volatile__ \
        (".set push; .set mips64; ssnop; ssnop; ssnop; ssnop; .set pop;")

#define PERIOD_2_CNTVALUE(p) ((- (i8) (p)) & 0x000000ffffffffffULL)

/* Puts the current count of cpu counter "counter_num" into "*dest" */
/* vxWorks doesn't support "select" field of mt/mfc0 inst, hence the
   .word kludge for "dmfc0 $8,$25,%1;" */
#define READ_COUNTER(counter_num,dest)    \
  __asm__ __volatile__ (".set push;"	  \
			".set mips64;"	  \
                        ".set noreorder;" \
           		"  .word (0x10 << 26) | (1 << 21) | (8 << 16) | (25 << 11) | (0xf & %1);\n" \
			"sd    $8,0(%0);" \
                        ".set pop;"	  \
			:/* no output */ \
			: "r" (dest), "i" ((counter_num)*2+1) \
			:/* modifies */ "$8");

/* Puts "*val" into the current count of cpu counter "counter_num" */
/* vxWorks doesn't support "select" field of mt/mfc0 inst, hence the
   .word kludge for "dmtc0 $4,$25,%1;" */
#define WRITE_COUNTER(counter_num,val)    \
  __asm__ __volatile__ (".set push;"	  \
			".set mips64;"	  \
                        ".set noreorder;" \
			"ld    $4,0(%0);" \
           		"  .word (0x10 << 26) | (5 << 21) | (4 << 16) | (25 << 11) | (0xf & %1);\n" \
                        ".set pop;"	  \
			:/* no output */  \
			: "r" (val), "i" ((counter_num)*2+1) \
			:/* modifies */ "$4");

/* Puts "val" into the current control reg of cpu counter "counter_num" */
/* vxWorks doesn't support "select" field of mt/mfc0 inst, hence the
   .word kludge for "mtc0 %0,$25,%1;" */
#define WRITE_COUNTER_SEL(counter_num,val) \
  __asm__ __volatile__ (".set push;"	  \
			".set mips64;"	  \
           		"SBPROF_BASE_REG$0 = 0;\n" \
           		"SBPROF_BASE_REG$1 = 1;\n" \
           		"SBPROF_BASE_REG$2 = 2;\n" \
           		"SBPROF_BASE_REG$3 = 3;\n" \
           		"SBPROF_BASE_REG$4 = 4;\n" \
           		"SBPROF_BASE_REG$5 = 5;\n" \
           		"SBPROF_BASE_REG$6 = 6;\n" \
           		"SBPROF_BASE_REG$7 = 7;\n" \
           		"SBPROF_BASE_REG$8 = 8;\n" \
           		"SBPROF_BASE_REG$9 = 9;\n" \
           		"SBPROF_BASE_REG$10 = 10;\n" \
           		"SBPROF_BASE_REG$11 = 11;\n" \
           		"SBPROF_BASE_REG$12 = 12;\n" \
           		"SBPROF_BASE_REG$13 = 13;\n" \
           		"SBPROF_BASE_REG$14 = 14;\n" \
           		"SBPROF_BASE_REG$15 = 15;\n" \
           		"SBPROF_BASE_REG$16 = 16;\n" \
           		"SBPROF_BASE_REG$17 = 17;\n" \
           		"SBPROF_BASE_REG$18 = 18;\n" \
           		"SBPROF_BASE_REG$19 = 19;\n" \
           		"SBPROF_BASE_REG$20 = 20;\n" \
           		"SBPROF_BASE_REG$21 = 21;\n" \
           		"SBPROF_BASE_REG$22 = 22;\n" \
           		"SBPROF_BASE_REG$23 = 23;\n" \
           		"SBPROF_BASE_REG$24 = 24;\n" \
           		"SBPROF_BASE_REG$25 = 25;\n" \
           		"SBPROF_BASE_REG$26 = 26;\n" \
           		"SBPROF_BASE_REG$27 = 27;\n" \
           		"SBPROF_BASE_REG$28 = 28;\n" \
           		"SBPROF_BASE_REG$29 = 29;\n" \
           		"SBPROF_BASE_REG$30 = 30;\n" \
           		"SBPROF_BASE_REG$31 = 31;\n" \
           		"  .word (0x10 << 26) | (4 << 21) | (SBPROF_BASE_REG%0 << 16) | (25 << 11) | (0xf & %1);\n" \
                        ".set pop;"	  \
			:/* no output */:  "r" (val), "i" ((counter_num)*2));

#define ARM_COUNTER(counter_num, code, value) \
  /* disable counter; set value to *value; enable counter */ \
  /* When called with counters frozen, perhaps the disable is unnecessary. */ \
  WRITE_COUNTER_SEL(counter_num, 1 << 30); \
  WRITE_COUNTER(counter_num, value); \
  WRITE_COUNTER_SEL(counter_num, code)

/* Sets counter "counter_num" to monitor event "hwcode" starting from initial
   value "value" */
static __inline__
void arm_counter(u1 counter_num, u4 hwcode, u8 value)
{
  u4 code;
  hwcode &= 0x3f;
  code =
    (1 << 30) |		/* freeze enable */
    (hwcode << 5) |	/* event code */
    0x1f;		/* interrupt enable, count always */

  switch(counter_num) {
    case 0: ARM_COUNTER(0, code, &value); break;
    case 1: ARM_COUNTER(1, code, &value); break;
    case 2: ARM_COUNTER(2, code, &value); break;
    case 3: ARM_COUNTER(3, code, &value); break;
    default: panic("arm_counter: unknown counter");
  }
}

#undef ARM_COUNTER

/* WRITE_COUNTER makes sure that an inactive counter doesn't look like it
   overflowed to the interrupt handler */
#define UNARM_COUNTER(counter_num) \
  WRITE_COUNTER_SEL(counter_num, 1 << 30); /* freeze enable */ \
  WRITE_COUNTER(counter_num, &zero);

static __inline__
void unarm_counters(void)
{
  u8 zero = 0;
  UNARM_COUNTER(0);
  UNARM_COUNTER(1);
  UNARM_COUNTER(2);
  UNARM_COUNTER(3);
  PAUSE;
}

#undef UNARM_COUNTER

static
void stop_intr(void)
{
  WRITE_COUNTER_SEL(0, 1 << 30); /* freeze enable */
  WRITE_COUNTER_SEL(1, 1 << 30); /* freeze enable */
  WRITE_COUNTER_SEL(2, 1 << 30); /* freeze enable */
  WRITE_COUNTER_SEL(3, 1 << 30); /* freeze enable */
  PAUSE;
}


/***************************************************
 * Interrupt Invariant (holds whenver interrupts enabled)
 *  XOR
 *   o curbuf == 2
 *   o buffer[curbuf] has a previous EVENT_HDR message
 *
 ***************************************************/

/* Requires: cpud->buffer[cpud->x.curbuf] has room for event header and event
             p == &cpud->buffer[cpud->x.curbuf][cpud->x.next]
   Effect:   adds event header */
static void enter_event_hdr(cpud, p)
     cpudata_t *cpud;
     u1 *p;
{
  *p++ = DP_D_EVENTS;
  /* p += 3;  space for 1-byte pad and nevents */
  cpud->x.next += EVENT_HDR_MSG_SIZE;
  cpud->x.last_event = cpud->x.next;
}

/* Requires: cpud->x.last_event == -1
   Effect:   Enters a new_context message followed by an event_hdr message */
static void enter_context(cpud, pid, cpuid)
     cpudata_t *cpud;
     u4 pid;
     u1 cpuid;
{
  u1 *p = (u1 *) &cpud->buffer[cpud->x.curbuf][cpud->x.next];

  *p++ = DP_D_CONTEXT;
  *p++ = cpuid;
  *p++ = cpud->x.slotid;
  p += 1; /* align to 4 bytes */
  *((u4 *) p) = pid; p += 4;
  cpud->x.next += NEW_CONTEXT_MSG_SIZE;

  enter_event_hdr(cpud, p);
}  

/* Requires: cpud->x.curbuf == 2
   Effect:   If cpud's nextbuf is not empty, returns 0.
             Otherwise, sets curbuf to nextbuf and nextbuf to 2-nextbuf,
	     and returns 1.
 */
static /*__inline__*/ 
int find_buffer(cpud)
     cpudata_t *cpud;
{
  u1 nextbuf = cpud->x.nextbuf;
  if (cpud->x.full[nextbuf] == 0) {
    cpud->x.curbuf = nextbuf;
    cpud->x.next = 0;
    cpud->x.nnewpid[nextbuf] = 0;
    cpud->x.nextbuf = 1 - nextbuf;
#ifdef SBPROF_DEBUG
    {
      int i;
      u4 *p = (u4 *) & cpud->buffer[cpud->x.curbuf][0];
      for (i = 0; i < SBPROF_BUFSIZE/4; i++) {
	*p++ = 0xdeadbeef;
      }
    }
#endif
    return 1;
  } else {
    return 0;
  }
}

/* Requires: cpud->x.curbuf != 2, cpud->x.last_event == -1
 * Effect:   Marks the current buffer as full.  Sets curbuf to 2 and returns 0
 *           iff both buffers are full.  Otherwise, switches to the other
 *           buffer and returns 1.
 */
static /*__inline__*/
int swap_buffers(cpudata_t *cpud)
{
  if (cpud->x.last_event != -1) panic("sbprof: swap_buffers");
  if (cpud->x.full[cpud->x.curbuf] != 0) {
    panic("sbprof swap_buffers");
  }
#ifdef SBPROF_DEBUG
  {
    int curbuf = cpud->x.curbuf;
    int i;
    printf("sbprof releasing buffer %d full %u\n",
	   cpud->x.curbuf, cpud->x.next);
    for (i = 0; i < cpud->x.nnewpid[curbuf]; i++) {
      int offset = cpud->x.newpid[curbuf][i];
      printf("newpid[%d] at offset 0x%x name %s\n",
	     i, offset+28, &cpud->buffer[curbuf][offset+28]);
    }
  }
#endif
  cpud->x.full[cpud->x.curbuf] = cpud->x.next;
  semGive(cpud->x.semSync[cpud->x.curbuf]);	/* Wake daemon task */
  if (find_buffer(cpud)) {
    return 1;
  } else {
    cpud->x.curbuf = 2;
    return 0;
  }
}

/* If cpud's curbuf has an EVENT_HDR message lacking nevents, fills in nevents.
   (Unless nevents would be 0, in which case the EVENT_HDR message is
   removed from the buffer.)
   Sets last_event to -1.
*/
static void backfill_nevents(cpudata_t *cpud)
{
  if (cpud->x.last_event >= 0) {
    if (cpud->x.next == cpud->x.last_event) {
      cpud->x.next -= EVENT_HDR_MSG_SIZE;
    } else {
      int idx = cpud->x.last_event - 2;
      u2 n = (cpud->x.next - cpud->x.last_event) / EVENT_MSG_SIZE;
      u2 *p;
     /*printf("backfill_nevents next %u - last_event %d = %u idx %d n %u\n",
       cpud->x.next, cpud->x.last_event,
       cpud->x.next - (u4) cpud->x.last_event,
       idx, (u4) n);
     */
      p = (u2 *) &cpud->buffer[cpud->x.curbuf][idx];
      *p = n;
    }
    cpud->x.last_event = -1;
  }
}

/* Requires: cpud->x.curbuf != 2
   Effect: If there's enough room in a single, non-full cpud buffer for a
           new_context message followed by a pc sample, enters the new_context
	   message + event_hdr message, swapping buffers if necessary, and
	   returns 0.  Else reports the curbuf as full and returns 1.
 */
static int report_new_context(cpud, pid, cpuid)
     cpudata_t *cpud;
     u4 pid;
     u1 cpuid;
{
  backfill_nevents(cpud);
  if (cpud->x.next > SBPROF_BUFSIZE - (NEW_CONTEXT_MSG_SIZE +
				       EVENT_HDR_MSG_SIZE +
				       EVENT_MSG_SIZE)) {
    if (swap_buffers(cpud) == 0) return 1;
  }
  enter_context(cpud, pid, cpuid);
  return 0;
}

static void record_sample(restartpc, ptr, counter_num, cpuid)
     u4 restartpc;
     u4 ptr;
     u1 counter_num;
     u1 cpuid;
{
  u1 curbuf = cpudata->x.curbuf;

  cpudata->x.cur_slot[counter_num].total++;
  restartpc = ((counter_num == 1) && cpudata->x.use_ptr) ? ptr : restartpc;

  /* Find a buffer with space for a sample */
  if (curbuf == 2) {
    if (find_buffer(cpudata)) {
      enter_context(cpudata, cpudata->x.last_pid, cpuid);
    } else {    
      cpudata->x.cur_slot[counter_num].dropped++;
      return;
    }
  }

#if 0
  /* If context has changed, report it */
  if (pid != cpudata->x.last_pid) {
    if (report_new_context(cpudata, pid, cpuid) != 0) {
      cpudata->x.cur_slot[counter_num].dropped++;
      return;
    }
  }
  /* Record current context and report sample */
  cpudata->x.last_pid = pid;
#endif
  curbuf = cpudata->x.curbuf;
  *((u4 *) &cpudata->buffer[curbuf][cpudata->x.next]) = restartpc | counter_num;
  cpudata->x.next += 4;
  if (cpudata->x.next + EVENT_MSG_SIZE >= SBPROF_BUFSIZE) {
    backfill_nevents(cpudata);
    if (swap_buffers(cpudata)) {
      enter_context(cpudata, cpudata->x.last_pid, cpuid);
    }
  }
}

static void slot_enter(cpup, slotid, pid, cpuid)
     cpudata_t *cpup;
     u1 slotid;
     u4 pid;
     u1 cpuid;
{
  int i;
  event_counter_state_t *ecs = cpup->x.event_counter_state[slotid];

  /* XXX add prefetching, avoid branch? */
  for (i = 0; i < MAX_COUNTERS; i++) {
    /* Zero the per-slot-activation counts */
    cpup->x.cur_slot[i].total = 0;
    cpup->x.cur_slot[i].dropped = 0;
    if (ecs[i].inuse) {
      /* Compute value for re-arming counter */
      cpup->x.cur_slot[i].start_period =
	((u8) ecs[i].start_period_low) |
	(((u8) ecs[i].start_period_high) << 32);
      /* Set up counters with values from exit of last activation */
      arm_counter(i, ecs[i].hwcode, u64max(ecs[i].value,
					   cpup->x.cur_slot[i].start_period));
    }
  }
  cpup->x.slotid = slotid;
  cpup->x.use_ptr = cpup->x.slot_use_ptr[slotid];
  if (cpup->x.curbuf != 2)
    report_new_context(cpup, pid, cpuid);
  /* else buffers all full; code that starts new buffer will report context */
}

static void slot_exit(cpup, slotid, counter_val)
     cpudata_t *cpup;
     u1 slotid;
     u8 *counter_val;
{
  int i;
  event_counter_state_t *ecs = cpup->x.event_counter_state[slotid];

  /* XXX add prefetching, avoid branch? */
  for (i = 0; i < MAX_COUNTERS; i++) {
    if (ecs[i].inuse) {
      /* Export the per-slot-activation counts */
      ecs[i].total += cpup->x.cur_slot[i].total;
      ecs[i].dropped += cpup->x.cur_slot[i].dropped;
      /* Remember where to start off next time */
      ecs[i].value = counter_val[i];
    }
  }
  unarm_counters();
}

/* Tight loops can have skewed sample distributions when the sampling period
   is fixed.  If the loop and the performance counter interrupt handler
   have no dynamic stalls, one could even see all of the samples landing on a
   single instruction of the loop.

   A solution is to introduce a "ripple" to offset the fixed sampling period
   by a periodic wave.  Ripples are implemented here as a value in the range
   0..RIPPLE_SIZE-1 added to the sampling period.  RIPPLE_SIZE/2 is then
   subtracted to center the wave around the desired sampling period.

   A ripple value is generated by incrementing a previous ripple value by
   RIPPLE_GENERATOR using addition mod RIPPLE_SIZE.  Addition by
   RIPPLE_CONSTANT mod RIPPLE_SIZE must generate all values 0..RIPPLE_SIZE-1.

   If memory were infinite, one could keep the state of a previous ripple for
   each possible instruction PC.  This implementation approximates that by
   direct-mapping the PCs to RIPPLE_TABLE_SIZE distinct distinct ripples.
*/

#define RIPPLE_SIZE (1 << 12)
#define RIPPLE_GENERATOR (RIPPLE_SIZE/5)

#define RIPPLE_TABLE_SIZE 1024
static uint16_t ripple[RIPPLE_TABLE_SIZE];


/* Called whenever a level 5 interrupt is raised.  Freeze-enable will freeze
   counters IFF a counter has overflowed. Returns non-zero IFF a sample was
   collected.  */
int sbprof_cpu_intr()
{
  u1 cpuid = 0;
  int counter_num;
  u4 epc;
  u4 cause;
  u4 restartpc;
  u4 ptr;
  u8 msr[MAX_COUNTERS];
  cpudata_t *mycpud = cpudata + cpuid;
  bool cycles_tick;
  bool found_sample = 0;

  __asm__ __volatile__("mfc0 %0, $14;" : "=r"(epc));
  __asm__ __volatile__("mfc0 %0, $13;" : "=r"(cause));
  restartpc = epc + (cause & CAUSE_BD ? 4 : 0);
  __asm__ __volatile__("dmfc0  %0, $22;" : "=r"(ptr));

  READ_COUNTER(0, &msr[0]);
  READ_COUNTER(1, &msr[1]);
  READ_COUNTER(2, &msr[2]);
  READ_COUNTER(3, &msr[3]);
  
  ptr = (ptr & 1) ? ptr + 3 : ptr;

  cycles_tick = (msr[0] & (1ULL << 40)) ? TRUE : FALSE;
  for (counter_num = 0; counter_num < 4; counter_num++) {
    if (msr[counter_num] & (1ULL << 40)) {
      u8 next_period = mycpud->x.cur_slot[counter_num].start_period;
      int index = (restartpc >> 2) % RIPPLE_TABLE_SIZE;

      record_sample(restartpc, (u4) ptr, counter_num, cpuid);
      next_period += ripple[index];
      ripple[index] += RIPPLE_GENERATOR;
      ripple[index] &= RIPPLE_SIZE - 1;
      msr[counter_num] = next_period;
      found_sample = TRUE;
    }
  }

  if (!found_sample) return 0;

  if (cycles_tick) {
    mycpud->x.threshold--;
    if ((mycpud->x.nslots > 1) && !mycpud->x.threshold) {
      mycpud->x.threshold = 16;
      slot_exit(mycpud, mycpud->x.slotid, msr);
      slot_enter(mycpud,
		 (mycpud->x.slotid + 1) % mycpud->x.nslots,
		 mycpud->x.last_pid,
		 cpuid);
      return 1;
    }
  }
  
  /* Case "no counter overflowed":
       The writes below will discard events that occurred during most of
       this routine.
     Case "some counter overflowed":
       For counters that overflowed, these writes unfreeze the counters and
       clear the interrupt request.  For counters that didn't overflow, these
       writes are NOPs since they write back the current value.
       Once the last overflowed counter is written, subsequent writes discard
       some events that might occur as the writes are happening.
  */

  WRITE_COUNTER(0, &msr[0]);
  WRITE_COUNTER(1, &msr[1]);
  WRITE_COUNTER(2, &msr[2]);
  WRITE_COUNTER(3, &msr[3]);
  return 1;
}


static u8 rand_seed;

static void sbprof_start(sbprof_mux_slots *mux_slots) {
  int i;
  int lock_level;

  if (!stopped) fatal("sbprof already started!");
  
  /* Initialize ripple start points to random values */
  for (i = 0; i < RIPPLE_TABLE_SIZE; i++) {
    ripple[i] = rand_seed & (RIPPLE_SIZE - 1);
    rand_seed = (rand_seed * 0x5deece66dULL + 0xb) & 0xffffffffffffULL;
  }
  
  /* Initialize cpu data. */
  cpudata->x.last_pid = -1;
  cpudata->x.curbuf = 2;
  cpudata->x.nextbuf = 0;
  cpudata->x.nnewpid[0] = 0;
  cpudata->x.nnewpid[1] = 0;
  cpudata->x.next = 0;
  cpudata->x.last_event = -1;
  cpudata->x.full[0] = 0;
  cpudata->x.full[1] = 0;
  cpudata->x.semSync[0] = semBCreate(SEM_Q_FIFO, SEM_EMPTY);
  cpudata->x.semSync[1] = semBCreate(SEM_Q_FIFO, SEM_EMPTY);
  cpudata->x.threshold = 16;
  for (i = 0; i < MAX_SLOTS; i++) {
    int c;
    if (mux_slots->slots[i].n > 0) cpudata->x.nslots = i + 1;
    cpudata->x.slot_use_ptr[i] = mux_slots->slots[i].use_ptr;
    for (c = 0; c < MAX_COUNTERS; c++) {
      event_counter_state_t *ecs = &cpudata->x.event_counter_state[i][c];
      u8 period = PERIOD_2_CNTVALUE(mux_slots->slots[i].counter[c].period);
      /* total/dropped values may be read regardless of whether counter is
	 in use */
      ecs->total = 0;
      ecs->dropped = 0;
      ecs->value = 0;
      ecs->event = mux_slots->slots[i].counter[c].event;
      ecs->hwcode = mux_slots->slots[i].counter[c].hwcode;
      ecs->start_period_low = (u4) period;
      ecs->start_period_high = (u1) (period >> 32);
      ecs->inuse = (c < mux_slots->slots[i].n);
    }
  }
  cpudata->x.slotid = 0;
 
  find_buffer(cpudata); /* slot_enter() will enter the context */
  /* With interrupts blocked, enter the first multiplexing slot */
  lock_level = intLock(); {
    u4 status;
    unarm_counters();
    slot_enter(cpudata, 0, -1, 0);
    __asm__ __volatile__ ("mfc0	%0, $12;" : "=r"(status));
    status |= 0x8000;
    __asm__ __volatile__ ("mtc0	%0, $12;" :: "r"(status));
    stopped = 0;
  } intUnlock(lock_level);
}


static void sbprof_stop()
{
  int lock_level;
  int curbuf = 2;
  u8 counter_val[MAX_COUNTERS];

  lock_level = intLock(); {
    stop_intr();
    stopped = 1;
  } intUnlock(lock_level);
  /* Wait for any last interrupts */
  PAUSE;
  lock_level = intLock(); {
    READ_COUNTER(0, &counter_val[0]);
    READ_COUNTER(1, &counter_val[1]);
    READ_COUNTER(2, &counter_val[2]);
    READ_COUNTER(3, &counter_val[3]);
    slot_exit(cpudata, cpudata->x.slotid, counter_val);
    if (cpudata->x.curbuf != 2) {
      backfill_nevents(cpudata);
      cpudata->x.full[cpudata->x.curbuf] = cpudata->x.next;
      curbuf = cpudata->x.curbuf;
      cpudata->x.curbuf = 2;
    }
  } intUnlock(lock_level);
  if (curbuf != 2)
    semGive(cpudata->x.semSync[curbuf]);	/* Wake daemon task */
}

/************************************************************************
 **** Daemon ****
 ************************************************************************/

#define VERBOSE 0

#define dprintf(fm, args...) do { if (VERBOSE) printf(fm, ## args); } while (0)

/*static comm_t s;*/

/* Requires: msg[msgind] is aligned to width of put, except for put8, where
   only 4-byte alignment is required */
#define MSG_PUT2(x) { *((u2 *) (&msg[msgind])) = (x); msgind += 2; }
#define MSG_PUT4(x) { *((u4 *) (&msg[msgind])) = (x); msgind += 4; }
#define MSG_PUT8(x) { _msg_put8((x), (u4 *) (&msg[msgind])); msgind += 8; }

static __inline__
void _msg_put8(u8 data, u4 *msg)
{
#if defined(_HOST_IS_BIG_ENDIAN)
  *msg++ = (data >> 32);
  *msg++ = data & 0xffffffff;
#else
  *msg++ = data & 0xffffffff;
  *msg++ = (data >> 32);
#endif
}

/* Requires: msg is aligned to width of peek */
#define MSG_PEEK1 ( *((u1 *) &msg[msgind]) )
#define MSG_PEEK2 ( *((u2 *) &msg[msgind]) )
#define MSG_PEEK4 ( *((u4 *) &msg[msgind]) )
#define MSG_PEEK8 ( *((u8 *) &msg[msgind]) )

/* returns TRUE if remote machine decided to close profile session */
static bool exchange_greetings(comm_t comm, sbprof_mux_slots *mux_slots)
{
  u4 nslots;
  /* Make msg 8-byte aligned; + 4 is extra space */
  u8 msg8[1 + (2*MAX_COUNTERS+7)/8 + MAX_COUNTERS + 4];
  u1 *msg = (u1 *) msg8;
  int msgind;
  u1 ncpus = 0;
  u8 system_rev = *((u8 *) 0xb0020000);
  u4 prid;
  bool driver_is_little_endian = TRUE;
#if defined(_HOST_IS_BIG_ENDIAN)
  driver_is_little_endian = FALSE;
#endif
  __asm__ __volatile__ ("mfc0	%0, $15;" : "=r"(prid));

  switch ((system_rev >> 16) & 0xffff) {
    /*case 0x1125: ncpus = 1; break; */
    case 0x1250: ncpus = 2; break;
    default:
      fatal("driver only supports sb1250");
  }
  if (ncpus != 2) {
    fatal("driver only supports 1250");
  }
  
  comm_recv(comm, msg, 1);
  assert(msg[0] == DP_G_PCARCH);
  msgind = 0;
  msg[msgind++] = OS_VXWORKS;
  {
    perf_cnt_arch_t pcarch = OS_LAST; /* initialize to silence compiler */
    switch (prid & 0xff) {
      case 1: pcarch = PERF_CNT_ARCH_SB1; break;
      case 2: pcarch = PERF_CNT_ARCH_SB1R2; break;
      default: fatal("unknown SB1\n");
    }
    msg[msgind++] = (u1) pcarch;
  }
  msg[msgind++] = TRUE; /* PCs are aligned */
  msg[msgind++] = driver_is_little_endian;
  msg[msgind++] = FALSE; /* PCs are not 8 bytes, i.e., they're 4 bytes */
  msg[msgind++] = 1;     /* cpu bitvector */
  msg[msgind++] = 2;	 /* driver major version; pgather may insist
			    on an equal major version. */
  msg[msgind++] = 0;	 /* driver minor version */
  MSG_PUT8(system_rev);  /* System version */
  MSG_PUT4(prid);        /* CPU version */
  msgind += 4;    /* pad */

  comm_send(comm, msg, msgind);

  comm_recv(comm, msg, 8);
  if (msg[0] == DP_G_CLOSE) return TRUE;
  assert(msg[0] == DP_G_START);
  {
    u1 i;
    nslots = msg[1];
    if (nslots < 1) fatal("need at least one slot");
    for (i = 0; i < nslots; i++) {
      u4 c;
      msgind = 0;
      comm_recv(comm, msg, 8 + 2*MAX_COUNTERS + 8*MAX_COUNTERS);
      mux_slots->slots[i].n = MSG_PEEK1; msgind++;
      mux_slots->slots[i].use_ptr = MSG_PEEK1; msgind++;
      msgind += 6;
      for (c = 0; c < MAX_COUNTERS; c++) {
	mux_slots->slots[i].counter[c].event = MSG_PEEK1; msgind++;
	mux_slots->slots[i].counter[c].hwcode = MSG_PEEK1; msgind++;
      }
      for (c = 0; c < MAX_COUNTERS; c++) {
	mux_slots->slots[i].counter[c].period = MSG_PEEK8; msgind += 8;
      }
    }
    for (i = nslots; i < MAX_SLOTS; i++) {
      mux_slots->slots[i].n = 0;
    }
  }

  msgind = 0;
  msg[msgind++] = DP_D_PID_IMAGE;
  msgind += 3; /* pad to 4 bytes */
  MSG_PUT4(-1);
  MSG_PUT8(1); /* imageid */
  MSG_PUT8(2); /* base */
  MSG_PUT4(3); /* num_inst */
  memcpy((void *) &msg[msgind], (void *) "4", strlen("4")+1);
  msgind += MAX_IMAGE_NAME;
  comm_send(comm, msg, msgind);
  return FALSE;
}

static void send_stopped(comm_t comm)
{
  u8 msg[1 + MAX_SLOTS*MAX_COUNTERS];
  u4 i;
  u4 msgind = 0;  /* index of u8, not u1 */
  
  /* Put DP_D_STOPPED in 1st byte regardless of endianness */
  *(u1 *)msg = DP_D_STOPPED;
  msgind++;
  /* Send count of dropped samples by mux slot */
  for (i = 0; i < MAX_SLOTS; i++) {
    u4 c;
    for (c = 0; c < MAX_COUNTERS; c++) {
      msg[msgind++] = cpudata->x.event_counter_state[i][c].dropped;
    }
  }

  comm_send(comm, (u1 *) msg, sizeof(msg));
}

static void empty_buffer(cpudata_t *cpudata, int buf, comm_t comm)
{
  if (cpudata->x.nnewpid[buf] > 0) {
    fatal("nnewpid > 0");
  }

  comm_send(comm, (u1 *) cpudata->buffer[buf], cpudata->x.full[buf]);
}

LOCAL void sbprof_stop_wait(int a1, int a2, int a3, int a4, int a5,
			    int a6, int a7, int a8, int a9, int a10)
{
  comm_t comm = (comm_t) a1;
  bool *stop_received = (bool *) a2;
  u1 msg[4];

  comm_recv(comm, msg, 1);
  assert(msg[0] == DP_G_STOP);
  sbprof_stop();
  *stop_received = TRUE;
}

static void profile_session(comm_t comm)
{
  int nextbuf = 0;
  sbprof_mux_slots mux_slots;
  volatile bool stop_received = FALSE;
  int my_priority;

  if (exchange_greetings(comm, &mux_slots)) return;
  if (taskPriorityGet(taskIdSelf(), &my_priority) != OK)
    fatal("taskPriorityGet failed\n");
  if (taskSpawn("sbprof_stop_wait", my_priority, 0, 8*1024,
		(FUNCPTR) &sbprof_stop_wait,
		(int) comm, (int) &stop_received, 0, 0, 0,
		0, 0, 0, 0, 0) == ERROR)
    fatal("taskSpawn failed\n");
  sbprof_start(&mux_slots);

  do {
    semTake(cpudata->x.semSync[nextbuf], WAIT_FOREVER);
    empty_buffer(cpudata, nextbuf, comm);
    cpudata->x.full[nextbuf] = 0;
    nextbuf = 1 - nextbuf;
  } while (!stop_received ||
	   (cpudata->x.full[nextbuf] != 0));

  {
    u8 total = 0;
    u8 total_dropped = 0;
    u4 c;
    int n;
    send_stopped(comm);
    for (n = 0; n < MAX_SLOTS; n++) {
      event_counter_state_t *ecs = cpudata->x.event_counter_state[n];
      if (!ecs[0].inuse || !mux_slots.slots[n].n) break;
      dprintf("slot %d\n", n);
      for (c = 0; c < MAX_COUNTERS; c++) {
	if (!ecs[c].inuse) continue;
	dprintf("total[%d]   %7llu ", c, ecs[c].total);
	dprintf("dropped[%d] %5llu\n", c, ecs[c].dropped);
	total += ecs[c].total;
	total_dropped += ecs[c].dropped;
      }
    }
  }
}

void bcm1250_sbprof_daemon(int a1, int a2, int a3, int a4, int a5,
			   int a6, int a7, int a8, int a9, int a10)
{
  u2 port = (u2) a1;
  comm_t s = create_listener(port);
  comm_t as;

  FOREVER {
    struct sockaddr_in sa;
    int size = sizeof(sa);

    dprintf("calling accept\n");
    fflush(stdout);
    as = accept(s, (struct sockaddr *) &sa, &size);
    if (as == ERROR) fatal_errno("accept failed");
    dprintf("connection from sin_addr 0x%08lx\n", sa.sin_addr.s_addr);
#if 0
    {
      unsigned int opt;
      socklen_t len = sizeof(opt);
      opt = 1;
      if (setsockopt(as, IPPROTO_TCP, TCP_NODELAY, &opt, len))
	dprintf("setsockopt failed\n");
    }
#endif
    profile_session(as);
    close(as);
    as = -1;
  }
  comm_close(s);
}

