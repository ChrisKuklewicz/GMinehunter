#ifndef INCL_GLADE_HELPER_HH
#define INCL_GLADE_HELPER_HH

// Adapted from GladeHelper

#include <gtk/gtk.h>          // GtkObject
#include <gtk--/main.h>       // Gtk::wrap_auto
#include <glade/glade.h>      // includes glade-xml.h: GladeXML, glade_xml_get_widget

/**
 * getWidgetPtr and GetWidgetPtr_GTK
 * These template functions wrap the glade_xml_get_widget call (from glade.h)
 * getWidgetPtr (used in BaseGtkWindow below) also uses Gtk::wrap_auto
 * These functions are included as templated method calls in BaseGtkWindow
 * TODO: learn what Gtk::wrap_auto is actually doing  ;)
 * Note: The implementation is here, so there is no GladeHelper.cc
 * Cannot add const to GladeXML* parameters, as glade-xml.h does not use it
 */

template<class T> T* getWidgetPtr(GladeXML* g, const char* name)
{
     T* result = static_cast<T*>(Gtk::wrap_auto((GtkObject*)glade_xml_get_widget(g, name)));
     if (result == NULL)
     {
	  cerr << "** ERROR **: unable to load widget: " << name << endl;
	  g_assert(result != NULL);
     }
     return result;
}

template<class T> T* getWidgetPtr_GTK(GladeXML* g, const char* name)
{
     T* result = (T*)glade_xml_get_widget(g, name);
     if (result == NULL)
     {
	  cerr << "** ERROR **: unable to load widget: " << name << endl;
	  g_assert(result != NULL);
     }
     return result;     
}

#endif

/*
$Header: /home/ckuklewicz/cvsroot/gminehunter/GladeHelper.hh,v 2.0 2000/07/22 17:21:14 ckuklewicz Exp $
$Log: GladeHelper.hh,v $
Revision 2.0  2000/07/22 17:21:14  ckuklewicz
Synchonizing release numbers

*/
