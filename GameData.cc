#include "GameData.hh"
#include <functional>  // Random routines to lay mines
#include <algorithm>  // for_each

GameData::GameData(GameType type, guint32 x_max, guint32 y_max, guint32 mines, unsigned int seed) : board_()
{
    if ( (x_max<1) || (y_max<1) || (x_max>xy_max_) || (y_max>xy_max_) ) 
    {
        cerr << endl << "That board is too big" << endl;
        exit(9);
    }
    game_type_=type;
    x_max_=x_max;
    y_max_=y_max;
    locations_=x_max_*y_max_;
    mines_=CLAMP(mines,1,locations_-1);
    selected_=0;
    measured_=0;
    board_.resize(locations_);
    seed_=seed;
    // Call member functions to randomly place mines
    lay_mines();
}

GameData::~GameData() { /* C++ does the right thing ?... */ 
    // DEBUG cout << "~GameData" << endl; cout.flush();
}

void GameData::reset_data(GameType type, guint32 x_max, guint32 y_max, guint32 mines, unsigned int seed)  
{
    if ( (x_max<1) || (y_max<1) || (x_max>xy_max_) || (y_max>xy_max_) ) 
    {
        cerr << endl << "That board is too big" << endl;
        exit(9);
    }
    game_type_=type;
    x_max_=x_max;
    y_max_=y_max;
    locations_=x_max_*y_max_;
    mines_=CLAMP(mines,1,locations_-1);
    // The next line is difference from the constructor
    for_each(board_.begin(), board_.end(), mem_fun_ref(&Location::reset));
    board_.resize(locations_);
    // The next line is difference from the constructor
    for_each(board_.begin(), board_.end(), mem_fun_ref(&Location::reset));
    lay_mines();
    comp_marked_.clear();
}
    
/// Called from constructor. A zero seed picks one based on time of day.
/// This assumes the board_ has been constructed/reset to empty state
void GameData::lay_mines() 
{
    guint32 laid=0;
    subtractive_rng rng=subtractive_rng();
    if (0==seed_) 
    {
        GTimeVal time;
        g_get_current_time(&time);
        /* Not trying to be cryptographically secure... */
        seed_=time.tv_sec+time.tv_usec;
    }
    rng.initialize(seed_);
    while (laid < mines_) 
    {
        Location& loc=get_location(rng(locations_));
        if (!loc.hasMine) 
        {
            loc.set_mine();
            ++laid;
        }
    }
}

ScanSet* GameData::calc_adjacent(guint32 x, guint32 y, guint32 *adj) 
{
    _nullChk(adj);
    Location& thisloc=get_location(x,y);
    guint xa,ya,count=0;
    ScanSet * new_set = new ScanSet();
    if ((!thisloc.hasMine) && (!thisloc.selected))
    {
        guint x1=((x>0)?x-1:0);
        guint y1=((y>0)?y-1:0);
        guint x2=((x>=x_max_-1)?x_max_:x+2);
        guint y2=((y>=y_max_-1)?y_max_:y+2);
        for (xa=x1; xa<x2; ++xa)
            for (ya=y1; ya<y2; ++ya)
                if ((xa!=x) || (ya!=y)) 
                {
                    Location& otherloc = get_location(xa,ya);
                    new_set->add_index(xy_to_loc(xa,ya));
                    if (!otherloc.measured)
                    {
                        otherloc.measured=true;
                        ++measured_;
                    }
                    if (otherloc.hasMine)
                    {
                        ++count;
                        ++new_set->min_mines();
                        ++new_set->max_mines();
                    }
                }
        thisloc.adjacentMines=count;
        thisloc.measured=true;
        ++measured_;
        thisloc.selected=true;
        ++selected_;
    }
    (*adj)=count;
    return new_set;
}

bool GameData::computer_marked(guint32 loc, bool is_guess)
{
    _show_args2(loc,is_guess);
    if (comp_marked_.find(loc)==comp_marked_.end())
    {
        if (is_guess)
        {
            get_location(loc).help=Location::MAYBE_MINED;
        }
        else
        {
            get_location(loc).help=Location::MINED;
        }
        if (COMPUTER_GAME==game_type_)
        {
            if (get_location(loc).hasMine)
            {
                comp_marked_.insert(loc);
            }
            else
            {
                // computer made a bad guess, oops!
                return false;
            }
        }
    }
    return true;
}

bool GameData::complete() 
{ 
    if (COMPUTER_GAME==game_type_)
    {
        return (mines_==comp_marked_.size());
    }
    else
    {
        return (selected_+mines_==locations_); 
    }
}

bool GameData::get_first_unmeasured(guint32 *x, guint32 *y)
{
    _nullChk(x);
    _nullChk(y);
    guint32 xi,yi;
    for (yi=0; yi < y_max_; ++yi)
        for (xi=0; xi < x_max_; ++xi)
            if (!get_location(xi,yi).measured)
            {
                (*x)=xi;
                (*y)=yi;
                return true;
            }
    return false;
}


/*
$Header: /home/ckuklewicz/cvsroot/gminehunter/GameData.cc,v 2.9 2000/08/11 04:56:03 ckuklewicz Exp $
$Log: GameData.cc,v $
Revision 2.9  2000/08/11 04:56:03  ckuklewicz
Worked out a few user interface policy bugs.
Made G_SignalBuffer_EventSource more like a swiss army knife.
(Can look like an event source, a timeout, or an idler)
Need to make timeout_msecs==-1 special.

The seed=0 randomly selected seed is reported in the text box.

Revision 2.8  2000/08/10 04:32:51  ckuklewicz
Reworked the locking in SafeSignalBuffer, removing busy_calling_
(but leaving it defined for now)

Added G_SignalBuffer_EventSource, works great

This will be the tarball version

Revision 2.7  2000/08/07 03:22:33  ckuklewicz
Wow...tons and tons of bugs fixed.
Added the sendPing, received_ping functions.
The COMPUTER_GAME seems to actually work.

Revision 2.6  2000/08/02 22:00:30  ckuklewicz
Try and add code to hook up App/Win to Minehunter
Who knows if it will compile

Revision 2.5  2000/08/02 13:58:32  ckuklewicz
Add WrapPointer.  Merge GreatGuess into Minehunter

Revision 2.4  2000/07/28 05:15:10  ckuklewicz
Single Player Works

Revision 2.3  2000/07/25 05:35:41  ckuklewicz
still not playable

Revision 2.2  2000/07/24 03:57:04  ckuklewicz
Compile cleanly under -Wall

Revision 2.1  2000/07/24 02:14:27  ckuklewicz
Split all into hh and cc

*/
