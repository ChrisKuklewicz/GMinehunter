#ifndef INCL_SIGNAL_BUFFER_H
#define INCL_SIGNAL_BUFFER_H

// Pull in SigC threads for C++ wrapper instead of my own
#define SIGC_THREAD_IMPL 1
#include <sigc++/thread.h>
// Get the WrapSlot
#include "WrapSlot.hh"
// Needed for SimpleSignalBuffer
#include <deque.h>  // STL
#include <glib.h> // Just for G_SignalBuffer_EventSource

using namespace std;
using namespace SigC;
using namespace Threads;
using namespace WrapSlot;

class SimpleSignalBuffer : public BaseSignalBuffer {
public:
    typedef deque<WrapSlot::Wrap*> Queue;
protected:
    /// This is the index of the last event called by call_one_event()
    int last_event_;
    bool discard_;
    Queue pending_;
public:
    void call_all();
    void call_one_event();
    void discard_all();
    explicit SimpleSignalBuffer(bool discard=true) : discard_(discard), pending_() 
    {
        callEvents.connect(slot(this,&SimpleSignalBuffer::call_all));
        callOneEvent.connect(slot(this,&SimpleSignalBuffer::call_one_event));
    };
    virtual ~SimpleSignalBuffer() {
	discard_all();
    };
    /// A NULL parameter is ignored.  Never store many copies of the
    /// same pointer since it will be passes to delete once for each
    /// copy.
    virtual void store_wrapped_event(WrapSlot::Wrap *w) {
	if (w)
	    pending_.push_back(w);
    };
    bool empty() const { return pending_.empty(); };
    size_t size() const { return pending_.size(); };
    void set_discard(bool discard) { discard_=discard; };
private:
    /// No copy or assignment allowed
    SimpleSignalBuffer(const SimpleSignalBuffer& sb) : pending_() {};
    /// No copy or assignment allowed
    SimpleSignalBuffer& operator=(const SimpleSignalBuffer& handle) { return *this; };
};


extern const MutexAttr fast_mutex;
extern const MutexAttr recursive_mutex;
extern const MutexAttr errorcheck_mutex;

/**
 * SafeSignalBuffer is an awefully paranoid thread safe implementation
 * of AbstractSignalBuffer.  Memory will not leak, the destructor will
 * exit the program to avoid deadlock or leaks.  Threads will not
 * deadlock themselves or each other.  One Achilles' heel is if a
 * pointer is stored twice, since the second access will be to a
 * deleted object.  Since normal Wrap elements are added once via the
 * static store function slot, this should not be a problem.  The
 * valid_ flag (indicating the destructor has been called) is checked
 * for in a paranoid fashion.  All interesting return values are
 * checked for an error and any such are reported to cerr.
 */
class SafeSignalBuffer : public BaseSignalBuffer {
public:
    /// The type of the Queue of pending events
    /// The pointers are owned by the Queue and must be deleted below
    typedef deque<WrapSlot::Wrap*> Queue;

    /// Empty AbstractSignalBuffer
    explicit SafeSignalBuffer(bool discard=true,const MutexAttr type_mutex = recursive_mutex);

    /// This wants to lock pending to erase all the events,
    /// but it cannot block, only pause.
    virtual ~SafeSignalBuffer();

    /// A NULL parameter is ignored.  Never store many copies of the
    /// same pointer since it will be passes to delete once for each
    /// copy.
    virtual void store_wrapped_event(WrapSlot::Wrap *w);

    /// Indicates when destructor is active, if false. No locking.
    inline bool valid() const { return valid_; };

    inline void set_discard(bool discard) { discard_=discard; }

    /// Empty can be checked without getting a lock
    inline bool empty() const { return ((!valid_)||empty_); };

    /// Number of pending events (requires a lock to read)
    size_t size() const;

    /// This delivers all pending events, and may discard or save them
    void call_all();

    /// This delivers only a single event
    void call_one_event();

    /// This destroys all pending events without delivering
    void discard_all();

    /// Returns true if signaled, false if timed out or aborted
    bool wait_for_change(unsigned long int sec=0, unsigned long int usec=0) const;

protected:
    /// This is the index of the last event called by call_one_event()
    int last_event_;
    /// This controls whether the events are deleted when delivered
    bool discard_;
    /// valid_ is set to true initially and false at start of destructor
    bool valid_;
    /// empty_ is a read-only flag to help avoid locking
    bool empty_;

    MutexAttr m_a,b_a;
    /// read/write operations on pending are guarded by this Mutex.
    mutable Mutex mutex_;
    /// this allows threads to block, waiting pending_ to change
    mutable Condition cond_;
    /// This stores the pointer to the Wrap events (owned by pending_)
    Queue pending_;
private:
    /// To avoid call_call events triggering recursive access, create a private
    /// errorcheck_mutex to synchonize access.
    mutable Mutex busy_calling_;

    int mutex_lock(Mutex& mutex, bool block=true) const;

    /// No copy or assignment allowed
    SafeSignalBuffer(const SafeSignalBuffer& sb) {};
    /// No copy or assignment allowed
    SafeSignalBuffer& operator=(const SafeSignalBuffer& handle) { return *this; };
};


/** Class wrapper for static methods to interface to gnome and gtk and
 * glib's event loop.  Essentially a wrapper for g_source_add.
 * Instance of this class register a SafeSignalBuffer in ctor and
 * unregister in dtor.  The lifetime of this class should never exceed
 * the lifetime of the signal buffer.  To aid in this goal, this class
 * can be asked to delete the signal buffer.
 *
 * This class ought to be multithread safe if you use a SafeSignalBuffer.
 */
class G_SignalBuffer_EventSource
{
protected:
    /// prepare return !sb_->empty()
    static gboolean prepare_(gpointer  source_data, 
                      GTimeVal *current_time,
                      gint     *timeout,
                      gpointer  user_data);
    /// prepare return !sb_->empty()
    static gboolean check_(gpointer  source_data,
                   GTimeVal *current_time,
                   gpointer  user_data);
    static gboolean dispatch_(gpointer  source_data, 
                       GTimeVal *current_time,
                       gpointer  user_data);
    static void destroy_(gpointer user_data);
    // I would make this constant but glib is c code and
    // the "discards" const warnings are annoying
    static GSourceFuncs  functions_;
    BaseSignalBuffer* sb_;
    guint event_source_id;
    GTimeVal next;
public:
    /** This class does nothing if sb is NULL, otherwise an event
     * source is added with g_source_add with the passed priority and
     * can_recurse values. If the destroy parameter is true then this
     * class will delete the signal buffer when it is removed from the
     * event loop.  This can be changed by toggling the will_destroy
     * member variable.
     */
    G_SignalBuffer_EventSource(BaseSignalBuffer* sb, gboolean destroy=false,
                               gint priority=G_PRIORITY_DEFAULT,
                               gboolean can_recurse=false);
    /** The destructor calls g_source remove.  The deletion of a
     * managed signalbuffer is handled by a static GDestroyNotify
     * callback called by glib, not by the dtor itself.
     */
    virtual ~G_SignalBuffer_EventSource();
    gboolean will_destroy;
    /** 
     * The timeout_msecs is the value in millisecods that the prepare_
     * call sends to the event loop.  The event loop will block for no
     * longer than timeout before checking the event souce.  This
     * class has a default value of 100 ms set in the ctor.
     *
     * If timeout_msecs is negative then it is interpreted
     * differently.  A negative time puts this class into "timer"
     * mode, and it checks if it is ready no sooner than after a delay
     * of the absolute value timeout_msecs since the last time it was
     * ready.
     *
     * A zero value of timeout_msecs is "idle" mode, and the event
     * will force the main to never block.  In this case, an IDLE
     * priority passed to the ctor may be appropriate.
     */
    gint timeout_msecs;
    // If false then callEvents is emitted, else callOneEvent is
    // emitted. Defaults to false in ctor.
    gboolean call_one_event;
    /// This can be used to enable or disable calling events
    /// If false, then the prepare and check functions return false
    /// (prepare also will return a 100ms timeout).
    /// enable is set to true in ctor.
    gboolean enabled;
private:
    /// no copy
    G_SignalBuffer_EventSource(const G_SignalBuffer_EventSource& other) {};
    /// no assignment
    G_SignalBuffer_EventSource& operator=(
        const G_SignalBuffer_EventSource& other) { return *this; };
};


#endif

/*
$Header: /home/ckuklewicz/cvsroot/gminehunter/SignalBuffer.hh,v 2.8 2000/08/20 00:23:42 ckuklewicz Exp $
$Log: SignalBuffer.hh,v $
Revision 2.8  2000/08/20 00:23:42  ckuklewicz
Numerous bugs,races,lockups, gui logic all fixed
It works great.  I love -O3

Revision 2.7  2000/08/11 04:56:03  ckuklewicz
Worked out a few user interface policy bugs.
Made G_SignalBuffer_EventSource more like a swiss army knife.
(Can look like an event source, a timeout, or an idler)
Need to make timeout_msecs==-1 special.

The seed=0 randomly selected seed is reported in the text box.

Revision 2.6  2000/08/10 04:32:51  ckuklewicz
Reworked the locking in SafeSignalBuffer, removing busy_calling_
(but leaving it defined for now)

Added G_SignalBuffer_EventSource, works great

This will be the tarball version

Revision 2.5  2000/08/02 20:40:06  ckuklewicz
Changed Buffers to allow emitOneEvent.
Moved GreatGuess code fully into Minehunter.cc
and removed files (to old/)
Now compiles with optMakefile, links with libcln.a

Revision 2.4  2000/07/28 05:15:10  ckuklewicz
Single Player Works

Revision 2.3  2000/07/27 03:44:05  ckuklewicz
Cleaned up signal buffers with BaseSignalBuffer, added DataBuffer

Revision 2.2  2000/07/24 03:57:04  ckuklewicz
Compile cleanly under -Wall

Revision 2.1  2000/07/24 02:14:28  ckuklewicz
Split all into hh and cc

Revision 2.0  2000/07/22 17:21:14  ckuklewicz
Synchonizing release numbers

*/
