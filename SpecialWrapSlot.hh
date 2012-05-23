#include "WrapSlot.cc"

#include <iostream.h>

using namespace WrapSlot;
/*
template <class Wrapped, class P1>
class Notifier : public Wrapped {
public:
    typedef typename Wrapped::SlotType ParentSlotType;
public:
//    Notifier(const ParentSlotType& slot) : Wrapped(slot) {
//        cout << "born" << endl;
//    };
    Notifier(const P1& p1, const ParentSlotType& slot) : Wrapped(p1,slot) {
        cout << "born 1" << endl;
    };
    virtual void re_emit() {
        Wrapped::re_emit();
    };
public:
    virtual ~Notifier() {
        cout << "died" << endl;
    };
};
*/
void a_slot()
{
    cout << "a_slot" << endl;
}


void b_slot(int value)
{
    cout << "b_slot value=" << value << endl;
}

int main(int argc, char** argv) {
    Wrap* event;
    int i=17;
//    event = new Notifier<Wrap0>(slot(&a_slot));
//    event->re_emit();
//    delete event;
    event=new Wrap1<int>(17,slot(&b_slot));
//    event=new Notifier<Wrap1<int>,int>(17,slot(&b_slot));
    event->re_emit();
    delete event;
    i=18;
    Wrap1<int> ev (i,slot(&b_slot));
    ev.re_emit();
}



