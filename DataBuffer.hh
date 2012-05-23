#include "SignalBuffer.hh"

/**
 * DataBuffer holds only a single wrapped event at a time.  Whether
 * new events replace old ones is part of the policy set in the
 * constructor.  Three different semantics are provided.
 *
 * This class allows one way state to be reported via signals and slots instead
 * of via a guarded shared data object that one thread writes and the
 * other reads.
 *
 * Example: The worker thread reports progress percents to the GUI.
 * Send the signal with the updated percent parameter into this
 * buffer.  The GUI thread can periodically callEvents on this buffer
 * to deliver the LAST_RECEIVED update event to the progress bar
 * widget.  Multiple updates from the worker get coalesced so only the
 * most recent is sent.  If no updates occur, then the widget never
 * gets called.
 *
 * Another use could be put this in REPEAT_LAST mode and setup a timer to
 * periodically callEvents.  *Any* event that returns void can be loaded
 * into the DataBuffer and be called periodically.  To stop, just delete_all(). 
 * You could even delete the DataBuffer and have the periodic timer
 * listed for the destroy signal.  Or just subclass this to roll the timer 
 * into the same class. 
 *
 * To simplify having many Buffers, simply connect as so:
 *
 * SafeSignalBuffer *pipe = new SafeSignalBuffer();
 * DataBuffer *data_a = new DataBuffer();
 * DataBuffer *data_b = new DataBuffer();
 * pipe->callEvents.connect(data_a->callEvents.slot());
 * pipe->callEvents.connect(data_b->callEvents.slot());
 * pipe->destroy.connect(DeleteCallback::delete_slot(data_a));
 * pipe->destroy.connect(DeleteCallback::delete_slot(data_b));
 *
 * So pipe.callEvents.emit() triggers pipe and data_a and data_b.
 * This technique is why BaseSignalBuffer was created with the callEvents and
 * destroy signals.  Later just call delete pipe to clean up.
 */


class DataBuffer : public WrapSlot::BaseSignalBuffer {
public:
    /// The Mode controls the policy of the DataBuffer
    enum Mode {
        /// New events overwrite old and the last received
        /// is sent when callEvents is triggered. Then the
        /// stored Wrap* is deleted
        LAST_RECEIVED = 0,
        /// The first Wrap* stored is kept, and later ones
        /// are deleted instead of stored.  Thus the first
        /// received is sent when callEvents is triggered.
        /// The the stored Wrap* is deleted and the DataBuffer
        /// is ready to store the first subsequent event.
        FIRST_RECEIVED = 1,
        /// This is like LAST_RECEIVED but the event is 
        /// not deleted after being sent. So the same event
        /// may be sent several times.
        REPEAT_LAST = 2
    };
    explicit DataBuffer(Mode mode = LAST_RECEIVED);
    virtual ~DataBuffer();
    /// A NULL paramter is ignored.  A pointer the same as the
    /// stored pointer is ignored.
    virtual void store_wrapped_event(Wrap* wrap);
    /// Looks thread safe without a lock, as other functions
    /// only check mode_ in at one place each.
    void set_mode(Mode mode) { mode_=mode; };
    // For consistency, keep same methods as SimpleSignalBuffer
    void call_all();
    void discard_all();
    bool empty() const { return (NULL==event_); };
    unsigned int size() const { return ((NULL==event_)?0:1); };
protected:
    mutable Mutex mutex_;
    Mode mode_;
    Wrap *event_;
    /// This is a helper function that check for some errors
    /// Returns true if the mutex_ lock is granted, false if
    /// there was an error.
    bool mutex_lock();
};



/* 
   $Header: /home/ckuklewicz/cvsroot/gminehunter/DataBuffer.hh,v 2.3 2000/08/02 20:40:05 ckuklewicz Exp $
   $Log: DataBuffer.hh,v $
   Revision 2.3  2000/08/02 20:40:05  ckuklewicz
   Changed Buffers to allow emitOneEvent.
   Moved GreatGuess code fully into Minehunter.cc
   and removed files (to old/)
   Now compiles with optMakefile, links with libcln.a

   Revision 2.2  2000/07/28 05:15:09  ckuklewicz
   Single Player Works

*/
   
