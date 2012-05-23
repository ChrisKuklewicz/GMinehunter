// Adapted from BaseGnomeDialog

#include "BaseGnomeDialog.hh"

BaseGnomeDialog::BaseGnomeDialog(const char* gladefilename, const char* widgetname)
     : BaseGtkWindow(gladefilename,widgetname)
{
     _thisDialog = static_cast<Gnome::Dialog*>(_thisWindow);
     _thisDialog->close_hides(false);
     _thisDialog->close.connect(slot(this, &BaseGnomeDialog::OnCloseQuery));
}

gboolean BaseGnomeDialog::OnCloseQuery()
{
     _thisWindow->destroy();
     return true;
}

/*
$Header: /home/ckuklewicz/cvsroot/gminehunter/BaseGnomeDialog.cc,v 2.0 2000/07/22 17:21:14 ckuklewicz Exp $
$Log: BaseGnomeDialog.cc,v $
Revision 2.0  2000/07/22 17:21:14  ckuklewicz
Synchonizing release numbers

*/
