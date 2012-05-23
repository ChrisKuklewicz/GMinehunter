#ifndef INCL_GMH_APP_HH
#define INCL_GMH_APP_HH

// Adapted from GabberApp.hh

#include "Minehunter.hh"
#include "SignalBuffer.hh"


class GMH_Win;
class GameData;

using namespace SigC;

/**
 * GMH_App 
 *
 * This is the practical entry point to the program, the main
 * function simply makes one of this class and calls run(). 
 */

class GMH_App : public SigC::Object {
public:
    GMH_App(int argc, char** argv);
    virtual ~GMH_App();
public:
    /// run is called from the main entrypoint, it calls Gnome::Main::run()
    void run();
    /// Currently just calls Gnome::Main.quit();
    void quit();
    /// Signals and Slots for talking to GMH_Win
    //@{
    void new_game(GameType game_type,guint32 x_max,guint32 y_max,guint32 mines, guint32 seed);
    void stop_game();
    void game_over(GameEnd result);
    void user_click(guint32 x, guint32 y, guint32 button);
    void receive_ping(bool new_ping);
    /// send a new ping (with true) or reply to a ping (with false)
    Signal1<void,bool> sendPing;
    /** Signal to indicate the current game is no longer being played.
     * @param result is a GameEnd enum (won or lost or cancelled)
     */
    Signal1<void,GameEnd> gameOver;
    //@}
protected:
    /// This loads the glade XML and implements the logic of the window
    GMH_Win * window_;
    GameData * data_;
private:
    /** 
     * This is built in GMH_App and Gnome::Main::run is called from run.
     * This is a singleton, and all public methods are static.
     */
    Gnome::Main        _GnomeMain;
    /// _SourceFile is the name of the .glade file to load
    string             sourcefile_;

    // Internal functions
    void select_loc(guint32 x, guint32 y);
    void select_adjacent(guint32 x, guint32 y);
    void mark_loc(guint32 x, guint32 y);
    void mark_guess(guint32 x, guint32 y, bool empty);

    /// Sent to Minehunter
    /// x max, y max, and mines
    Signal3<void,guint32,guint32,guint32> setupMinehunter;
    Signal1<void,ScanSet *> addScanset;
    Signal1<void,guint32> markEmpty;
    Signal1<void,guint32> markMine;
    /// Receive from Minehunter
    void mark_empty(guint32 loc);
    void mark_mine(guint32 loc);
    void guess(MapProbLoc_t *prob_loc);
    
    Minehunter *mine_hunter_;
    SafeSignalBuffer *to_worker_,*from_worker_;
    G_SignalBuffer_EventSource *source_from_worker_;
};

#endif

/*
  $Header: /home/ckuklewicz/cvsroot/gminehunter/GMH_App.hh,v 2.7 2000/08/11 04:56:03 ckuklewicz Exp $
  $Log: GMH_App.hh,v $
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

  Revision 2.5  2000/08/07 03:22:33  ckuklewicz
  Wow...tons and tons of bugs fixed.
  Added the sendPing, received_ping functions.
  The COMPUTER_GAME seems to actually work.

  Revision 2.4  2000/08/02 22:00:30  ckuklewicz
  Try and add code to hook up App/Win to Minehunter
  Who knows if it will compile

  Revision 2.3  2000/07/28 05:15:09  ckuklewicz
  Single Player Works

  Revision 2.2  2000/07/25 05:35:41  ckuklewicz
  still not playable

  Revision 2.1  2000/07/23 02:38:35  ckuklewicz
  The canvas is under my control

  Revision 2.0  2000/07/22 17:21:14  ckuklewicz
  Synchonizing release numbers

*/
