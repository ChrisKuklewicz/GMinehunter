#ifndef INCL_LOCATION_H
#define INCL_LOCATION_H

// Reduce the header dependencies:

#include <glib.h>
#include <gnome--/canvas-item.h>
#include <gnome--/canvas-rect.h>

/** 
 * Location contains the state of a square on the board.
 * This used to be owened by the computer player, but now
 * this is owned by the GUI.
 */
class Location
{
public:
    Location();
    /// This cleans up the back and front canvas items
    virtual ~Location();
    /// Copy constructor, does not copy back,front handles
    Location(const Location& other);
    /// Assignment operator, does not copy back, front handles
    Location& operator=(const Location& other);
public:
    /// The user can choose to "flag" certain locations as mines
    /// In the future I might as the Microsoft's "?" characters
    enum UserState {
	NOTHING,      ///< No user information
	FLAGGED       ///< User has added a flag to this location
    };
    /// The computer player has several possible opinions:
    enum CompState {
	NONE,         ///< No computer information
	EMPTY,        ///< Certain it is empty
	MINED,        ///< Certain it is mined
	MAYBE_EMPTY,  ///< Guess that it is empty
	MAYBE_MINED   ///< Guess that it is mined
    };

    /// This cleans up the back and front canvas items as well
    void reset();

    // So much data, forget the trailing underscores
   
    /// These are set by GMH_Win
    //@{
    Gnome::CanvasRect *back;
    Gnome::CanvasItem *front;
    //@}

    /// These are set by GameData, read elsewhere
    //@{
    bool hasMine;
    bool selected;
    bool measured;
    guint32 adjacentMines;
    //@}

    /// Manipulate user and help from App
    //@{
    UserState user;
    CompState help;
    //@}

    /// Used during setup
    void set_mine() { hasMine=true; };
};

#endif

/*
$Header: /home/ckuklewicz/cvsroot/gminehunter/Location.hh,v 2.5 2000/08/02 22:00:30 ckuklewicz Exp $
$Log: Location.hh,v $
Revision 2.5  2000/08/02 22:00:30  ckuklewicz
Try and add code to hook up App/Win to Minehunter
Who knows if it will compile

Revision 2.4  2000/07/24 03:57:04  ckuklewicz
Compile cleanly under -Wall

Revision 2.3  2000/07/24 02:14:28  ckuklewicz
Split all into hh and cc

Revision 2.2  2000/07/23 02:38:35  ckuklewicz
The canvas is under my control

Revision 2.1  2000/07/22 17:35:34  ckuklewicz
Added

*/
