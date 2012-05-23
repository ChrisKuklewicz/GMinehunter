#ifndef INCL_COMMCHANNEL_H
#define INCL_COMMCHANNEL_H

#include "Minehunter.hh"
#include <pthread.h>
#include <glib.h>

/* Reminder from glib.h
struct GTimeVal
 {
   glong tv_sec;
   glong tv_usec;
 };
*/

/** 
 * This is a simple wrapper for a pointer to a glib mutex (a fast mutex).
 * The mutex is created and destroyed with the wrapper.
 * All is public, since you have to know what you are doing anyway...
 */
class G_Mutex {
    GMutex *_mutex;
    G_Mutex() { _mutex=g_mutex_new(); };
    virtual ~G_Mutex() { g_mutex_free(_mutex); };
    void lock() { g_mutex_lock(_mutex); };
    bool trylock() { return g_mutex_trylock(_mutex)?true:false; };
    void unlock() { g_mutex_unlock(_mutex); };
    GMutex* get_mutex() { return _mutex };
}

/**
 * This is a simple wrapper around a GCond.
 */
class G_Cond 
{
    GCond *_cond;
    G_Cond() { _cond=g_cond_new(); };
    virtual ~G_Cond() { g_cond_free(_cond); };
    void signal() { g_cond_signal(_cond); };
    void broadcast() { g_cond_broadcast(_cond); };
    void wait(G_Mutex* mutex) { g_cond_wait(_cond,mutex->get_mutex()); };
    void wait(GMutex* mutex) { g_cond_wait(_cond,mutex); };
    bool timed_wait(G_Mutex* mutex, GTimeVal *abs_time) { 
	return g_cond_timed_wait(_cond,mutex->get_mutex(),abs_time)?true:false; };
    bool timed_wait(GMutex* mutex, GTimeVal *abs_time) { 
	return g_cond_timed_wait(_cond,_mutex,abs_time)?true:false; };
}

/**
 * This is a very simple wrapper for a mutex / condition pair.
 * It supports the most common use of a condition, where it is
 * always associated with the same "parent" mutex.
 */
class G_CondMutex :  public G_Mutex, public G_Cond 
{
    bool wait() { g_cond_wait(_cond,_mutex); };
    /// This waits until absolute time passed or until signaled.
    bool timed_wait(GTimeVal *abs_time) { g_cond_wait(_cond,_mutex,abs_time); };
    /// This waits until secs seconds + usecs microseconds in the future
    bool timed_wait(glong secs, glong usecs);
}

bool G_CondMutex::timed_wait(glong secs, glong usecs)	
{
    GTimeVal theTime;
    g_get_current_time(&theTime);
    theTime.tv_sec+=secs;
    theTime.tv_usec+=usecs;
    return timed_wait(theTime);
}

/// This is the function type that is called when a thread is created
typedef void  * (* p_thread_function)(void *);

/// This declared the standard function to start a P_Thread object
void * p_thread_start(void * p_thread);

/** 
 * This is a hacked wrapper to create a more "Java" like worker
 * thread abstraction.  Subclasses override the run() member function.
 */
class P_Thread 
{
public:
    /** Prepares attr with the default attributes with pthread_attr_init
     * You should not need to override this for normal use.     */
    P_Thread();
    /** Destroys attr with pthread_attr_destroy
     * This does nothing to join/stop/kill/cancel the thread.     */
    virtual ~P_Thread();

    /// This can be used by the parent to refer to the child thread
    pthread_t thread;
    /// This holds the attributes used to create the thread
    pthread_attr_t attr;
    /// This holds the pointer that would normally be passed to pthread_start
    void * data;
    /** This refers to a helper function to pass to pthread_start
     * that will in turn call run on this P_Thread object it is passed */
    p_thread_function function;
    /**
     * This calls pthread_start with the helper function and "this" as
     * the data.  The some_data parameter is stored in this->data to be
     * accessed by the this->run() function.  Runs in the parent thread.
     */
    virtual void * start(void * some_data);
    /* The run function as defined here simply returns the data value.
     * You must override the run function in a child class with the 
     * desired behavior. Runs in the child thread.
     */
    virtual void * run() { return data; };
}


///////////////////////////////////////////////////////////////////


void * p_thread_start(void * p_thread)
{
    return ((P_Thread*)p_thread)->run();
}


void * P_Thread::start(void * some_data); { 
    data=some_data;
    return pthread_create(&thread,attr,function,this);
}

P_Thread::P_Thread() : 
    data (NULL), function(p_thread_start)
{
    if (int err=pthread_attr_init(&attr)) {
	cerr << endln << "P_Thread is dead, pthread_att_destroy returned error " << err << endln;
	exit(err);
    }
}

P_Thread::~P_Thread()
{
    if (int err=pthread_attr_destroy(&attr)) {
	cerr << endln << "P_Thread is dead, pthread_attr_destroy returned error " << err << endln;
	exit(err);
    }

}

/////////////////////////////////////////////////////////////////////////

#endif


/*
$Header$
$Log$
*/
