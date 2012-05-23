#ifndef INCL_GAMEDATA_H
#define INCL_GAMEDATA_H

#include "mh.hh"
#include <vector>      // Linear array of Locations
#include <set>
#include "Location.hh"


/**
 * The GameData holds the object used by the GUI to track the game
 * state.  It is also the data model for the visible canvas items.
 * No real logic is stored here.  In particular it does not know
 * what type of game is going on.
 */
class GameData {
private:
    typedef vector<Location> BoardType;  // kept in linear array
    BoardType board_;
    GameType game_type_;
    guint32 x_max_, y_max_,locations_,mines_,seed_;
    guint32 measured_,selected_;
    set<guint32> comp_marked_;
    GameData(const GameData& gd) { };
    GameData& operator=(const GameData& gd) { return *this; };
public:
    /// This is largest height and width allowed
    static const guint32 xy_max_(1024);
    /// This prepares the data structures for a new game
    GameData(GameType type, guint32 x_max, guint32 y_max, guint32 mines, unsigned int seed);
    virtual GameData::~GameData();
    /// This recycles the data structures to the initial state for a new game
    void reset_data(GameType type, guint32 x_max, guint32 y_max, guint32 mines, unsigned int seed);
    
    /** Return the game parameters */
    //@{
    GameType get_type() const { return game_type_; };
    void get_sizes(guint32 *x_max, guint32 *y_max) const
	{ *x_max=x_max_; *y_max=y_max_; };
    guint32 get_locations() const {return locations_; };
    guint32 get_mines() const {return mines_; };
    guint32 get_seed() const {return seed_; };
    //@}

    /** Translation functions */
    //@{
    void loc_to_xy(guint32 location, guint32 * x,guint32 * y) const {
	*x=(location % x_max_); 	*y=(location / x_max_);
	*x=CLAMP(*x,0,x_max_-1);	*y=CLAMP(*y,0,y_max_-1);
    };
    guint32 xy_to_loc(guint32 x,guint32 y) const {
	x=CLAMP(x,0,x_max_-1);	y=CLAMP(y,0,y_max_-1);
	return (y*x_max_)+x;
    };
    //@}


    /** Access to Locations */
    //@{
    Location& get_location(guint32 x, guint32 y) {return board_[xy_to_loc(x,y)];};
    const Location& get_const_location(guint32 x, guint32 y) const {return board_[xy_to_loc(x,y)];};
    Location& get_location(guint32 loc) {return board_[CLAMP(loc,0,locations_-1)];};
    const Location& get_const_location(guint32 loc) const {return board_[CLAMP(loc,0,locations_-1)];};
    //@}
    
    /**
     * The calc_adjacent function handles setting the
     * Location::measured fields to true.  And sets the
     * Location::adjacentMines field.
     */
    ScanSet* calc_adjacent(guint32 x, guint32 y, guint32 *adj);
    
    bool computer_marked(guint32 loc,bool is_guess);

    bool complete();

    bool get_first_unmeasured(guint32 *x, guint32 *y);

protected:
    void lay_mines();
};


#endif


/*
$Header: /home/ckuklewicz/cvsroot/gminehunter/GameData.hh,v 2.10 2000/08/11 04:56:03 ckuklewicz Exp $
$Log: GameData.hh,v $
Revision 2.10  2000/08/11 04:56:03  ckuklewicz
Worked out a few user interface policy bugs.
Made G_SignalBuffer_EventSource more like a swiss army knife.
(Can look like an event source, a timeout, or an idler)
Need to make timeout_msecs==-1 special.

The seed=0 randomly selected seed is reported in the text box.

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

Revision 2.6  2000/07/28 05:15:10  ckuklewicz
Single Player Works

Revision 2.5  2000/07/25 05:35:41  ckuklewicz
still not playable

Revision 2.4  2000/07/24 03:57:04  ckuklewicz
Compile cleanly under -Wall

Revision 2.3  2000/07/24 02:14:27  ckuklewicz
Split all into hh and cc

Revision 2.2  2000/07/23 02:38:35  ckuklewicz
The canvas is under my control

Revision 2.1  2000/07/22 17:35:34  ckuklewicz
Added

*/
