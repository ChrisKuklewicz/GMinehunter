#ifndef INCL_WRAP_POINTER_HH
#define INCL_WRAP_POINTER_HH

#include "WrapSlot.hh"

namespace WrapSlot {

template <typename T>
class WrapPointer : public Wrap
{
public: 
    typedef Slot1<void,T*> SlotType;
    typedef void (*StoreFunc) (T*,SlotType,AbstractSignalBuffer* const);
    typedef Slot3<void,T*,SlotType,AbstractSignalBuffer* const> StoreType;
protected:
    T* p1_;
    SlotType slot_;
public:
    WrapPointer(T* p1, SlotType& s);
    virtual ~WrapPointer();
    virtual void re_emit();
    static void store(T* p1, SlotType s, AbstractSignalBuffer* const sb);
    inline static SlotType wrap_slot(const SlotType& dest_slot, AbstractSignalBuffer* const sb) { 
        return bind(slot(&store),dest_slot,sb);
    };
};

template<class T>
Slot1<void,T*> wrap_pointer_slot(const Slot1<void,T*>& dest_slot, AbstractSignalBuffer* const sb) {
    return  WrapPointer<T>::wrap_slot(dest_slot, sb);
};


}; // end namespace WrapSlot

#endif
