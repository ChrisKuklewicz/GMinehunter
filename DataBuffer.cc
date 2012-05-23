#include "DataBuffer.hh"
#include <errno.h>

DataBuffer::DataBuffer(Mode mode = LAST_RECEIVED) : 
    mutex_(recursive_mutex), mode_(mode), event_(NULL) 
{
    callEvents.connect(slot(this,&DataBuffer::call_all));
    callOneEvent.connect(slot(this,&DataBuffer::call_all));
}

DataBuffer::~DataBuffer()
{
    if ((NULL!=event_) && mutex_lock())
    {
        delete event_;
        event_=NULL;
        mutex_.unlock();
    }
}

void DataBuffer::store_wrapped_event(Wrap* wrap) 
{
    if ((NULL!=wrap) && (event_!=wrap) && mutex_lock())
    {
        if (NULL!=event_)
        {
            if (FIRST_RECEIVED!=mode_)
            {
                delete event_;
                event_=wrap;
                wrap=NULL;
            }
            else
            {
                delete wrap;
            }
        }
        else
        {
            event_=wrap;
        }
        mutex_.unlock();
    }
}
    
void DataBuffer::call_all()
{
    if ((NULL!=event_) && mutex_.lock())
    {
        if (REPEAT_LAST!=mode_) {
            Wrap *fire = event_;
            event_=NULL;
            fire->re_emit();
            delete fire;
        } 
        else 
        {
            event_->re_emit();
        }
        mutex_.unlock();
    }
}

void DataBuffer::discard_all()
{
    if ((NULL!=event_) && mutex_.lock())
    {
        delete event_;
        mutex_.unlock();
    }
}

bool DataBuffer::mutex_lock() {
    int err=mutex_.lock();
    if (0!=err) {
        if (EINVAL==err) {
            cerr << endl << "DataBuffer::mutex_lock() failed: " << endl;
            cerr << "The lock returned EINVAL, so the mutex was not properly initialized" << endl;
            cerr << "This is fatal, terminating with code 11" << endl;
            exit(11);
        }
    }
    return 0==err;
}

