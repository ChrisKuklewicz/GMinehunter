#include "WrapPointer.hh"

namespace WrapSlot {

template<class T>
WrapPointer<T>::WrapPointer(T* p1, SlotType& s) : 
    p1_(p1), 
    slot_(s) {};
	
template<class T>
WrapPointer<T>::~WrapPointer()
{
    if (NULL!=p1_)
    {
        delete p1_;  // WrapPointer died before delivery with re_emit()
        p1_=NULL;
    }
}

template<class T>
void WrapPointer<T>::re_emit() { 
    T* p(p1_);       // temporary pointer p
    p1_=NULL;        // if re_emit() is recursive, only deliver once!
    slot_.call(p);   // it is now the problem of the receiver to delete
}

template<class T>
void WrapPointer<T>::store(T* p1, SlotType s, AbstractSignalBuffer* const sb) {
    if (sb)
    {
        sb->store_wrapped_event(new WrapPointer(p1,s));
    }
    else
    {
        s.call(p1);
    }
}

}; // end namespace WrapSlot


