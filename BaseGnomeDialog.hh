#ifndef INCL_BASE_GNOME_DIALOG_HH
#define INCL_BASE_GNOME_DIALOG_HH

// Adapted from BaseGnomeDialog

#include "BaseGtkWindow.hh"

/**
 * BaseGnomeDialog
 * Very much like (and a child of) BaseGnomeWindow, but a dialog.
 * The behavior is to destroy the dialog on a close event.
 * This must be subclassed by specific dialog implementations.
 */

class BaseGnomeDialog
     : public BaseGtkWindow
{
public:
     BaseGnomeDialog(const char* gladefilename, const char* widgetname);
     virtual ~BaseGnomeDialog() {}
protected:
  /** _thisDialog is actually just a downcast of _thisWindow */
     Gnome::Dialog* _thisDialog;
  /** The innovation here is that OnCloseQuery is connected to _thisDialog.close */
     gboolean OnCloseQuery();
};

#endif

/*
$Header: /home/ckuklewicz/cvsroot/gminehunter/BaseGnomeDialog.hh,v 2.0 2000/07/22 17:21:14 ckuklewicz Exp $
$Log: BaseGnomeDialog.hh,v $
Revision 2.0  2000/07/22 17:21:14  ckuklewicz
Synchonizing release numbers

*/
