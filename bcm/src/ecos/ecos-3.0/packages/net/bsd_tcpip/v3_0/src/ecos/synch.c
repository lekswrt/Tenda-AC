//==========================================================================
//
//      src/ecos/synch.c
//
//==========================================================================
// ####BSDCOPYRIGHTBEGIN####                                    
// -------------------------------------------                  
// This file is part of eCos, the Embedded Configurable Operating System.
//
// Portions of this software may have been derived from FreeBSD 
// or other sources, and if so are covered by the appropriate copyright
// and license included herein.                                 
//
// Portions created by the Free Software Foundation are         
// Copyright (C) 2002 Free Software Foundation, Inc.            
// -------------------------------------------                  
// ####BSDCOPYRIGHTEND####                                      
//==========================================================================

//==========================================================================
//
//      ecos/synch.c
//
//      eCos wrapper and synch functions
//
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    gthomas, hmt
// Contributors: gthomas, hmt
// Date:         2000-01-10
// Purpose:      
// Description:  
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================


// Synch routines, etc., used by network code

#include <sys/param.h>
#include <pkgconf/net.h>

#include <cyg/infra/diag.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/kernel/kapi.h>

#include <cyg/infra/cyg_ass.h>

//---------------------------- splx() emulation ------------------------------
// This contains both the SPLX stuff and tsleep/wakeup - because those must
// be SPLX aware.  They release the SPLX lock when sleeping, and reclaim it
// (if needs be) at wakeup.
//
// The variable spl_state (and the associated bit patterns) is used to keep
// track of the "splx()" level.  This is an artifact of the original stack,
// based on the BSD interrupt world (interrupts and processing could be
// masked based on a level value, supported by hardware).  This is not very
// real-time, so the emulation uses proper eCos tools and techniques to
// accomplish the same result.  The key here is in the analysis of the
// various "levels", why they are used, etc.
//
// SPL_IMP is called in order to protect internal data structures
// short-term, primarily so that interrupt processing does not interfere
// with them.
// 
// SPL_CLOCK is called in order to ensure that a timestamp is valid i.e. no
// time passes while the stamp is being taken (since it is a potentially
// non-idempotent data structure).
//
// SPL_SOFTNET is used to prevent all other stack processing, including
// interrupts (DSRs), etc.
//
// SPL_INTERNAL is used when running the pseudo-DSR in timeout.c - this
// runs what should really be the network interface device's DSR, and any
// timeout routines that are scheduled.  (They are broken out into a thread
// to isolate the network locking from the rest of the system)
//
// NB a thread in thi state can tsleep(); see below.  Tsleep releases and
// reclaims the locks and so on.  This necessary because of the possible
// conflict where
//     I splsoft
//     I tsleep
//     He runs, he is lower priority
//     He splsofts
//     He or something else awakens me
//     I want to run, but he has splsoft, so I wait
//     He runs and releases splsoft
//     I awaken and go.

static volatile cyg_uint32 spl_state = 0;
#define SPL_IMP      0x01
#define SPL_NET      0x02
#define SPL_CLOCK    0x04
#define SPL_SOFTNET  0x08
#define SPL_INTERNAL 0x10

static cyg_mutex_t splx_mutex;
static volatile cyg_handle_t splx_thread;

#ifdef CYGIMPL_TRACE_SPLX

#define SPLXARGS const char *file, const int line
#define SPLXMOREARGS , const char *file, const int line
#define SPLXMOREPARMS , file, line

/* Trace functions */
#define MAX_SPL_EVENTS 256
static struct spl_event_t {
	char *in_file;
	int in_line;
	cyg_uint32 in_lock;
	int in_id;
	char *out_file;
	int out_line;
	cyg_uint32 out_lock;
	int out_id;
} spl_event[MAX_SPL_EVENTS];

static int spl_next;
static int spl_gauge;

static void
spl_trace_in(char *file, int line)
{
	struct spl_event_t *se;

	cyg_scheduler_lock();

	if (spl_next >= MAX_SPL_EVENTS-1) {
		diag_printf("%s: spl_next >= %d, please check....\n",
			__func__, spl_next >= MAX_SPL_EVENTS-1);
		cyg_scheduler_unlock();
		return;
	}

	se = &spl_event[spl_next];
	spl_next++;

	if (spl_next > spl_gauge)
		spl_gauge = spl_next;

	/* Preserve spl information */
	memset(se, 0, sizeof(*se));
	se->in_file = file;
	se->in_line = line;
	se->in_lock = spl_state;
	se->in_id = cyg_thread_get_id(cyg_thread_self());

	cyg_scheduler_unlock();
	return;
}

static void
spl_trace_out(char *file, int line)
{
	struct spl_event_t *se;

	cyg_scheduler_lock();

	if (spl_next == 0) {
		diag_printf("%s: BUG, spl_next == 0, please check....\n", __func__);
		cyg_scheduler_unlock();
		return;
	}

	spl_next--;
	se = &spl_event[spl_next];
	se->out_file = file;
	se->out_line = line;
	se->out_lock = spl_state;
	se->out_id = cyg_thread_get_id(cyg_thread_self());

	cyg_scheduler_unlock();
	return;
}

static volatile char *splx_func;
static volatile int splx_line;
#define SPLXSAVEARGS	{ splx_func = (char *)file; splx_line = line; }
#define SPLXTRACEIN spl_trace_in((char *)file, line)
#define SPLXTRACEOUT spl_trace_out((char *)file, line)

#else
#define SPLXARGS void
#define SPLXMOREARGS
#define SPLXMOREPARMS
#define SPLXSAVEARGS
#define SPLXTRACEIN
#define SPLXTRACEOUT
#endif


static inline cyg_uint32
spl_any( cyg_uint32 which SPLXMOREARGS )
{
    cyg_uint32 old_spl = spl_state;
    if ( cyg_thread_self() != splx_thread ) {
        while ( !cyg_mutex_lock( &splx_mutex ) )
            continue;
        old_spl = 0; // Free when we unlock this context
        CYG_ASSERT( 0 == splx_thread, "Thread still owned" );
        CYG_ASSERT( 0 == spl_state, "spl still set" );
        splx_thread = cyg_thread_self();
        SPLXSAVEARGS;
    }
    CYG_ASSERT( splx_mutex.locked, "spl_any: mutex not locked" );
    CYG_ASSERT( (cyg_handle_t)splx_mutex.owner == cyg_thread_self(),
                "spl_any: mutex not mine" );
    spl_state |= which;
    return old_spl;
}


cyg_uint32
cyg_splimp(SPLXARGS)
{
    SPLXTRACEIN;
    return spl_any( SPL_IMP SPLXMOREPARMS );
}

cyg_uint32
cyg_splclock(SPLXARGS)
{
    SPLXTRACEIN;
    return spl_any( SPL_CLOCK SPLXMOREPARMS );
}

cyg_uint32
cyg_splnet(SPLXARGS)
{
    SPLXTRACEIN;
    return spl_any( SPL_NET SPLXMOREPARMS );
}

cyg_uint32
cyg_splhigh(SPLXARGS)
{
    SPLXTRACEIN;
    // splhigh did SPLSOFTNET in the contrib, so this is the same
    return spl_any( SPL_SOFTNET SPLXMOREPARMS );
}

cyg_uint32
cyg_splsoftnet(SPLXARGS)
{
    SPLXTRACEIN;
    return spl_any( SPL_SOFTNET SPLXMOREPARMS );
}

cyg_uint32
cyg_splinternal(SPLXARGS)
{
    SPLXTRACEIN;
    return spl_any( SPL_INTERNAL SPLXMOREPARMS );
}


//
// Return to a previous interrupt state/level.
//
void
cyg_splx(cyg_uint32 old_state SPLXMOREARGS)
{
    SPLXTRACEOUT;
    
    CYG_ASSERT( 0 != spl_state, "No state set" );
    CYG_ASSERT( splx_mutex.locked, "splx: mutex not locked" );
    CYG_ASSERT( (cyg_handle_t)splx_mutex.owner == cyg_thread_self(),
                "splx: mutex not mine" );

    spl_state &= old_state;

    if ( 0 == spl_state ) {
        splx_thread = 0; 
        cyg_mutex_unlock( &splx_mutex );
    }
}

void
show_splx(void)
{
#ifdef CYGIMPL_TRACE_SPLX
	int i;

	cyg_scheduler_lock();

	diag_printf("SPLX: spl_state: %d, function: %s, line: %d, thread id: %d\n",
		spl_state,
		splx_func,
		splx_line,
		splx_thread == 0 ? 0 : cyg_thread_get_id(splx_thread));

	diag_printf("spl_next = %d, spl_gauge = %d\n", spl_next, spl_gauge);

	for (i = 0; i < spl_gauge; i++) {
		/* Print SPL TRACE IN */
		if (spl_event[i].in_file) {
	 		diag_printf("[%s] SPLTRACE IN(tid %d): %s(%d), state:%d\n",
				i < spl_next ? " " : (i == spl_next ? "*" : "-"),
				spl_event[i].in_id, spl_event[i].in_file,
				spl_event[i].in_line, spl_event[i].in_lock);
		}

		/* Print SPL TRACE OUT */
		if (spl_event[i].out_file) {
 			diag_printf("[%s] SPLTRACE OUT(tid %d): %s(%d), state:%d\n",
				i < spl_next ? " " : (i == spl_next ? "*" : "-"),
				spl_event[i].out_id, spl_event[i].out_file,
				spl_event[i].out_line, spl_event[i].out_lock);
		}
	}

	cyg_scheduler_unlock();

#else
	diag_printf("SPLX: spl_state: %d, thread id: %d\n",
		spl_state,
		splx_thread == 0 ? 0 : cyg_thread_get_id(splx_thread));
#endif
}


//------------------ tsleep() and wakeup() emulation ---------------------------
//
// Structure used to keep track of 'tsleep' style events
//
struct wakeup_event {
    void *chan;
    cyg_sem_t sem;
};
static struct wakeup_event wakeup_list[CYGPKG_NET_NUM_WAKEUP_EVENTS];


// Called to initialize structures used by timeout functions
void
cyg_tsleep_init(void)
{
    int i;
    struct wakeup_event *ev;
    // Create list of "wakeup event" semaphores
    for (i = 0, ev = wakeup_list;  i < CYGPKG_NET_NUM_WAKEUP_EVENTS;  i++, ev++) {
        ev->chan = 0;
        cyg_semaphore_init(&ev->sem, 0);
    }
    // Initialize the mutex and thread id:
    cyg_mutex_init( &splx_mutex );
    splx_thread = 0;
}


//
// Signal an event
void
cyg_wakeup(void *chan)
{
    int i;
    struct wakeup_event *ev;
    cyg_scheduler_lock(); // Ensure scan is safe
    // NB this is broadcast semantics because a sleeper/wakee holds the
    // slot until they exit.  This avoids a race condition whereby the
    // semaphore can get an extra post - and then the slot is freed, so the
    // sem wait returns immediately, AOK, so the slot wasn't freed.
    for (i = 0, ev = wakeup_list;  i < CYGPKG_NET_NUM_WAKEUP_EVENTS;  i++, ev++)
        if (ev->chan == chan)
            cyg_semaphore_post(&ev->sem);

    cyg_scheduler_unlock();
}

// ------------------------------------------------------------------------
// Wait for an event with timeout
//   tsleep(event, priority, state, timeout)
//     event - the thing to wait for
//     priority - unused
//     state    - a descriptive message
//     timeout  - max time (in ticks) to wait
//   returns:
//     0         - event was "signalled"
//     ETIMEDOUT - timeout occurred
//     EINTR     - thread broken out of sleep
//
int       
cyg_tsleep(void *chan, int pri, char *wmesg, int timo)
{
    int i, res = 0;
    struct wakeup_event *ev;    
    cyg_tick_count_t sleep_time;
    cyg_handle_t self = cyg_thread_self();
    int old_splflags = 0; // no flags held

    cyg_scheduler_lock();

    // Safely find a free slot:
    for (i = 0, ev = wakeup_list;  i < CYGPKG_NET_NUM_WAKEUP_EVENTS;  i++, ev++) {
        if (ev->chan == 0) {
            ev->chan = chan;
            break;
        }
    }
    CYG_ASSERT( i <  CYGPKG_NET_NUM_WAKEUP_EVENTS, "no sleep slots" );
    CYG_ASSERT( 1 == cyg_scheduler_read_lock(),
                "Tsleep - called with scheduler locked" );
    // Defensive:
    if ( i >= CYGPKG_NET_NUM_WAKEUP_EVENTS ) {
        cyg_scheduler_unlock();
        return ETIMEDOUT;
    }

    // If we are the owner, then we must release the mutex when
    // we wait.
    if ( self == splx_thread ) {
        old_splflags = spl_state; // Keep them for restoration
        CYG_ASSERT( spl_state, "spl_state not set" );
        // Also want to assert that the mutex is locked...
        CYG_ASSERT( splx_mutex.locked, "Splx mutex not locked" );
        CYG_ASSERT( (cyg_handle_t)splx_mutex.owner == self, "Splx mutex not mine" );

	// Simon
	spl_trace_out(__func__, __LINE__);

        splx_thread = 0;
        spl_state = 0;
        cyg_mutex_unlock( &splx_mutex );
    }

    // Re-initialize the semaphore - it might have counted up arbitrarily
    // in the time between a prior sleeper being signalled and them
    // actually running.
    cyg_semaphore_init(&ev->sem, 0);    

    // This part actually does the wait:
    // As of the new kernel, we can do this without unlocking the scheduler
    if (timo) {
        sleep_time = cyg_current_time() + timo;
        if (!cyg_semaphore_timed_wait(&ev->sem, sleep_time)) {
            if( cyg_current_time() >= sleep_time )
                res = ETIMEDOUT;
            else
                res = EINTR;
        }
    } else {
        if (!cyg_semaphore_wait(&ev->sem) ) {
            res = EINTR;
        }
    }

    ev->chan = 0;  // Free the slot - the wakeup call cannot do this.
    
    if ( old_splflags ) { // restore to previous state
	// Simon
	spl_trace_in(__func__, __LINE__);
    
        // As of the new kernel, we can do this with the scheduler locked
        cyg_mutex_lock( &splx_mutex ); // this might wait
        CYG_ASSERT( 0 == splx_thread, "Splx thread set in tsleep" );
        CYG_ASSERT( 0 == spl_state, "spl_state set in tsleep" );
        splx_thread = self; // got it now...
        spl_state = old_splflags;
    }

    cyg_scheduler_unlock();
    return res;
}



// ------------------------------------------------------------------------
// DEBUGGING ROUTINES
#ifdef CYGIMPL_TRACE_SPLX
/* Note the following code doesn't work yet */
#if 0
#undef cyg_scheduler_lock
#undef cyg_scheduler_safe_lock
#undef cyg_scheduler_unlock

#define MAX_SCHED_EVENTS 256
static struct _sched_event {
    char *fun, *file;
    int line, lock;
} sched_event[MAX_SCHED_EVENTS];
static int next_sched_event = 0;
static int total_sched_events = 0;

static void
do_sched_event(char *fun, char *file, int line, int lock)
{
    struct _sched_event *se = &sched_event[next_sched_event];
    if (++next_sched_event == MAX_SCHED_EVENTS) {
        next_sched_event = 0;
    }
    se->fun = fun;
    se->file = file;
    se->line = line;
    se->lock = lock;
    total_sched_events++;
}

static void
show_sched_events(void)
{
    int i;
    struct _sched_event *se;
    if (total_sched_events < MAX_SCHED_EVENTS) {
        i = 0;
    } else {
        i = next_sched_event + 1;
        if (i == MAX_SCHED_EVENTS) i = 0;
    }
    diag_printf("%d total scheduler events\n", total_sched_events);
    while (i != next_sched_event) {
        se = &sched_event[i];
        diag_printf("%s - lock: %d, called from %s.%d\n", se->fun, se->lock, se->file, se->line);
        if (++i == MAX_SCHED_EVENTS) i = 0;
    }
}

#define SPLX_TRACE_DATA() cyg_scheduler_read_lock()

void
_cyg_scheduler_lock(char *file, int line)
{
    cyg_scheduler_lock();
    do_sched_event(__FUNCTION__, file, line, SPLX_TRACE_DATA());
}

void
_cyg_scheduler_safe_lock(char *file, int line)
{
    cyg_scheduler_safe_lock();
    do_sched_event(__FUNCTION__, file, line, SPLX_TRACE_DATA());
}

void
_cyg_scheduler_unlock(char *file, int line)
{
    cyg_scheduler_unlock();
    do_sched_event(__FUNCTION__, file, line, SPLX_TRACE_DATA());
}
#endif	// 0
#endif // CYGIMPL_TRACE_SPLX

// EOF synch.c
