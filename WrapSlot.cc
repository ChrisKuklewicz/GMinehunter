// So I remember the command ios::setf(ios::unitbuf)
#include "WrapSlot.hh"

namespace WrapSlot 
{

//////////////////////////////////////////////////////////////////////

Wrap0::Wrap0(const SlotType& s) : 
    slot_(s) {}

// virtual Wrap0::~Wrap0() { /* slot_ should auto-destruct */ };

void Wrap0::re_emit() { 
    slot_.call(); 
}

void Wrap0::store(SlotType s, AbstractSignalBuffer * const sb) {
    // Note how no objects but the SignalBuffer and the Wrap 
    // itself know how to access the wrapped event.
    // So all outside access coms via the SignalBuffer, and that
    // makes it easy to protect with a mutex.
    if (sb)
        // Note: Object is constructed, then 
        // passes to sb, which can lock the mutex,
        // add to list,
        // then unlock the mutex and return
        sb->store_wrapped_event(new Wrap0(s));
    else
        s.call();
    // At this point, the sb could be calling the event, even before
    // this store function returns, but this is safe.
}

//////////////////////////////////////////////////////////////////////

template<class P1>
Wrap1<P1>::Wrap1(const P1& p1, const SlotType& s) : 
    p1_(p1), slot_(s) {}
	
template<class P1>
void Wrap1<P1>::re_emit() { 
    slot_.call(p1_); 
}

template<class P1>
void Wrap1<P1>::store(P1 p1, SlotType s, AbstractSignalBuffer* const sb) {
    if (sb)
        sb->store_wrapped_event(new Wrap1(p1,s));
    else
        s.call(p1);
}

//////////////////////////////////////////////////////////////////////

template<class P1, class P2>
Wrap2<P1,P2>::Wrap2(const P1& p1, const P2& p2, const SlotType& s) : 
    p1_(p1), p2_(p2), slot_(s) {}

template<class P1, class P2>
void Wrap2<P1,P2>::re_emit() { 
    slot_.call(p1_,p2_); 
}

template<class P1, class P2>
void Wrap2<P1,P2>::store(P1 p1, P2 p2,SlotType s, AbstractSignalBuffer* const sb) {
    if (sb)
        sb->store_wrapped_event(new Wrap2(p1,p2,s));
    else
        s.call(p1,p2);
}

//////////////////////////////////////////////////////////////////////

template<class P1, class P2, class P3>
Wrap3<P1,P2,P3>::Wrap3(const P1& p1,const P2& p2,const P3& p3, SlotType s) : 
    p1_(p1), p2_(p2), p3_(p3), slot_(s) {}

template<class P1, class P2, class P3>
void Wrap3<P1,P2,P3>::re_emit() { 
    slot_.call(p1_,p2_,p3_); 
}

template<class P1, class P2, class P3>
void Wrap3<P1,P2,P3>::store(P1 p1, P2 p2, P3 p3,SlotType s, AbstractSignalBuffer* const sb) {
    if (sb)
        sb->store_wrapped_event(new Wrap3(p1,p2,p3,s));
    else
        s.call(p1,p2,p3);
}

//////////////////////////////////////////////////////////////////////


namespace DeleteCallback {

void delete_victim(void *victim)
{
    delete victim;
}

Slot0<void> delete_slot(void *victim)
{
    return bind(slot(&delete_victim), victim);
}

};


} // End of namespace WrapSlot
/*
$Header: /home/ckuklewicz/cvsroot/gminehunter/WrapSlot.cc,v 2.6 2000/08/07 03:22:33 ckuklewicz Exp $
$Log: WrapSlot.cc,v $
Revision 2.6  2000/08/07 03:22:33  ckuklewicz
Wow...tons and tons of bugs fixed.
Added the sendPing, received_ping functions.
The COMPUTER_GAME seems to actually work.

Revision 2.5  2000/07/28 05:15:10  ckuklewicz
Single Player Works

Revision 2.4  2000/07/27 03:44:05  ckuklewicz
Cleaned up signal buffers with BaseSignalBuffer, added DataBuffer

Revision 2.3  2000/07/24 03:57:04  ckuklewicz
Compile cleanly under -Wall

Revision 2.1  2000/07/22 17:33:07  ckuklewicz
Added

*/
