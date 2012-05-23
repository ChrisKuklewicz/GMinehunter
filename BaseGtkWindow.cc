// Adapted from BaseGabberWindow.cc (now called BaseGtkWindow)

#include "BaseGtkWindow.hh"

BaseGtkWindow::BaseGtkWindow(const char* gladefilename, const char* widgetname) :
    filename_(gladefilename),widget_name_(widgetname)
{ 
     _thisGH =  glade_xml_new(gladefilename, widgetname);
     _thisWindow  = getWidgetPtr<Gtk::Window>(_thisGH, widgetname);
     reference();  // Hmmm...maybe this is the ref count for _thisWindow ?
}

void BaseGtkWindow::set_dynamic()
{
     SigC::Object::set_dynamic();
     set_sink();  // adjusts the floating property of the SigC Object
}

void BaseGtkWindow::close()
{
     unreference();
}

BaseGtkWindow::~BaseGtkWindow()
{
     _thisWindow->destroy();
     gtk_object_unref(GTK_OBJECT(_thisGH));
}

/*
$Header: /home/ckuklewicz/cvsroot/gminehunter/BaseGtkWindow.cc,v 2.0 2000/07/22 17:21:14 ckuklewicz Exp $
$Log: BaseGtkWindow.cc,v $
Revision 2.0  2000/07/22 17:21:14  ckuklewicz
Synchonizing release numbers

*/
