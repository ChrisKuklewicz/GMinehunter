#ifndef INCL_MINEHUNTER_H
#define INCL_MINEHUNTER_H
#define SIGC_THREAD_IMPL 1
#include <sigc++/thread.h>

#include "mh.hh"
#include "Game.hh"
#include "SignalBuffer.hh"
//#include <sigc++/signal_system.h>
#include <slist.h>

#define GREAT_GUESS_MAX_LOCATIONS 32

using namespace SigC;

/** 
 * Minehunter
 * This constructs, owns and coordinates all the other objects
 * used by the computer player.
 * 
 */

class Minehunter : public Threads::Thread, public SigC::Object {
private:
    /// no copy
    Minehunter(const Minehunter& mh) {};
    /// no assignment
    Minehunter& operator=(const Minehunter& mh) { return *this; };
public:
    Minehunter() : state_(NEEDS_SETUP),pings_pending_(0),game_board_(),
                   known_(),add_(),moves_(),prob_loc_(NULL) {};
    /// Destructor does nothing special
    virtual ~Minehunter() {};

    /// Thread hook, runs the main event/computation loop.
    /// This function does not "own" and will not delete the signal_buffer.
    /// This function will add a reference to the signal_buffer.
    /// @parm signalbuffer must derived from BaseSignalBuffer* 
    /// @return is always NULL for now
    /// THIS FUNCTION ENDS WITH A "delete this" call !
    virtual void* main(void* signal_buffer);

    /// This reports the state of execution
    enum MHState 
    {
	NEEDS_SETUP,    ///< After default constructor or GameOver
	NEEDS_SCANSET,  ///< When nothing else to process
	PROCESSING,     ///< When still processing
	NEEDS_GAMEOVER, ///< When solved
        CANCELED        ///< After destoy is called
    };

    /// Slot functions
    /// These should only be called from within the thread of the
    /// Minehunter::main event loop, except for the destroy()
    /// slot, which can be called from any thread (I hope)
    //@{ 
    /// This is called to initialize and start a new game
    void setup(guint32 x_max,guint32 y_max,guint32 mines);
    /// Receive ownership of a new ScanSet (must later delete)
    void add_scanset(ScanSet * newset);
    /// Mark a location as empty
    void mark_empty(guint32 loc);
    /// Mark a location as a mine
    void mark_mine(guint32 loc);
    /// If new_ping is true, sendPing.emit(false)
    /// If new_ping is false, proceed if a guess is still needed
    void receive_ping(bool new_ping);
    /// Stop the game and wait for setup
    void game_over(GameEnd ending);
    /// Exit the main loop and end the thread. For multithreaded
    /// reasons, need an is_canceled, since this thread might clobber
    /// the state_ variable if another thread calls destroy.
    void destroy() { state_=CANCELED; is_canceled=true; };
    //@}

    /// Signals
    //@{
    /// Claim this location is empty
    Signal1<void, guint32> markEmpty;
    /// Claim this location is a mine
    Signal1<void, guint32> markMine;
    /// Call with true to souce ping, false to reply
    Signal1<void,bool> sendPing;
    /// The recipient must delete the passed pointer.
    /// (not sent if not connected to prevent memory leak)
    Signal1<void,MapProbLoc_t*> guess;
    /// In case of error, stop the game
    Signal0<void> stopGame;
    /// The allows messages to the user to be sent
    Signal1<void,string> publishText;
    //@}

protected:
    // Data members
    bool is_canceled;
    MHState state_;
    int pings_pending_;
    GameBoardType game_board_;
    KnownType known_;
    AddType add_;
    MovesType moves_;

    /// These functions make up the GreatGuess algorithm
    //@{
    void clean_up();
    bool initialize(string& out_report);
    bool call_op(string &out_report);
    void op();
    void calc_prob(string& out_report);
    void fake_guess(string& out_report);
    //@}

    // sub-routines for advancing the game
    // Called when there must be a random guess
    void guess_it();
    // Called when there are sets to add
    void add_it();
    // Called when there are squares which have been decided
    void move_it();

    // Helpers
    void queue_set(const ScanSet& S);
    void process();
public:
    /********************************************
     *  The rest of the class is for the        *
     *  GreatGuess routine.  It needs so        *
     *  much of Minehunter, just include it     *
     *  wholesale.                              *
     ********************************************/

    /// For the GreatGuess routine
    /// game_board_.measured() must be < maxloc.
    static const unsigned long maxloc=GREAT_GUESS_MAX_LOCATIONS;

protected:
    // The buffer we get messages from
    SafeSignalBuffer* sb_;

    /// A list of indices in k_list_. 
    typedef slist<guint32> K_List_Type;
    // Forward iterating through a slist is very fast
    typedef K_List_Type::const_iterator K_Iterator;

   // Where to put the output probabilities
    MapProbLoc_t *prob_loc_;

 
    /// This is the total # of mines in the measured m_max_ locations
    unsigned long measured_mines_;
    /// This is min and max measured_mines_ that work
    unsigned long min_mm_,max_mm_;
    /// This is total # of mines unlocated
    unsigned long total_mines_;
    /// This is the total # of unmeasured locations
    unsigned long num_unmeasured_;

    /// Use 0..(m_max_) entries in m_known_[] and loc_[]
    unsigned long m_max_;
    /// Set to 1 if m has a mine in the current layout.
    guint32 m_mined_[maxloc];
    /// For index m, m_known_[m] is a list of indices k in k_mines_
    K_List_Type m_known_[maxloc];

    /// For mm measured mines mm_ways_[mm] is the # of layouts
    unsigned long mm_ways_[maxloc];
    /// For mm and m, mm_m_ways_[mm][m] is the # of layets with m mined
    unsigned long mm_m_ways_[maxloc][maxloc];

    /// For index m, m_loc_[m] is a location on the game_board_
    guint32 m_loc_[maxloc];
    /// For location loc, loc_m_[loc] is an index m and m_loc_[m]==loc;
    // guint32 loc_m_[1024];
    map<guint32,guint32> loc_m_;

    /// Use 0..(k_max-1) entries in k_mines_
    guint32 k_max_;
    /// For index k, k_mines_[k] is the # of mines in a known ScanSet
    vector<guint32> k_mines_;
    // guint32 k_mines_[1024];
    /// For index k, k_tot_[k] is the known # of mines in the ScanSet
    vector<guint32> k_tot_;
    // guint32 k_tot_[1024];

    /// Pre-allocate some iterators, and sundry variables
    /// These are all used in the op() member function
    /// m_ is the recursion depth
    /// make static for better compiler / run time
    //@{
     K_Iterator k1,k2,kend;
     guint32 k_,m_,m_temp;
     gint32 result_;
    //@}
};

#undef GREAT_GUESS_MAX_LOCATIONS

#endif

/*
$Header: /home/ckuklewicz/cvsroot/gminehunter/Minehunter.hh,v 2.9 2000/08/10 04:32:51 ckuklewicz Exp $
$Log: Minehunter.hh,v $
Revision 2.9  2000/08/10 04:32:51  ckuklewicz
Reworked the locking in SafeSignalBuffer, removing busy_calling_
(but leaving it defined for now)

Added G_SignalBuffer_EventSource, works great

This will be the tarball version

Revision 2.8  2000/08/07 03:22:33  ckuklewicz
Wow...tons and tons of bugs fixed.
Added the sendPing, received_ping functions.
The COMPUTER_GAME seems to actually work.

Revision 2.7  2000/08/02 22:00:30  ckuklewicz
Try and add code to hook up App/Win to Minehunter
Who knows if it will compile

Revision 2.6  2000/08/02 20:40:05  ckuklewicz
Changed Buffers to allow emitOneEvent.
Moved GreatGuess code fully into Minehunter.cc
and removed files (to old/)
Now compiles with optMakefile, links with libcln.a

Revision 2.5  2000/08/02 13:58:32  ckuklewicz
Add WrapPointer.  Merge GreatGuess into Minehunter

Revision 2.4  2000/07/31 04:22:15  ckuklewicz
Everything compiles and links.  Happy Day
Single user still works.
Need to implement guess_it() from java
The 2^29 limit in cln may need examining

Revision 2.3  2000/07/30 23:23:06  ckuklewicz
Mostly cleaned up!

Revision 2.2  2000/07/30 05:54:08  ckuklewicz
CLeaninG UP minHunteR cOde

Revision 2.1  2000/07/22 17:35:34  ckuklewicz
Added

*/
