// Adapted from GabberWin.cc (now TryWin)

#include <list>
#define __USE_ISOC9X 1
#include <math.h>
#include "flag.xpm"
#include "mine.xpm"
#include "GMH_Win.hh"

using namespace SigC;

GMH_Win::GMH_Win(const char * glade_filename) : 
    BaseGtkWindow(glade_filename,"GMH_Win"),
    data_(NULL), x_down(-1), y_down(-1),button(-1),running_(false),
    fontname_("-bitstream-courier-bold-r-*-*-16-*-*-*-*-*-*-*")
    //  ,  _font(fontname_) 
{
    _font.create(fontname_);
    number_[0]="0";
    number_[1]="1";
    number_[2]="2";
    number_[3]="3";
    number_[4]="4";
    number_[5]="5";
    number_[6]="6";
    number_[7]="7";
    number_[8]="8";
    // Wrap widgets, make sure != NULL
    _about=NULL;    
    _nullChk(_canvas=getWidget<Gnome::Canvas>("GMH_Canvas"));
    _nullChk(_text=getText("GMH_Text"));
    _nullChk(_x_max=getSpinButton("GMH_Columns_spin"));
    _nullChk(_y_max=getSpinButton("GMH_Rows_spin"));
    _nullChk(_mines=getSpinButton("GMH_Mines_spin"));
    _nullChk(_seed=getSpinButton("GMH_Seed_spin"));
    // Connect toolbar controls
    _nullChk(_human=getButton("GMH_Human_btn"));
    _nullChk(_assisted=getButton("GMH_Assisted_btn"));
    _nullChk(_computer=getButton("GMH_Computer_btn"));
    _nullChk(_stop=getButton("GMH_Stop_btn"));
    // Setup initial values
    data_=NULL;
    _stop->set_sensitive(false);
    const guint init_size=10;
    x1_=-1;
    y1_=-1;
    x2_=init_size+1;
    y2_=init_size+1;
    _x_max->set_value(init_size);
    _y_max->set_value(init_size);
    _mines->set_value(init_size);
    _seed->set_value(2001);
    x_max_=init_size;
    y_max_=init_size;
    seed_=2001;
    _canvas->set_scroll_region(x1_,y1_,x2_,y2_);
    // I am clumsy with this for now
    _text->set_point(0);
    _text->forward_delete(_text->get_length());
    _text->insert("Welcome, student, to Minehunter!\nEnjoy!\n");
    // Make primitives
    _nullChk(_flag=gdk_pixbuf_new_from_xpm_data ((const char**)flag_xpm));
    _nullChk(_mine=gdk_pixbuf_new_from_xpm_data ((const char**)mine_xpm));
    _boom=NULL;
    _must_assert(_canvas->get_color("grey",&_color[INITIAL]));  // empty or flag
    _must_assert(_canvas->get_color("blue",&_color[SELECTED]));  // white text
    _must_assert(_canvas->get_color("green",&_color[EMPTY]));   // hint
    _must_assert(_canvas->get_color("red",&_color[MINED]));    // hint
    _must_assert(_canvas->get_color("yellow",&_color[GUESS_EMPTY]));  // hint
    _must_assert(_canvas->get_color("purple2",&_color[GUESS_MINED]));  // hint
    _must_assert(_canvas->get_color("white",&_color[EXPLODED]));  // mine
    _must_assert(_canvas->get_color("white",&_color[MISMARKED]));  // flag
    // Connect window and menu events
    getMenuItem("exit1")->activate.connect(slot(this, &on_exit));
    getMenuItem("about1")->activate.connect(slot(this, &on_about));
    _thisWindow->delete_event.connect(slot(this, &on_window_delete));
    // Connect toolbar controls
    _human->clicked.connect(slot(this,&on_human_click));
    _assisted->clicked.connect(slot(this,&on_assisted_click));
    _computer->clicked.connect(slot(this,&on_computer_click));
    _stop->clicked.connect(slot(this,&on_stop_click));
    // Connect canvas root group item
    _canvas->root()->event.connect(slot(this,&on_canvas_event));
    // Turn it on
    _thisWindow->realize();
    _thisWindow->show();
} 

GMH_Win::~GMH_Win() 
{
    _text->destroy();
    gtk_object_unref(GTK_OBJECT(_text));
    _x_max->destroy();
    gtk_object_unref(GTK_OBJECT(_x_max));
    _y_max->destroy();
    gtk_object_unref(GTK_OBJECT(_y_max));
    _mines->destroy();
    gtk_object_unref(GTK_OBJECT(_mines));

//    _seed->destroy();
    //  gtk_object_unref(GTK_OBJECT(_seed));
    _canvas->destroy();
    gtk_object_unref(GTK_OBJECT(_canvas));
}

gint GMH_Win::on_window_delete(GdkEventAny* e) 
{
    Gnome::Main::quit();
    return 0;
}


void GMH_Win::on_about() 
{
    const string title("GMinehunter");
    const string version("x.x");
    const string copyright("Copyright 2000 GPL");
    const string author("Christopher Kuklewicz <chrisk@mit.edu>");
    list<string> authors;
    authors.insert(authors.begin(),author);
//    const char * const * authors= {"Christopher Kuklewicz <chrisk@mit.edu>"};
    const string comments("Made with gnome--/gtk--/libsigc++/libglade\nEnjoy!");
    _nullChk(_about=new Gnome::About(title,version,copyright,authors,comments,""));
    _about->set_parent(*_thisWindow);
    _about->run_and_close();
    delete _about;
    _about=NULL;
}

void GMH_Win::on_exit() 
{
    // Brutal but effective
    Gnome::Main::quit();
}


/**
 * on_canvas_event responds to proper clicks (down and up) when they
 * are on the same location.  It then send the location and button
 * number to the userClick signal
 */
gint32 GMH_Win::on_canvas_event(GdkEvent * event) 
{
    if (!running_) 
        return TRUE;
    if (NULL==event) {
        cerr << "NULL event passes to GMH_Win::on_canvas_event" << endl;
        return FALSE;
    }
    // guint32 time=event->button.time;
    if(GDK_BUTTON_PRESS==event->type) {
        gdouble x=event->button.x;
        gdouble y=event->button.y;
        gint b=event->button.button;
        if ( ( 0.0 <= x )  && ( x < x_max_ ) && 
             ( 0.0 <= y )  && ( y < y_max_ ) )           
        {
            x_down=lround(floor(x));
            y_down=lround(floor(y));
            button=b;
        } else {
            x_down=-1;
            y_down=-1;
        }
    } else { 
        if(GDK_BUTTON_RELEASE==event->type) {
            gdouble x=event->button.x;
            gdouble y=event->button.y;
            gint b=event->button.button;
            if ( ( 0.0 <= x )  && ( x < x_max_ ) && 
                 ( 0.0 <= y )  && ( y < y_max_ ) ) 
            {
                // x_up and y_up are non-negative
                gint x_up=lround(floor(x));
                gint y_up=lround(floor(y));
                if ((x_up==x_down) && (y_up==y_down) && (b==button)) 
                {
                    x_down=-1;
                    y_down=-1;
                    //cout << "(" << x << ", " << y << ") #" 
                    //   << "(" << x_up << ", " << y_up << ") #" 
                    //  << b << endl;
                    userClick.emit(x_up,y_up,b);
                }
            }
        }
    }
    return FALSE;
}

void GMH_Win::on_human_click() 
{ 
    // cout << "Human" << endl;
    emit_newgame(HUMAN_GAME);
}

void GMH_Win::on_assisted_click() 
{ 
    // cout << "Assisted" << endl;
    emit_newgame(ASSISTED_GAME);
}

void GMH_Win::on_computer_click() 
{ 
    // cout << "Computer" << endl;
    emit_newgame(COMPUTER_GAME);
}

void GMH_Win::on_stop_click() 
{ 
    // cout << "Stop" << endl;
    stopGame.emit();
}

void GMH_Win::game_over(GameEnd ending) 
{
    _show_args1((int)ending);
    // TODO : Draw any remaining mines with place_mine
    set_state(false);
}

/** 
 * This function when the game starts.
 * In some sense, the controller (GMH_App) passes the
 * model (GameData) to the view (GMH_Win).
 */
void GMH_Win::set_gamedata(GameData * data) 
{
    guint32 x,y; // Used in get_sizes and as loop variables
    data_=data;
    // Setup canvas scroll region
    data_->get_sizes(&x,&y);
    x_max_=x;
    y_max_=y;
    mines_=data_->get_mines();
    x1_=-1;
    y1_=-1;
    x2_=x_max_+1;
    y2_=x_max_+1;
    _canvas->set_scroll_region(x1_,y1_,x2_,y2_);
    // Reset the text area
    _text->set_point(0);
    _text->forward_delete(_text->get_length());
    _text->insert("Welcome, student, to Minehunter!\nEnjoy!\n");

    _canvas->freeze();
    for(x=0; x<x_max_; x++) {
        for (y=0; y<y_max_; y++) {
            set_background(x,y,INITIAL);
        }
    }
    _canvas->thaw();
}

void GMH_Win::emit_newgame(GameType type) 
{
    set_state(true);
    _text->set_point(0);
    _text->forward_delete(_text->get_length());

    x_max_=CLAMP((guint32)(_x_max->get_value_as_int()),0,GameData::xy_max_);
    _x_max->set_value(x_max_);
    y_max_=CLAMP((guint32)(_y_max->get_value_as_int()),0,GameData::xy_max_);
    _y_max->set_value(y_max_);
    mines_=CLAMP((guint32)(_mines->get_value_as_int()),0,x_max_*y_max_);
    _mines->set_value(mines_);
    seed_=(guint32)(_seed->get_value_as_int());
    newGame.emit(type,x_max_,y_max_,mines_,seed_);
}

void GMH_Win::set_state(bool running) 
{
    running_=running;
    if (running) {
        _human->set_sensitive(false);
        _assisted->set_sensitive(false);
        _computer->set_sensitive(false);
        _stop->set_sensitive(true);
    } else {
        _human->set_sensitive(true);
        _assisted->set_sensitive(true);
        _computer->set_sensitive(true);
        _stop->set_sensitive(false);
    }
}

/// Methods to set background and foreground of cells
/// Return values can be ignored
Gnome::CanvasRect* GMH_Win::setup_background(guint32 x, guint32 y) 
{
    _nullChk(data_);
    _assert(x<x_max_);
    _assert(y<y_max_);

    if (NULL==data_->get_const_location(x,y).back) {
        //DEBUG cout << "setup new " << x << " "<< y << endl;
        gdouble x1=x,y1=y,x2=x+1.0,y2=y+1.0;
        // Instead of calling manage, the ~Location will delete the back
        data_->get_location(x,y).back=new Gnome::CanvasRect(
            *(_canvas->root()),"x1",x1,"y1",y1,"x2",x2, "y2",y2,
            "outline_color","black",NULL);
    } else {
        //DEBUG cout << "already setup " << x << " " << y << endl;
    }
    set_background(x,y,INITIAL);
    return data_->get_location(x,y).back;
}

Gnome::CanvasRect* GMH_Win::set_background(guint32 x, guint32 y, ColorState cstate) 
{
    _nullChk(data_);
    _assert(x<x_max_);
    _assert(y<y_max_);
    //DEBUG cout << "setback " << x << " " << y << endl;
    if (NULL==data_->get_location(x,y).back) {
        setup_background(x,y);
    }
    data_->get_location(x,y).back->set_fill_color_gdk(_color[cstate]);
    return data_->get_location(x,y).back;
}

Gnome::CanvasItem* GMH_Win::place_flag(guint32 x, guint32 y) 
{
    Gnome::CanvasItem * item;
    _nullChk(data_);
    // DEBUG cout << "place_flag" << x << y << endl;
    _assert(x<x_max_);
    _assert(y<y_max_);

    if (NULL!=data_->get_const_location(x,y).front) {
        remove_front(x,y);
    }
    item=Gtk::wrap(GNOME_CANVAS_ITEM(
        gnome_canvas_item_new(gnome_canvas_root(_canvas->gtkobj()),
                              gnome_canvas_pixbuf_get_type(),
                              "pixbuf", _flag,
                              "height_in_pixels", TRUE,
                              "width_in_pixels", TRUE,
                              "x",(gdouble)2.0,
                              "y",(gdouble)2.0,
                              "x_in_pixels", TRUE,
                              "y_in_pixels", TRUE,
                              NULL)));
    item->move(x,y);
    data_->get_location(x,y).front=item;
    item->show();
    return data_->get_location(x,y).front;
}

Gnome::CanvasItem* GMH_Win::place_mine(guint32 x, guint32 y) 
{
    Gnome::CanvasItem * item;
    _nullChk(data_);
    // DEBUG cout << "place_mine" << x << y << endl;
    _assert(x<x_max_);
    _assert(y<y_max_);

    if (NULL!=data_->get_const_location(x,y).front) {
        remove_front(x,y);
    }
    item=Gtk::wrap(GNOME_CANVAS_ITEM(
        gnome_canvas_item_new(_canvas->root()->gtkobj(),
                              gnome_canvas_pixbuf_get_type(),
                              "pixbuf", _mine,
                              "height_in_pixels", TRUE,
                              "width_in_pixels", TRUE,
                              "x",(gdouble)2.0,
                              "y",(gdouble)2.0,
                              "x_in_pixels", TRUE,
                              "y_in_pixels", TRUE,
                              NULL)));
    item->move(x,y);
    data_->get_location(x,y).front=item;
    item->show();
    return data_->get_location(x,y).front;
}

/// Postcondition : NULL==data_->get_location(x,y).front
void GMH_Win::remove_front(guint32 x, guint32 y)
{
    _nullChk(data_);
    // DEBUG cout << "remove_front" << x << y << endl;
    _assert(x<x_max_);
    _assert(y<y_max_);
    if (NULL!=data_->get_const_location(x,y).front) {
//        data_->get_const_location(x,y).front->destroy();
        data_->get_location(x,y).front->destroy();
        data_->get_location(x,y).front=NULL;

//        gtk_object_unref(GTK_OBJECT(data_->get_const_location(x,y).front->gtkobj()));
//        data_->get_location(x,y).front=NULL;
    }
}

Gnome::CanvasText* GMH_Win::place_text(guint32 x, guint32 y)
{
    _nullChk(data_);
    // DEBUG cout << "place_text" << x << y << endl;
    _assert(x<x_max_);
    _assert(y<y_max_);
    Location& loc = data_->get_location(x,y);
    // Use managed memory model for front items
    Gnome::CanvasText *text = manage(
	new Gnome::CanvasText(*(_canvas->root())));
    text->set_text(number_[loc.adjacentMines]);
    text->set_font(fontname_);
    text->set_x(x+0.5);
    text->set_y(y+0.5);
    text->set_fill_color("white");
    //    text->set_font_gdk(_font);
    // DEBUG cout << "placed a number " << loc.adjacentMines << number_[loc.adjacentMines].c_str() << endl;
    if (NULL!=loc.front)
        remove_front(x,y);

    text->hide();
    text->show();
    loc.front=text;
    return text;
}

void GMH_Win::update_display(guint32 x, guint32 y)
{
    _show_args2(x,y);
    _nullChk(data_);
    // DEBUG cout << "update_display" << x << y << endl;
    _assert(x<x_max_);
    _assert(y<y_max_);
    // This handles alot of cases..in fact all of them
    Location& loc = data_->get_location(x,y);
    if (loc.selected) 
    {
        if (loc.hasMine) 
        {
            // GAME OVER LOCATION : LOST_GAME
            // TODO : nice explosion pixmap
            set_background(x,y,EXPLODED);
            place_mine(x,y);
            // DEBUG cout << "DIED" << endl;
        } 
        else 
        {
            Gnome::CanvasText* p_text=place_text(x,y);
            if (Location::MAYBE_EMPTY==loc.help)
            {
                // Keep visual record of Assist
                // after it is selected.
                set_background(x,y,GUESS_EMPTY);
                p_text->set_fill_color("black");
            }
            else
            {
                set_background(x,y,SELECTED);
            }
        }
    }
    else
    {
        // Set background
        switch (loc.help) 
        {
            case Location::NONE:
                set_background(x,y,INITIAL);
                break;
            case Location::EMPTY:
                set_background(x,y,EMPTY);
                break;
            case Location::MINED:
                set_background(x,y,MINED);
                break;
            case Location::MAYBE_EMPTY:
                set_background(x,y,GUESS_EMPTY);
                break;
            case Location::MAYBE_MINED:
                set_background(x,y,GUESS_MINED);
                break;
            default:
                _never_get_here;
        }
        // Set foreground
        if (Location::FLAGGED==loc.user)
        {
            place_flag(x,y);
            set_background(x,y,INITIAL);
        }
        else
        {
            remove_front(x,y);
        }
    }
    _canvas->update_now();
}


/*
  $Header: /home/ckuklewicz/cvsroot/gminehunter/GMH_Win.cc,v 2.8 2000/08/11 04:56:03 ckuklewicz Exp $
  $Log: GMH_Win.cc,v $
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

  Revision 2.5  2000/07/28 05:15:09  ckuklewicz
  Single Player Works

  Revision 2.4  2000/07/27 03:44:05  ckuklewicz
  Cleaned up signal buffers with BaseSignalBuffer, added DataBuffer

  Revision 2.3  2000/07/25 05:35:41  ckuklewicz
  still not playable

  Revision 2.2  2000/07/24 03:57:04  ckuklewicz
  Compile cleanly under -Wall

  Revision 2.1  2000/07/23 02:38:35  ckuklewicz
  The canvas is under my control

  Revision 2.0  2000/07/22 17:21:14  ckuklewicz
  Synchonizing release numbers

*/
