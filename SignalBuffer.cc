#include "SignalBuffer.hh"
#include "GMH.hh"
// Need time to help seed random number generator
#include <sys/time.h>
// Need sleep to wait for mutex in destructor
#include <unistd.h>
// Need standard error
#include <iostream>
#include <errno.h>

using namespace std;
using namespace SigC;
using namespace Threads;
using namespace WrapSlot;

// Local to this file
static pthread_mutexattr_t fast_mutexattr = {PTHREAD_MUTEX_FAST_NP};
static pthread_mutexattr_t recursive_mutexattr = {PTHREAD_MUTEX_RECURSIVE_NP};
static pthread_mutexattr_t errorcheck_mutexattr = {PTHREAD_MUTEX_ERRORCHECK_NP};

// Above used to initialize these exported global constants
const MutexAttr fast_mutex={ &fast_mutexattr };
const MutexAttr recursive_mutex = { &recursive_mutexattr };
const MutexAttr errorcheck_mutex = { &errorcheck_mutexattr };

void SimpleSignalBuffer::call_all() 
{
    if (!discard_) 
    {
        // A *growing* deque will not have the new events called.
        // A *shrinking* deque will not overrun the index range.
        size_t i;
        size_t end=pending_.size();
        for (i=0; (i<end) && (i<pending_.size()); ++i) {
            pending_[i]->re_emit();
        } 
    }
    else 
    {
        Wrap* event;
        size_t total=pending_.size();
        // Delete one at a time in case we are interrupted
        // A *growing* deque will not have the new events called.
        // A *shrinking* deque will end when all events are gone.
        while ((total>0)&&(!pending_.empty())) 
        {
            --total;
            event=pending_.front();
            pending_.pop_front();
            event->re_emit();
            delete event;
            event=NULL;
        }
    }
    last_event_=-1;
}

void SimpleSignalBuffer::call_one_event() 
{
    if (pending_.empty())
    {
        return;
    }
    if (!discard_) 
    {
        ++last_event_;
        if ((unsigned int)last_event_ >= pending_.size())
        {
            last_event_=0;
        }
        pending_[last_event_]->re_emit();
    }
    else 
    {
        last_event_=-1;
        Wrap* event=pending_.front();
        pending_.pop_front();
        event->re_emit();
        delete event;
        event=NULL;
    }
}

void SimpleSignalBuffer::discard_all() 
{
    Queue::iterator i=pending_.begin();
    const Queue::iterator& end=pending_.end();
    while (i!=end)
    {
        delete (*(i++));
    }
    pending_.erase(pending_.begin(),end); // ignore signed vs unsigned warning
    last_event_=-1;
}


#ifndef SIGC_PTHREADS
#error "NO SIGC_PTHREADS"
#endif
#ifndef SIGC_THREAD_IMPL
#error NO SIGC_THREAD_IMPL
#endif


SafeSignalBuffer::SafeSignalBuffer(bool discard=true,const MutexAttr type_mutex)
    : last_event_(-1),     discard_(discard),
      valid_(true),        empty_(true),
      mutex_(type_mutex),  cond_(),
      pending_(),          busy_calling_(errorcheck_mutex) 
{
    _show_args1(this);
    callEvents.connect(slot(this,&SafeSignalBuffer::call_all));
    callOneEvent.connect(slot(this,&SafeSignalBuffer::call_one_event));
}

SafeSignalBuffer::~SafeSignalBuffer() 
{
    _show_args1(this);
    if (valid_) {
        valid_=false;
        if (mutex_.trylock()) {
            cerr << endl << "~SafeSignalBuffer waiting 0.3 seconds for lock" << endl;
            usleep(300*1000);
            if (mutex_.trylock()) {
                cerr << "~SafeSignalBuffer could not lock the mutex" << endl;
                cerr << "This is fatal, terminating with code 10" << endl;
                exit(10);
            }
        }
        // Now the mutex_ has been locked
        Queue::iterator i=pending_.begin();
        const Queue::iterator end=pending_.end();
        while (i!=end) {
            delete(*(i++));
        }
        pending_.clear();
        mutex_.unlock();
        cond_.broadcast();
    }
}

void SafeSignalBuffer::store_wrapped_event(WrapSlot::Wrap *w) 
{
    _show_args1(this);
    if (valid_&&(NULL!=w)) {
        if (-1==mutex_lock(mutex_,true))
        {
            // Cleanup before returning
            delete w;
            w=NULL;
            return;
        }
        if (valid_)
        {
            pending_.push_back(w);
            empty_=pending_.empty();
            cond_.broadcast();
        }
        mutex_.unlock();
    }
}

size_t SafeSignalBuffer::size() const 
{ 
    SafeSignalBuffer::Queue::size_type my_size=0;
    if (valid_) 
    {
        if (-1==mutex_lock(mutex_,true)) 
        {
            return 0;
        }
        my_size=pending_.size();
        mutex_.unlock();
    }
    _show_args2(this,my_size);
    return my_size;
}

// This funtion returns the number of events delivered
// It will return -1 if there is some kind of error.
void SafeSignalBuffer::call_all() 
{
    _show_args1(this);
    if (valid_) 
    {
        if (-1==mutex_lock(mutex_,true))
        {
            return;
        }
        // Record the total value upon entering call_all
        // No more than total events will be delivered
        // This helps prevent inifinite loops
        size_t total=pending_.size();
        guint delivered=0;
        Wrap *event;

        if (!discard_) 
        {
            // A *growing* deque will not have the new events called.
            // A *shrinking* deque will not overrun the index range.
            for (delivered=0; 
                 valid_ && (delivered<total) && (delivered<pending_.size()); 
                 ++delivered)
            {
                event=pending_[delivered];
                mutex_.unlock();   //  Unlock during side effects
                if (valid_)
                {         
                    event->re_emit();   //  re_emit side effects
                }
                else
                {
                    return;
                }
                if (-1==mutex_lock(mutex_,true))
                {
                    return;
                }
            }
        }
        else 
        {
            // Delete one at a time in case we are interrupted
            // A *growing* deque will not have the new events called.
            // A *shrinking* deque will end when all events are gone.
            while (valid_&&(delivered<total)&&(!pending_.empty()))
            {
                ++delivered;
                event=pending_.front();
                pending_.pop_front();
                empty_=pending_.empty();
                mutex_.unlock();  // Unlock during side effects
                if (valid_) 
                {
                    event->re_emit();   //  re_emit side effects
                    delete event;       //  dtor side effects
                    event=NULL;
                }
                else
                {
                    return;
                }
                if (-1==mutex_lock(mutex_,true))
                {
                    return;
                }
            }
        }
        if (valid_) 
        {
            empty_=pending_.empty();
            cond_.broadcast();
            last_event_=-1;

        }
        mutex_.unlock();
    }
}

void SafeSignalBuffer::call_one_event() 
{
    _show_args1(this);
    if (valid_) 
    {
        if (0==mutex_lock(mutex_,true)) 
        {
            if (!discard_) 
            {
                ++last_event_;
                if ((unsigned int)last_event_ >= pending_.size())
                    last_event_=0;
                {
                
                    pending_[last_event_]->re_emit();
                }
            } 
            else 
            {
                last_event_=-1;
                Wrap* event=pending_.front();
                pending_.pop_front();
                empty_=pending_.empty();
                event->re_emit();
                delete event;
                event=NULL;
            }
            if (valid_) 
            {
                empty_=pending_.empty();
                cond_.broadcast();
                last_event_=-1;
            }
            mutex_.unlock();
        }
    }
}


void SafeSignalBuffer::discard_all() 
{
    _show_args1(this);
    if (valid_) 
    {
        if (0==mutex_lock(mutex_,true))
        {
            while (!pending_.empty()) 
            {
                // If you delete, you must pop to be consistent
                delete pending_.front();
                pending_.pop_front();
                if (!valid_)
                {
                    return; // cannot unlock dead mutex
                }
            }
            empty_=pending_.empty();
            cond_.broadcast();
            last_event_=-1;
            mutex_.unlock();
        }
    }
}

/* The error handling inside wait_for_change is a bit unique..  The
 * simple interface usally returns true if signalled normally, and
 * false if interrupted or timed out.  Deadlock errors return false,
 * except that every 5th time it returns true as an experiment to
 * shake the calling routine out of a loop.  After 100 deadlock errors
 * it gives up and kills the program.  Deadlocks can only happen if
 * initialized with an errorcheck_mutex and wait_for_change is called
 * from a thread already holding the main lock, or anytime that a
 * re_emit from call_all causes wait_for_change to be called.
 *
 * This will only wait if there are zero events in pending_
 */
bool SafeSignalBuffer::wait_for_change(unsigned long int sec=0, unsigned long int usec=0) const 
{
    _show_args3(this,sec,usec);
    bool result=false;
    static int deadlocks=0;  // used to randomize return value for deadlocks

    if (!valid_) // Check ASAP
        return false;
    switch (mutex_lock(mutex_,true))
    {
        case -1:
            result = (0==(deadlocks++)%5); // every nth deadlock, return true
            if (100<=deadlocks) 
            {
                exit(12);  // give up
            }
            return result;
        case 0:
            break;
        default:
            _never_get_here;
    }
    if (0==pending_.size())
    {
        if (sec||usec) 
        {
            timeval now;
            timespec then;
            if (gettimeofday(&now,0)) 
            {
                cerr << endl << "SafeSignalBuffer::wait_for_change gettimeofday failed" << endl;
                exit(10);
            } 
            else 
            {
                then.tv_sec=now.tv_sec+sec;
                gulong temp_usec=now.tv_usec+usec;
                then.tv_sec += temp_usec / 1000000;
                then.tv_nsec = 1000*(temp_usec % 1000000);
                if (valid_) // check close to blocking call
                {
                    result=(0==cond_.wait(mutex_,&then));
                }
            }
        }
        else 
        {
            if (valid_)  // check close to blocking call
            {
                result=(0==cond_.wait(mutex_));
            }
        }
    }
    else
    {
        result=true;
    }
    mutex_.unlock();
    return result;
}

/// If block is true, use lock, else use trylock
/// A return of -1 if EDEADLK (a notice is sent to cerr)
/// A return of 0 if lock granted
/// A return of +1 if EBUSY
/// exits the program if EINVAL returned (error sent to cerr)
int SafeSignalBuffer::mutex_lock(Mutex& mutex, bool block=true) const
{
    //    _show_args3(this,mutex,block);
    int err;
    if (block)
    {
        err=mutex.lock();
    }
    else 
    {
        err=mutex.trylock();
    }
    switch (err)
    {
        case 0:
            return 0;
        case EINVAL:
            cerr << endl << "SafeSignalBuffer::mutex_lock() failed: " << endl;
            cerr << "The lock returned EINVAL, so the mutex was not properly initialized" << endl;
            cerr << "This is fatal, terminating with code 11" << endl;
            exit(11);
        case EDEADLK:
            cerr << endl << "FYI : SafeSignalBuffer::mutex_lock()" << endl;
            cerr << "FYI : Errorchecking mutex detected deadlock  (EDEADLK)" << endl;
            return -1;
        case EBUSY:
            return +1;
        default:
            _never_get_here;
    }
    return 0==err;
}

// All the magic paramters are passed to the constructor
G_SignalBuffer_EventSource::G_SignalBuffer_EventSource(
    BaseSignalBuffer* sb,gboolean destroy,gint priority,gboolean can_recurse)
    : sb_(sb), event_source_id(0), will_destroy(destroy), timeout_msecs(100),
      call_one_event(false), enabled(true)
{
    g_get_current_time(&next);
    if (NULL!=sb_)
    {
        sb_->reference(); // Since it is a SigC::Object
        // Technically, it is potentially dangerous to give "this"
        // to the external world in a constructor, since another thread
        // could access non-static G_SignalBuffer_EventSource stuff
        // before "this" is done being constructed.  In practice,
        // the object *is* done being constructed (modulo child classes)
        // at this point, so it *is* safe.
        event_source_id=g_source_add(priority,can_recurse,&functions_,sb_,this,NULL);
    }
}

G_SignalBuffer_EventSource::~G_SignalBuffer_EventSource()
{
    if (NULL!=sb_)
    {
        sb_->unreference(); // Since it is a SigC::Object
        if (!g_source_remove (event_source_id))
        {
            // ignore error
        }
    }
}


gboolean G_SignalBuffer_EventSource::prepare_(
    gpointer source_data,GTimeVal *current_time,
    gint *timeout, gpointer user_data)
{
    G_SignalBuffer_EventSource *me=(G_SignalBuffer_EventSource*)user_data;
    if (me->enabled)
    {
        // Set (*timeout)
        if (0<=me->timeout_msecs)
        {
            (*timeout)=me->timeout_msecs;
            return !((BaseSignalBuffer*)source_data)->empty();
        }
        else
        {
            // Are we too early?
            if ((current_time->tv_sec > me->next.tv_sec)
                || ( (current_time->tv_sec == me->next.tv_sec)
                     && (current_time->tv_usec > me->next.tv_usec)))
            {
                (*timeout)=1000*(current_time->tv_sec - me->next.tv_sec);
                (*timeout)+=(me->next.tv_usec - current_time->tv_usec)/1000;
                // Do not bother to check if empty()
                return false;
            }
            else
            {
                // start from now
                me->next=*current_time;
                // go -timeout_msecs into the future
                me->next.tv_usec += 1000 * ((-1 * me->timeout_msecs ) % 1000 );
                me->next.tv_sec += (-1 * me->timeout_msecs ) / 1000;
                // check for wrap-around : sec should become zero or one
                glong sec=(me->next.tv_usec / 1000000);
                if (0!=sec)
                {
                    // correct for wrap-around
                    me->next.tv_usec -= 1000000 * sec;
                    me->next.tv_sec += sec;
                }
                // go for non-blocking main loop on this cycle
                (*timeout) = 0;
                // Check if empty()
                return !((BaseSignalBuffer*)source_data)->empty();
            }
        }
    }
    else
    {
        // Default to 100ms if disabled
        (*timeout)=100;
        return false;
    }
}

gboolean G_SignalBuffer_EventSource::check_(gpointer  source_data,
                   GTimeVal *current_time,
                   gpointer  user_data)

{
    G_SignalBuffer_EventSource *me=(G_SignalBuffer_EventSource*)user_data;
    if (me->enabled)
    {
        if (0<=me->timeout_msecs)
        {
            return !((BaseSignalBuffer*)source_data)->empty();
        }
        else
        {
            // Are we too early?
            if ((current_time->tv_sec > me->next.tv_sec)
                || ( (current_time->tv_sec == me->next.tv_sec)
                     && (current_time->tv_usec > me->next.tv_usec)))
            {
                // Do not bother to check if empty()
                return false;
            }
            else
            {
                // start from now
                me->next=*current_time;
                // go -timeout_msecs into the future
                me->next.tv_usec += 1000 * ((-1 * me->timeout_msecs ) % 1000 );
                me->next.tv_sec += (-1 * me->timeout_msecs ) / 1000;
                // check for wrap-around
                glong sec=(me->next.tv_usec / 1000000);
                if (0!=sec)
                {
                    // correct for wrap-around
                    me->next.tv_usec -= 1000000 * sec;
                    me->next.tv_sec += sec;
                }
                return !((BaseSignalBuffer*)source_data)->empty();
            }
        }
    }
    else
    {
        return false;
    }
}

gboolean G_SignalBuffer_EventSource::dispatch_(gpointer  source_data, 
                          GTimeVal *current_time,
                          gpointer  user_data)
{
    static int count=0;
    ++count;
    _show_args1(count);
    // Paranoid about thread safety
    gdk_threads_enter();
    if (((G_SignalBuffer_EventSource*)user_data)->call_one_event)
    {
        ((BaseSignalBuffer*)source_data)->callOneEvent.emit();
    }
    else
    {
        ((BaseSignalBuffer*)source_data)->callEvents.emit();
    }
    gdk_threads_leave();
    --count;
    return true; // WHAT DOES THE RETURN VALUE MEAN ?
}

void G_SignalBuffer_EventSource::destroy_(gpointer user_data)
{
    if (((G_SignalBuffer_EventSource*)user_data)->call_one_event)
    {
        delete ((G_SignalBuffer_EventSource*)user_data)->sb_;
        // habit...
        ((G_SignalBuffer_EventSource*)user_data)->sb_=NULL;
    }
}

GSourceFuncs G_SignalBuffer_EventSource::functions_ = {
    &G_SignalBuffer_EventSource::prepare_,
    &G_SignalBuffer_EventSource::check_,
    &G_SignalBuffer_EventSource::dispatch_,
    &G_SignalBuffer_EventSource::destroy_
};

/*
$Header: /home/ckuklewicz/cvsroot/gminehunter/SignalBuffer.cc,v 2.11 2000/08/20 00:23:42 ckuklewicz Exp $
$Log: SignalBuffer.cc,v $
Revision 2.11  2000/08/20 00:23:42  ckuklewicz
Numerous bugs,races,lockups, gui logic all fixed
It works great.  I love -O3

Revision 2.10  2000/08/11 04:56:03  ckuklewicz
Worked out a few user interface policy bugs.
Made G_SignalBuffer_EventSource more like a swiss army knife.
(Can look like an event source, a timeout, or an idler)
Need to make timeout_msecs==-1 special.

The seed=0 randomly selected seed is reported in the text box.

Revision 2.9  2000/08/10 04:32:51  ckuklewicz
Reworked the locking in SafeSignalBuffer, removing busy_calling_
(but leaving it defined for now)

Added G_SignalBuffer_EventSource, works great

This will be the tarball version

Revision 2.8  2000/08/07 03:22:33  ckuklewicz
Wow...tons and tons of bugs fixed.
Added the sendPing, received_ping functions.
The COMPUTER_GAME seems to actually work.

Revision 2.7  2000/08/02 20:40:06  ckuklewicz
Changed Buffers to allow emitOneEvent.
Moved GreatGuess code fully into Minehunter.cc
and removed files (to old/)
Now compiles with optMakefile, links with libcln.a

Revision 2.6  2000/07/31 04:22:15  ckuklewicz
Everything compiles and links.  Happy Day
Single user still works.
Need to implement guess_it() from java
The 2^29 limit in cln may need examining

Revision 2.5  2000/07/30 23:23:06  ckuklewicz
Mostly cleaned up!

Revision 2.4  2000/07/28 05:15:10  ckuklewicz
Single Player Works

Revision 2.3  2000/07/27 03:44:05  ckuklewicz
Cleaned up signal buffers with BaseSignalBuffer, added DataBuffer

Revision 2.2  2000/07/24 03:57:04  ckuklewicz
Compile cleanly under -Wall

*/
