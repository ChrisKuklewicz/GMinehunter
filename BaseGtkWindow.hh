#ifndef INCL_BASE_GTK_WINDOW_H
#define INCL_BASE_GTK_WINDOW_H
// Adapted from BaseGabberWindow.hh

#include "GMH.hh"
#include "GladeHelper.hh"   // getWidgetPtr; includes glade,glade-xml: GladeXML
// GladeHelper.hh also pulls in gtk and gtk--/main
#include <string>

using namespace std;
using SigC::slot;
using SigC::bind;

/** 
 * BaseGtkWindow
 *
 * This acts as a genetic wrapper around a window defined in the .glade file
 * This also acts as a wrapper for the protected Gtk::Window
 * Children of this class correspond to specific windows, and implement
 * and connect the needed slots to methods they have.
 * By the philosophy here, this should know all/most of the Gtk-- classes
 * I added spinbutton, clist,text
 *
 * Instead of passing the widgetname and relying on a global in "GabberApp" to
 * do the loading to return the GladeXML, accept the filename and widget name.
 * One less global dependence.  One more parameter.  Decendents usually handle
 * the detailed constructor call. So this is not burdensome.
 *
 * If, horrors, something other than a file is used, this will need another constructor.
 */


class BaseGtkWindow : public SigC::Object
{
public:
    /** BaseGtkWindow, widgetname is the glade name of the window */
    BaseGtkWindow(const char* gladefilename, const char* widgetname);
    virtual ~BaseGtkWindow();
    void show() { _thisWindow->show(); }
    void hide() { _thisWindow->hide(); }
    virtual void close(); 
    // Object extender
    virtual void set_dynamic();
    const string filename_;
    const string widget_name_;
protected:
    BaseGtkWindow();
    BaseGtkWindow& operator=(const BaseGtkWindow&) { return *this; }
    BaseGtkWindow(const BaseGtkWindow&) {}
protected:
    // Helper functions
    Gtk::Button*          getButton(const char* name)      { return getWidgetPtr<Gtk::Button>(_thisGH, name); }
    Gtk::MenuItem*        getMenuItem(const char* name)    { return getWidgetPtr<Gtk::MenuItem>(_thisGH, name); }
    Gtk::PixmapMenuItem*  getPixmapMenuItem(const char* name) { return getWidgetPtr<Gtk::PixmapMenuItem>(_thisGH, name); }
    Gtk::Entry*           getEntry(const char* name)       { return getWidgetPtr<Gtk::Entry>(_thisGH, name); }
    Gtk::CheckButton*     getCheckButton(const char* name) { return getWidgetPtr<Gtk::CheckButton>(_thisGH, name); }
    Gtk::Label*           getLabel(const char* name)       { return getWidgetPtr<Gtk::Label>(_thisGH, name); }
    Gtk::SpinButton*      getSpinButton(const char* name)  { return getWidgetPtr<Gtk::SpinButton>(_thisGH, name); }
    Gtk::CList*           getCList(const char* name)       { return getWidgetPtr<Gtk::CList>(_thisGH, name); }
    Gtk::Text*            getText(const char* name)        { return getWidgetPtr<Gtk::Text>(_thisGH, name); }
    template <class T> T* getWidget(const char* name)      { return getWidgetPtr<T>(_thisGH, name); }
    template <class T> T* getWidget_GTK(const char* name)  { return getWidgetPtr_GTK<T>(_thisGH, name); }
     
    // Internal refs
    /** This is a wapper around the gtk+ window and child widgets specified in _thisGH */ 
    Gtk::Window*  _thisWindow;
private:
    /** This is the libglade access point for the window and its children */
    GladeXML*     _thisGH;
};
#endif

/*
$Header: /home/ckuklewicz/cvsroot/gminehunter/BaseGtkWindow.hh,v 2.1 2000/07/24 03:57:03 ckuklewicz Exp $
$Log: BaseGtkWindow.hh,v $
Revision 2.1  2000/07/24 03:57:03  ckuklewicz
Compile cleanly under -Wall

Revision 2.0  2000/07/22 17:21:14  ckuklewicz
Synchonizing release numbers

*/
