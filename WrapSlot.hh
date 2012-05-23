#ifndef INCL_WRAPSLOT_H
#define INCL_WRAPSLOT_H

// So I remember the command ios::setf(ios::unitbuf)
// Needed for namespace WrapSlot
#include <sigc++/signal_system.h>  // signals and slots
#include <sigc++/bind.h>
//#include <sigc++/handle.h>
//#include <sigc++/class_slot.h>

using namespace std;
using namespace SigC;

/**
 * The WrapSlot namespace holds the class AbstractSignalBuffer, 
 * class Wrap, classes Wrap#, and function wrap_slot.  The Wrap0
 * is documented below as a model for the other Wrap#.
 */
namespace WrapSlot {

/**
 * Wrap is the abstract base class whose children will encapsulate a
 * single emit event.  The re_emit() method will send the stored
 * event. No operator() semantics, and named re_emit or call so as not
 * to confused the namespace with a Signal# or a Slot#().  This is
 * also not derived from SigC::Object. Thus it can be applied
 * flexibly.  In general, only the provided Wrap# children are needed.
 * The lifetime of actual Wrap class is usually controlled by some
 * derivative of AbstractSignalBuffer that eventually calls delete
 * explicitly.
 *
 * To consider: add an accept_visitor(visitor * v) {v->visit(this)}
 */
class Wrap {
public:
    Wrap() {};
    virtual ~Wrap() {};
    /**
     * Calling re_emit() is supposed to send the stored parameters to
     * the stored slot. Calling re_emit() several times is explicitly
     * allowed by this documentation.  It may be called from several
     * different threads, but need not guarantee thread safety or
     * behavior if re-entered.  */
    // Slot#.call() not const so re_emit cannot be const().
    virtual void re_emit()=0;
    // re_emit will likely be called from the SignalBuffer while it 
    // holds the lock on the mutex.  If the slot_.call() runs
    // code that tries to access the SignalBuffer, one needs to
    // make the SingalBuffer hold a RECURSIVE not FAST mutex or
    // there will be deadlock.
    // Only if you can can guarantee that delivering the event will not
    // cause new events to be added to sb (or any other access) should
    // you use a FAST mutex.  The one-way pipe semantics imply this
    // safe condition.
};

/**
 * The AbstractSignalBuffer only has a single method that the Wrap#
 * classes need.  The Wrap objects as passed as pointers, and must
 * eventually be deleted by the implementation of the SignalBuffer.
 */
class AbstractSignalBuffer {
public:
    /** There is no return value. The implementation should handle any
     * errors itself, and it should always eventually delete the
     * passed pointer.
     */
    virtual void store_wrapped_event(Wrap *)=0;
};

/**
 * The BaseSignalBuffer does not implement the abstract store_wrapped_event
 * call but it does provide signal sematics for descendent signal buffers.
 */
class BaseSignalBuffer : public AbstractSignalBuffer, public SigC::Object {
public:
    /**
     * When this signal emits, the signal buffer should call all the
     * pending wrapped events.  Note: do not call clear() on this or you
     * may break the signal buffer.
     */
    Signal0<void> callEvents;
    /**
     * When this signal emits, the signal buffer should send the next
     * waiting event.  Note: do not call clear() on this or you may
     * break the signal buffer.
     */
    Signal0<void> callOneEvent;
    /**
     * The destroy signal is called when the AbstractSignalBuffer is
     * being destructed.
     */
    Signal0<void> destroy;
    /// Triggering the destroy signal is handled in this dtor
    virtual ~BaseSignalBuffer() {
        destroy.emit();
    };
    /// The empty_ and size methods allow other routines to
    /// handle SignalBuffers in a generic way.
    virtual bool empty() const=0;
    virtual size_t size() const=0;
};

// These functions allow the detroy signal from one object
// to delete another object
namespace DeleteCallback {
extern void delete_victim(void *victim);
extern Slot0<void> delete_slot(void *victim);
};

//////////////////////////////////////////////////////////////////////

/**
 * The Wrap0 is not a template since Slot0<void> is a definite type.
 * Documentaion is provided for this class, the other Wrap# are then
 * obvious.
 */
class Wrap0 : public Wrap {
public: 
    /// Useful typedefs in all Wrap# classes
    //@{
    /// The type of Slot that is being wrapped
    typedef Slot0<void> SlotType;
    /// The static class function pointer type of the store function
    typedef void (*StoreFunc) (SlotType,AbstractSignalBuffer * const);
    /// The type of Slot that refers to the store function
    typedef Slot2<void,SlotType,AbstractSignalBuffer * const> StoreType;
    //@}
protected:
    /// This is the final destinal slot that re_emit() calls. Set in constructor
    SlotType slot_;
public:
    /// The constructor ought to be protected....use the store factory method */
    Wrap0(const SlotType& s);
    /// Goes Nowhere, Does Nothing
    // virtual ~Wrap0();
    /// This implements the Wrap::re_emit() call
    virtual void re_emit();
    /**
     * This is the factory for creating the Wrap# objects.  It also
     * passes them to the AbstractSignalBuffer so they can be managed.
     * If the AbstractSignalBuffer is null, then the call is NOT
     * wrapped, but rather delivered immediately.  The user does not
     * call this, but rather a signal is connected by wrap_slot which
     * does.  This matches the "unbuffered" semantics one might expect
     * when there is no (i.e. a null) buffer.
     */
    static void store(SlotType s, AbstractSignalBuffer * const sb);
    /**
     * The wrap_slot factory creates new slots for old.  The dest_slot
     * and AbstractSignalBuffer* are bound to the new returned slot
     * that refers to the store factory method.  The
     * WrapSlot::wrap_slot helper functions are overloaded to make
     * calling this easier.  
     * 
     * TODO: Ask if there is a way to cache the result of a single 
     * slot(&Wrap0::store) call to increase efficiency.  Some kind
     * of static declaration in wrap_slot or Wrap#, perhaps?
     */
    inline static SlotType wrap_slot(const SlotType& dest_slot, AbstractSignalBuffer  * const sb) {
        // This only modifies the target slot; the connection is not
        // made until after this function is returned; so store cannot
        // be called until wrap_slot returns.  Rather than binding so
        // many parameters a SlotBufferProxy class be made to hold the
        // paramters, and then return a member function in the Proxy.
        // The bind avoids writing such an extra class (whose lifetime
        // would be difficult to manage)
        return bind(slot(&store),dest_slot,sb);
    };
};

/**
 * This is the first of many overloaded / templated functions in the WrapSlot
 * namespace that act like bind to automatically select the right Wrap#
 * and template parameters.
 */
inline Slot0<void> wrap_slot(const Slot0<void>& dest_slot, AbstractSignalBuffer  * const sb) {
    // Again, the slot is not connected until after this returns
    return  Wrap0::wrap_slot(dest_slot, sb);
};

//////////////////////////////////////////////////////////////////////

template<class P1>
class Wrap1 : public Wrap {
public: 
    typedef Slot1<void,P1> SlotType;
    typedef void (*StoreFunc) (P1,SlotType,AbstractSignalBuffer* const);
    typedef Slot3<void,P1,SlotType,AbstractSignalBuffer* const> StoreType;
protected:
    P1 p1_;
    SlotType slot_;
public:
    Wrap1(const P1& p1, const SlotType& s);
    virtual void re_emit();
    static void store(P1 p1, SlotType s, AbstractSignalBuffer* const sb);
    inline static SlotType wrap_slot(const SlotType& dest_slot, AbstractSignalBuffer* const sb) { 
        return bind(slot(&store),dest_slot,sb);
    };
};

template<class P1>
Slot1<void,P1> wrap_slot(const Slot1<void,P1>& dest_slot, AbstractSignalBuffer* const sb) {
    return  Wrap1<P1>::wrap_slot(dest_slot, sb);
};

//////////////////////////////////////////////////////////////////////

template<class P1, class P2>
class Wrap2 : public Wrap {
public: 
    typedef Slot2<void,P1,P2> SlotType;
    typedef void (*StoreFunc) (P1,P2,SlotType,AbstractSignalBuffer* const);
    typedef Slot4<void,P1,P2,SlotType,AbstractSignalBuffer* const> StoreType;
protected:
    P1 p1_;
    P2 p2_;
    SlotType slot_;
public:
    Wrap2(const P1& p1, const P2& p2, const SlotType& s);
    virtual void re_emit();
    static void store(P1 p1, P2 p2,SlotType s, AbstractSignalBuffer* const sb);
    static SlotType wrap_slot(const SlotType& dest_slot, AbstractSignalBuffer* const sb) { 
        return bind(slot(&store),dest_slot,sb);
    };
};

template<class P1, class P2>
inline Slot2<void,P1,P2> wrap_slot(const Slot2<void,P1,P2>& dest_slot, AbstractSignalBuffer* const sb) {
    return  Wrap2<P1,P2>::wrap_slot(dest_slot, sb);
};

//////////////////////////////////////////////////////////////////////

template<class P1, class P2, class P3>
class Wrap3 : public Wrap {
public: 
    typedef Slot3<void,P1,P2,P3> SlotType;
    typedef void (*StoreFunc) (P1,P2,P3,SlotType,AbstractSignalBuffer* const);
    typedef Slot5<void,P1,P2,P3,SlotType,AbstractSignalBuffer* const> StoreType;
protected:
    P1 p1_;
    P2 p2_;
    P3 p3_;
    SlotType slot_;
public:
    Wrap3(const P1& p1,const P2& p2,const P3& p3, SlotType s);
    virtual void re_emit();
    static void store(P1 p1, P2 p2, P3 p3,SlotType s, AbstractSignalBuffer* const sb);
    inline static SlotType wrap_slot(const SlotType& dest_slot, AbstractSignalBuffer* const sb) { 
        return bind(slot(&store),dest_slot,sb);
    };
};

template<class P1, class P2, class P3>
inline Slot3<void,P1,P2,P3> wrap_slot(const Slot3<void,P1,P2,P3>& dest_slot, AbstractSignalBuffer* const sb) {
    return  Wrap3<P1,P2,P3>::wrap_slot(dest_slot, sb);
};
    
};  // End of namespace WrapSlot

//////////////////////////////////////////////////////////////////////
#endif

/*
  $Header: /home/ckuklewicz/cvsroot/gminehunter/WrapSlot.hh,v 2.7 2000/08/10 04:32:51 ckuklewicz Exp $
  $Log: WrapSlot.hh,v $
  Revision 2.7  2000/08/10 04:32:51  ckuklewicz
  Reworked the locking in SafeSignalBuffer, removing busy_calling_
  (but leaving it defined for now)

  Added G_SignalBuffer_EventSource, works great

  This will be the tarball version

  Revision 2.6  2000/08/02 20:40:06  ckuklewicz
  Changed Buffers to allow emitOneEvent.
  Moved GreatGuess code fully into Minehunter.cc
  and removed files (to old/)
  Now compiles with optMakefile, links with libcln.a

  Revision 2.5  2000/07/31 04:22:15  ckuklewicz
  Everything compiles and links.  Happy Day
  Single user still works.
  Need to implement guess_it() from java
  The 2^29 limit in cln may need examining

  Revision 2.4  2000/07/28 05:15:10  ckuklewicz
  Single Player Works

  Revision 2.3  2000/07/27 03:44:05  ckuklewicz
  Cleaned up signal buffers with BaseSignalBuffer, added DataBuffer

  Revision 2.2  2000/07/24 03:57:04  ckuklewicz
  Compile cleanly under -Wall

*/


