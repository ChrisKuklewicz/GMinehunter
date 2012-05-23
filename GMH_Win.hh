#ifndef INCL_GMH_WIN_H
#define INCL_GMH_WIN_H

// Adapted from GabberWin.hh (now TryWin)
#include "BaseGtkWindow.hh"
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gdk-pixbuf/gnome-canvas-pixbuf.h>
#include <gnome--/canvas.h>
#include "GameData.hh"
#include "GMH_App.hh"
/**
 * TryWin
 * FIXME: This need to be rewritten when the window logic is added.
 * Added widget_name to hold name in the glade file.
 * 
 */
using namespace SigC;

class GMH_Win : public BaseGtkWindow
{
protected:
    void emit_newgame(GameType type);
    void set_state(bool running);

    GameData *data_;
    /// The widgets that are in the window.
    //@{
    Gnome::Canvas *_canvas;  ///< Game display
    Gnome::About *_about;
    Gtk::Text *_text;
    Gtk::SpinButton *_x_max, *_y_max, *_mines, *_seed;
    Gtk::Button *_human,*_assisted,*_computer,*_stop;
    //@}
    gdouble x1_,y1_,x2_,y2_; ///< Scroll Area
    guint32 x_max_, y_max_, mines_, seed_; ///< Values for current game
public:
    static const char * const glade_name_ = "GMH_Win";
    GMH_Win(const char * glade_filename);
    virtual ~GMH_Win();
    Gtk::Window* thisWindow() { return _thisWindow; };
private:
    GMH_Win(const GMH_Win& win) : BaseGtkWindow(win.filename_.c_str(),glade_name_) {};
    GMH_Win& operator=(const GMH_Win& win) { return *this; };
    /// These are used by on_canvas_event
    //@{
    gint x_down;
    gint y_down;
    gint button;
    bool running_;
    //@}
public:
    /// Slots for window/menu/widget events.
    //@{
    gint on_window_delete(GdkEventAny* e); // send to GMH_App?
    void on_about();
    void on_exit();           // send to GMH_App?
    gint32 on_canvas_event(GdkEvent * event);
    void on_human_click();
    void on_assisted_click();
    void on_computer_click();
    void on_stop_click();   // send to GMH_App?
    //@}
    /// Signals and Slots for internal events
    //@{
    void set_gamedata(GameData * data);
    void append_text(string text) { _text->insert(text); };
    void game_over(GameEnd ending);
    /** Signal that passes new game info
     * @param The type of game (Human, Assisted, Computer)
     * @param x_max, the number of columns
     * @param y_max, the number of rows
     * @param seed, the random number generator seed
     */
    Signal5<void,GameType,guint32,guint32,guint32,guint32> newGame;
    /** This is the cleaned up singal sent by on_canvas_event
     * @param x of type guint32
     * @param y of type guint32
     * @param button of type guint
     */
    Signal3<void,guint32,guint32,guint> userClick;
    /// Signal to request the game to end
    Signal0<void> stopGame;
    //@}

    /** 
     * The update_display function is called to notify the window
     * that the Location data has changed and the back/front canvas
     * colors / items may need to be adjusted.
     */
    void update_display(guint32 x, guint32 y);
protected:
    /// Used for indexing the _color array of GdkColors
    enum ColorState {
	INITIAL=0,
	SELECTED=1,
	EMPTY=2,
	MINED=3,
	GUESS_EMPTY=4,
	GUESS_MINED=5,
	EXPLODED=6,
	MISMARKED=7,
	SIZE_COLORSTATE=8
    };
    // I name the wrapper object with leading underscore
    // and other fields with a trailing underscore.
    // Setup these items in the constructor, and use
    // them to help create items later
    string fontname_;//="-bitstream-courier-bold-r-*-*-16-*-*-*-*-*-*-*";
    Gdk_Font _font;//(fontname_);
    string number_[9];//={"0","1","2","3","4","5","6","7","8"}
    GdkPixbuf *_flag;
    GdkPixbuf *_mine;
    GdkPixbuf *_boom;
    Gdk_Color _color[SIZE_COLORSTATE];
 
    /// Methods to set background and foreground of cells
    /// Return values can be ignored
    Gnome::CanvasRect* setup_background(guint32 x, guint32 y);
    Gnome::CanvasRect* set_background(guint32 x, guint32 y, ColorState cstate);
    Gnome::CanvasItem* place_flag(guint32 x, guint32 y);
    Gnome::CanvasItem* place_mine(guint32 x, guint32 y);
    Gnome::CanvasItem* place_boom(guint32 x, guint32 y);
    Gnome::CanvasText* GMH_Win::place_text(guint32 x, guint32 y);
    void remove_front(guint32 x, guint32 y);
};

#endif

/*
$Header: /home/ckuklewicz/cvsroot/gminehunter/GMH_Win.hh,v 2.7 2000/08/07 03:22:33 ckuklewicz Exp $
$Log: GMH_Win.hh,v $
Revision 2.7  2000/08/07 03:22:33  ckuklewicz
Wow...tons and tons of bugs fixed.
Added the sendPing, received_ping functions.
The COMPUTER_GAME seems to actually work.

Revision 2.6  2000/07/30 05:54:08  ckuklewicz
CLeaninG UP minHunteR cOde

Revision 2.5  2000/07/28 05:15:10  ckuklewicz
Single Player Works

Revision 2.4  2000/07/27 03:44:05  ckuklewicz
Cleaned up signal buffers with BaseSignalBuffer, added DataBuffer

Revision 2.3  2000/07/25 05:35:41  ckuklewicz
still not playable

Revision 2.1  2000/07/23 02:38:35  ckuklewicz
The canvas is under my control

Revision 2.0  2000/07/22 17:21:14  ckuklewicz
Synchonizing release numbers

*/










