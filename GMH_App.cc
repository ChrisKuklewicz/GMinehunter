// Adapted from GabberApp.cc (now called GMH_App)

/* do not use gnome configuration management yet */
#define PACKAGE "gminehunter"
#define VERSION "0.1.0"
#define GLADE "gminehunter.glade"

#include "WrapSlot.cc"
#include "WrapPointer.cc"
#include "GMH_Win.hh"
#include "GMH_App.hh"
#include <strstream.h>
#include <sigc++/retbind.h>
#include "Minehunter.hh"

using namespace SigC;
using namespace WrapSlot;

/** 
 * The constructor create all the needed objects, but not start
 * the application. It pulls information from #defines
 */
GMH_App::GMH_App(int argc, char** argv)
    : _GnomeMain(PACKAGE, VERSION, argc, argv),
      sourcefile_(GLADE) // TODO : make a full path
{
    cout << PACKAGE << " " << VERSION << endl;
	
    /* No window icon 
       // Set the default window icon
       #ifdef GABBER_WINICON
       string window_icon = _pix_path + "gnome-gabber.xpm";
       gnome_window_icon_set_default_from_file(window_icon.c_str());
       gnome_window_icon_init();
       #endif
    */

    glade_gnome_init(); // From /usr/include/glade/glade.h

    // Locate UI/Glade file
    if (!g_file_exists(sourcefile_.c_str()))
        sourcefile_ = "./" GLADE;
    if (!g_file_exists(sourcefile_.c_str()))
        sourcefile_ = "./src/" GLADE;
    if (!g_file_exists(sourcefile_.c_str()))
    {
        g_error("Could not find gabber.glade in ./src or ./ !");
        exit(-1);
    }

    // Create dynamic member objects
    window_  = new GMH_Win(sourcefile_.c_str());  // delete in ~GMH_App
    mine_hunter_ = new Minehunter();
    to_worker_ = new SafeSignalBuffer(true,recursive_mutex);
    from_worker_ = new SafeSignalBuffer(true,recursive_mutex);
    data_ = NULL;

    // Connect this App to itself
    gameOver.connect(slot(this,&GMH_App::game_over));

    // Connect window_ to this App
    window_->newGame.connect(slot(this,&new_game));
    window_->stopGame.connect(slot(this,&stop_game));
    window_->userClick.connect(slot(this,&user_click));

    // Connect this App to window_
    gameOver.connect(slot(window_,&GMH_Win::game_over));

    // Connect this App to mine_hunter_
    gameOver.connect(wrap_slot(
        slot(mine_hunter_,&Minehunter::game_over),to_worker_));
    setupMinehunter.connect(wrap_slot(
        slot(mine_hunter_,&Minehunter::setup),to_worker_));
    addScanset.connect(wrap_pointer_slot(  // pointer
        slot(mine_hunter_,&Minehunter::add_scanset),to_worker_));
    markEmpty.connect(wrap_slot(
        slot(mine_hunter_,&Minehunter::mark_empty),to_worker_));
    markMine.connect(wrap_slot(
        slot(mine_hunter_,&Minehunter::mark_mine),to_worker_));
    sendPing.connect(wrap_slot(
        slot(mine_hunter_,&Minehunter::receive_ping),to_worker_));
    
    // Connect mine_hunter_ to this App
    mine_hunter_->markEmpty.connect(wrap_slot(
        slot(this,&mark_empty),from_worker_));
    mine_hunter_->markMine.connect(wrap_slot(
        slot(this,&mark_mine),from_worker_));
    mine_hunter_->guess.connect(wrap_pointer_slot(  // pointer
        slot(this,&guess),from_worker_));
    mine_hunter_->stopGame.connect(wrap_slot(
        slot(this,&stop_game),from_worker_));
    mine_hunter_->sendPing.connect(wrap_slot(
        slot(this,&receive_ping),from_worker_));
    
    // Connect mine_hunter_ to window_
    mine_hunter_->publishText.connect(wrap_slot(
        slot(window_,&GMH_Win::append_text),from_worker_));

    // Connect Gtk idle event handler to callEvents from mine_hunter_
    // _GnomeMain.idle.connect(retbind<gint>(from_worker_->callEvents.slot(),1));
    // THE IDLE CALLBACK CONSUMES TOO MUCH CPU TIME, USE TIMEOUT BELOW INSTEAD

    // Connect Gtk timeout to callEvents every 1/10 of a second
    // _GnomeMain.timeout.connect(retbind<gint>(from_worker_->callEvents.slot(),1),15);
    
    // Instead of the IDLE or TIMEOUT, customize behavior:
    source_from_worker_ = new G_SignalBuffer_EventSource(from_worker_); 
    source_from_worker_->call_one_event=true;  // one event at time
    // Start mine_hunter_ background thread, which has its own event loop
    mine_hunter_->start(to_worker_);
}

GMH_App::~GMH_App()
{
    if (mine_hunter_)
    {
        // cannot use to_worker_ buffer to relay this message
        mine_hunter_->destroy();
    }
    // Kill source_from_worker_ before from_worker_
    delete source_from_worker_;
    source_from_worker_=NULL;
    delete from_worker_;
    from_worker_=NULL;
    // Kill window_ before data_
    delete window_; // new in GMH_App
    window_=NULL;
    delete data_;  // new in new_game
    data_=NULL;
    // Last to go....
    delete to_worker_;
    to_worker_=NULL;
}

void GMH_App::run()
{
    // Let the gnome app start processing messages
    gdk_threads_enter();
    Gnome::Main::run();
    gdk_threads_leave();
}

void GMH_App::quit()
{
    // Stop gtk/gnome message loop..returns to main.cc
    Gnome::Main::quit();
}

void GMH_App::new_game(GameType game_type,guint32 x_max,guint32 y_max,guint32 mines,guint32 seed) 
{
    _show_args5((int)game_type,x_max,y_max,mines,seed);
    _assert(x_max<=GameData::xy_max_);
    _assert(y_max<=GameData::xy_max_);
    _assert(mines<=(x_max*y_max));
    if (NULL!=data_) {
        // DEBUG cout << "Resetting old data object" << endl;
        // data_->reset_data(game_type,x_max,y_max,mines,seed);
        delete data_;
        data_=NULL;
    }
    // DEBUG cout << "Creating new data object" << endl;
    _nullChk(data_=new GameData(game_type,x_max,y_max,mines,seed));  // delete in ~GMH_App, new_game
    window_->set_gamedata(data_); // must do this before mark_empty

    if (0==seed)
    {
        ostrstream msg;
        msg << "The seed chosen was " << data_->get_seed() << endl << ends;
        window_->append_text(msg.str());
    }

    // Perhaps this will cause the drawing commands to complete
    while (gtk_events_pending())
    {
        gtk_main_iteration_do(false);
    }

    if (HUMAN_GAME!=game_type)
    {
        setupMinehunter.emit(x_max,y_max,mines);
        if (COMPUTER_GAME==game_type)
        {
            mark_empty(0);  // pretend computer selected corner
        }
    }
    return;
}

void GMH_App::game_over(GameEnd result)
{
    switch(result)
    {
        case CANCEL_GAME:
            window_->append_text("\nThe Game was CANCELED!\n");
            break;
        case LOST_GAME:
            // cout << "LOST_GAME" << endl;
            window_->append_text("\nThe Game was LOST!\n");
            break;
        case WON_GAME:
            // cout << "WON_GAME" << endl;
            window_->append_text("\nThe Game was WON!\n");
            break;
        default:
            _never_get_here;
    }
}

void GMH_App::receive_ping(bool new_ping) 
{ 
    _show_args1(new_ping); 
    if (new_ping) 
    { 
        sendPing.emit(false); 
    } 
}

void GMH_App::stop_game() 
{
    _show_args0();
    gameOver.emit(CANCEL_GAME);
}

void GMH_App::select_adjacent(guint32 x, guint32 y) 
{
    _show_args2(x,y);
    guint xa,ya;
    data_->get_sizes(&xa,&ya);
    guint x1=((x>0)?x-1:0);
    guint y1=((y>0)?y-1:0);
    guint x2=((x>=xa-1)?xa:x+2);
    guint y2=((y>=ya-1)?ya:y+2);
    // DEBUG cout << "select_adjacent" << x1 << y1 << " " << x2 << y2 << endl;
    for (xa=x1; xa<x2; ++xa)
        for (ya=y1; ya<y2; ++ya)
            if ((xa!=x) || (ya!=y)) 
                if (!(data_->get_location(xa,ya).selected)) 
                    select_loc(xa,ya);
    return;
}

/// Called when user selects a location
/// Also called if the computer selected the location
void GMH_App::select_loc(guint32 x, guint32 y) 
{
    _show_args2(x,y);
    _nullChk(data_);
    // DEBUG cout << "select_loc" << x << y << endl;
    Location& loc = data_->get_location(x,y);
    ScanSet *new_set;
    guint adj = 0;
    if (loc.selected)
        return;
    if (Location::FLAGGED==loc.user)
        return;
    if (Location::MINED==loc.help)
        return;
    if (!loc.hasMine) 
    {
        loc.user=Location::NOTHING;
        _assert(Location::MINED!=loc.help);  // computer never makes mistakes
        if ((Location::EMPTY!=loc.help) &&
            (Location::MAYBE_EMPTY!=loc.help)) 
        {
            loc.help=Location::NONE;
        }
        new_set=data_->calc_adjacent(x,y,&adj);
        window_->update_display(x,y);
        // Tell other thread, this is the only call to markEmpty.emit()
        markEmpty.emit(data_->xy_to_loc(x,y));
        addScanset.emit(new_set);

        if ((0==adj) && (COMPUTER_GAME!=data_->get_type()))
        {
            select_adjacent(x,y);  // which calls select_loc...recursion
        }
    } 
    else
    {
        loc.selected=true;  // Means we selected a mine and died
        window_->update_display(x,y);
        gameOver.emit(LOST_GAME);
    }

    ////////////////////////
    if (data_->complete()) {
        gameOver.emit(WON_GAME);
    }
    ////////////////////////
}

void GMH_App::mark_loc(guint32 x, guint32 y) 
{
    _show_args2(x,y);
    _nullChk(data_);
    // DEBUG cout << "mark_loc" << x << y << endl;
    Location& loc = data_->get_location(x,y);
    if (loc.selected)
        return;
    if ( (COMPUTER_GAME!=data_->get_type()) && (Location::FLAGGED==loc.user) )
    {
        loc.user=Location::NOTHING;  // Only user can toggle off
    }
    else
    {
        loc.user=Location::FLAGGED;
    }
    window_->update_display(x,y);
    ////////////////////////
    if (data_->complete()) {
        gameOver.emit(WON_GAME);
    }
    ////////////////////////
}

void GMH_App::user_click(guint32 x, guint32 y, guint32 button)
{
    _show_args3(x,y,button);
    if (COMPUTER_GAME!=data_->get_type())
    {
        switch (button) 
        {
            case 1:
                select_loc(x,y);
                break;
            case 2:
                // do nothing
                break;
            case 3:
                mark_loc(x,y);  // toggle flag
                break;
            default:
                // do nothing (Scroll wheel, etc...)
                break;
        }
    }
    return;
}


// Computer is certain
void GMH_App::mark_empty(guint32 loc)
{
    _show_args1(loc);
    guint32 x,y;
    data_->loc_to_xy(loc,&x,&y);
    Location &location=data_->get_location(loc);
    if (COMPUTER_GAME==data_->get_type())
    {
        location.help=Location::EMPTY;
        location.user=Location::NOTHING; // unmark any flag
        select_loc(x,y);  // This tells Minehunter immediately
    }
    else
    {
        // ASSISTED_GAME does not tell Minehunter immediately
        location.help=Location::EMPTY;
        location.user=Location::NOTHING; // unmark any flag
        window_->update_display(x,y);
    }
    // Perhaps this will cause the drawing commands to complete
    while (gtk_events_pending())
    {
        gtk_main_iteration_do(false);
    }
}

// Computer is certain
void GMH_App::mark_mine(guint32 loc)
{
    _show_args1(loc);
    guint32 x,y;
    data_->loc_to_xy(loc,&x,&y);
    if (!data_->computer_marked(loc,false))
    {
        cerr << "Computer was certain of mine, but was wrong!" << endl;
        cerr << "This should never happen" << endl;
        _assert(false);
        gameOver.emit(LOST_GAME);
    }
    else
    {
        if (COMPUTER_GAME==data_->get_type())
        {
            // go ahead and add a flag
            mark_loc(x,y); 
        }
        else
        {
            // Just let user know
            window_->update_display(x,y);
        }
        // Tell Minehunter thread immediately in both cases
        // (also can be emitted in mark_guess function)
        markMine.emit(data_->xy_to_loc(x,y));
    }
    // Perhaps this will cause the drawing commands to complete
    while (gtk_events_pending())
    {
        gtk_main_iteration_do(false);
    }
}

void GMH_App::mark_guess(guint32 x, guint32 y, bool empty)
{
    _show_args3(x,y,empty);
    static guint32 prev_x=1000000, prev_y=1000000;  // this # means none
    Location& loc=data_->get_location(x,y);
    if (empty)
    {
        if (ASSISTED_GAME==data_->get_type())
        {
            if (prev_x!=1000000)
            {
                // erase previous guess marker
                Location& prev_loc=data_->get_location(prev_x,prev_y);
                if (Location::MAYBE_EMPTY==prev_loc.help)
                {
                    prev_loc.help=Location::NONE;
                    window_->update_display(prev_x,prev_y);
                }
            }
            prev_x=x;
            prev_y=y;
        }
        loc.help=Location::MAYBE_EMPTY;
        if (COMPUTER_GAME==data_->get_type())
        {
            loc.user=Location::NOTHING;
            select_loc(x,y);  // This will tell Minehunter (or gameOver)
        } 
        else
        {
            // Do NOT tell Minehunter, only user about empty guess
            window_->update_display(x,y);
        }
    }
    else
    {
        if (!data_->computer_marked(data_->xy_to_loc(x,y),true))
        {
            gameOver.emit(LOST_GAME);
        }
        else
        {
            if (COMPUTER_GAME==data_->get_type())
            {
                // Do NOT put a flag on a guess
                // Tell Minehunter immediately
                // (also emitted in mark_mine function)
                markMine.emit(data_->xy_to_loc(x,y));
            }
            else
            {
                // Do NOT tell Minehunter, only user about mine guess
                window_->update_display(x,y);
            }
        }
    }
}

void GMH_App::guess(MapProbLoc_t *prob_loc)
{
    _show_args0();
    _nullChk(prob_loc);
    guint32 x,y;
    ostrstream report;
    const MapProbLoc_t::iterator& low=prob_loc->begin();
    MapProbLoc_t::reverse_iterator high(prob_loc->rbegin());

    // Look for "unmeasured" location as low or high value
    if (low->second >= data_->get_locations())
    {
        report << "Lowest prob is for unmeasured locations."  << endl;
        //        bool found=
        if (data_->get_first_unmeasured(&x,&y))
        {
            low->second=data_->xy_to_loc(x,y);
        }
        else
        {
            _never_get_here;
        }
    }
    if (high->second >= data_->get_locations())
    {
        report << "Highest prob is for unmeasured locations."  << endl;
        //bool found=
        if (data_->get_first_unmeasured(&x,&y))
        {
            high->second=data_->xy_to_loc(x,y);
        }
        else
        {
            _never_get_here;
        }
    }            

    // Describe Low and High probs
    data_->loc_to_xy(low->second,&x,&y);
    report << "Lowest prob " << setprecision(7) << low->first << " at ( " 
           << x << " , " << y << " )   ";
    data_->loc_to_xy(high->second,&x,&y);
    report << "Highest prob " << setprecision(7) << high->first << " at ( " 
           << x << " , " << y << " )" << endl;

    // Pick whichever is more accurate (with slight bias toward low)
    if ((low->first >= 1.001 - high->first) && (ASSISTED_GAME==data_->get_type()))
    {
        data_->loc_to_xy(high->second,&x,&y);  // set x and y
        report << "Guessing highest is a mine" << endl << ends;
        window_->append_text(report.str());
        if (1.0<=high->first)
        {
            // not guessing, knowing...
            mark_mine(high->second);
        }
        else
        {
            mark_guess(x,y,false);
        }
    }

    data_->loc_to_xy(low->second,&x,&y);  // set x and y
    report << "Guessing Lowest is empty" << endl << ends;
    window_->append_text(report.str());
    if (1e-6>=low->first)
    {
        // not guessing, knowing...
        mark_empty(low->second);
    }
    else
    {
        mark_guess(x,y,true);
    }
    // The reciever (this function) must delete the passed pointer
    delete prob_loc;
    prob_loc=NULL;
}

/*
  $Header: /home/ckuklewicz/cvsroot/gminehunter/GMH_App.cc,v 2.10 2000/08/20 00:23:42 ckuklewicz Exp $
  $Log: GMH_App.cc,v $
  Revision 2.10  2000/08/20 00:23:42  ckuklewicz
  Numerous bugs,races,lockups, gui logic all fixed
  It works great.  I love -O3

  Revision 2.9  2000/08/11 13:45:39  ckuklewicz
  Changed version number to 0.0.2

  Revision 2.8  2000/08/11 04:56:03  ckuklewicz
  Worked out a few user interface policy bugs.
  Made G_SignalBuffer_EventSource more like a swiss army knife.
  (Can look like an event source, a timeout, or an idler)
  Need to make timeout_msecs==-1 special.

  The seed=0 randomly selected seed is reported in the text box.

  Revision 2.7  2000/08/10 04:32:51  ckuklewicz
  Reworked the locking in SafeSignalBuffer, removing busy_calling_
  (but leaving it defined for now)

  Added G_SignalBuffer_EventSource, works great

  This will be the tarball version

  Revision 2.6  2000/08/07 03:22:33  ckuklewicz
  Wow...tons and tons of bugs fixed.
  Added the sendPing, received_ping functions.
  The COMPUTER_GAME seems to actually work.

  Revision 2.5  2000/08/02 22:00:30  ckuklewicz
  Try and add code to hook up App/Win to Minehunter
  Who knows if it will compile

  Revision 2.4  2000/07/28 05:15:09  ckuklewicz
  Single Player Works

  Revision 2.3  2000/07/27 03:44:05  ckuklewicz
  Cleaned up signal buffers with BaseSignalBuffer, added DataBuffer

  Revision 2.2  2000/07/25 05:35:41  ckuklewicz
  still not playable

  Revision 2.1  2000/07/23 02:38:35  ckuklewicz
  The canvas is under my control

  Revision 2.0  2000/07/22 17:21:14  ckuklewicz
  Synchonizing release numbers

*/
