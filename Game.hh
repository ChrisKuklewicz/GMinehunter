#ifndef INCL_GAME_H
#define INCL_GAME_H
#include "mh.hh"  // TODO : Capitalize this filename

/**
 * Game keeps track of what the Computer player knows about the board.
 * Unlike the console version, which had to be like GameData, the
 * Game class needs to only keep track of the board paramters,
 * and solved empty and mined locations.
 */
class Game
{
    // XXX private:
public:
    guint32 x_max_,y_max_,locations_,mines_,remaining_;
    ScanSet empty_, mined_, measured_;
    /// disabled
    Game(const Game& game) {};
    /// disabled
    Game& operator=(const Game& game) { return *this; };
public:
    Game() {};
    virtual ~Game() {};
    
    /// This re-initializes the Game object
    void reset(guint32 x_max, guint32 y_max, guint32 mines); 

    /// The usual query methods
    //@{
    guint32 x_max() const { return x_max_; };
    guint32 y_max() const { return y_max_; };
    guint32 locations() const { return locations_; };
    guint32 mines() const { return mines_; };
    guint32 empty() const { return empty_.size(); };
    guint32 mined() const { return mined_.size(); };
    guint32 measured() const { return measured_.size(); };
    guint32 unmeasured() const { return locations_-empty_.size()-mined_.size()-measured_.size(); };
    //@}

    /// The interface for adding to the solved part of the board
    /// They return true if the location is new, false if already
    /// known 
    //@{
    bool is_marked(index_type loc);
    bool is_measured(index_type loc);
    bool mark_empty(index_type loc);
    bool mark_mine(index_type loc);
    bool mark_measured(const ScanSet& S);
    //@}

    /// The removes known empty and mined locations from a
    /// new ScanSet passed from the GUI thread.
    ScanSet trim(const ScanSet& input) const
    {
        return ((input-empty_)-mined_);
    };
    /// This returns true when the game is won.
    bool complete() const { return (0==remaining_); };
};



#endif



