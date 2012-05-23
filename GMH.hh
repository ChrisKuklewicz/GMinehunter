// Define __GNU_SOURCE to get recursive mutex types
#define SIGC_THREAD_IMPL 1
#define _GNU_SOURCE
#ifndef INCL_GMH_H
#define INCL_GMH_H
// This is the central #include header file

// Pull from Gabber.hh to get includes:

#include <glib.h>
#include <gtk/gtk.h>
#include <glade/glade.h>
#include <gnome.h>
#include <libgnome/gnome-i18n.h>

#include <gtk--.h>
#include <gnome--.h>
#include <gnome--/dialog.h>

#include <iostream.h>
#include <iomanip.h>
#include <fstream>
#include <string>

#include <sigc++/bind.h>
#include <sigc++/signal_system.h>
using namespace std;
//using SigC::slot;
//using SigC::bind;

// Functions called via macros

inline void _real_assert(bool b,const char *file,int line,
                         const char *func, const char *msg) 
{
    if (!b) {
	cerr << endl << "! " << msg << " failed : " << file 
             << " : " << line << " : " << func << endl;
	exit(1);
    }
}

inline void _real_message(const char *file,int line,
                          const char *func, const char *msg) 
{
    cerr << "$ Message : " << file << " : "
         << line << " : " << func << " : " << msg << endl;
}

#define _must_assert(__f__) _real_assert((__f__),__FILE__,__LINE__, \
 __PRETTY_FUNCTION__,"_assert " #__f__)


#define NOT_DEBUGING 1

#ifndef NOT_DEBUGING

// Macros to help debug that call the above functions

#define _assert(__f__) _real_assert((__f__),__FILE__,__LINE__, \
 __PRETTY_FUNCTION__,"_assert " #__f__)

#define _nullChk(__p__) _real_assert(NULL!=(__p__),__FILE__,__LINE__, \
 __PRETTY_FUNCTION__,"_nullChk " #__p__)

#define _messsage(__m__) _real_message(__FILE__,__LINE__, \
 __PRETTY_FUNCTION__,__m__)

#define _never_get_here _real_assert(false,__FILE__,__LINE__, \
 __PRETTY_FUNCTION__,"_never_get_here")

// Execution tracing macros for debugging
#define _you_are_here do { cerr << "@ " << __FILE__ << " : " << __LINE__ \
 << " : " << __PRETTY_FUNCTION__ << endl; } while(0)

#define _show_args0() _you_are_here;

#define _show_args1(Arg1)  do { _you_are_here; cerr << "args: " \
 << #Arg1 << "=" << (Arg1) \
 << endl; } while (0)

#define _show_args2(Arg1,Arg2)  do { _you_are_here; cerr << "args: " \
 << #Arg1 << "=" << (Arg1)  \
 << ", " << #Arg2 << "=" << (Arg2) \
 << endl; } while (0)

#define _show_args3(Arg1,Arg2,Arg3) do { _you_are_here; \
 cerr << "args: " \
 << #Arg1 << "=" << (Arg1)  \
 << ", " << #Arg2 << "=" << (Arg2) \
 << ", " << #Arg3 << "=" << (Arg3) \
 << endl; } while (0)

#define _show_args4(Arg1,Arg2,Arg3,Arg4) do { _you_are_here; \
 cerr << "args: " \
 << #Arg1 << "=" << (Arg1)  \
 << ", " << #Arg2 << "=" << (Arg2) \
 << ", " << #Arg3 << "=" << (Arg3) \
 << ", " << #Arg4 << "=" << (Arg4) \
 << endl; } while (0)

#define _show_args5(Arg1,Arg2,Arg3,Arg4,Arg5) do { _you_are_here; \
 cerr << "args: " \
 << #Arg1 << "=" << (Arg1)  \
 << ", " << #Arg2 << "=" << (Arg2) \
 << ", " << #Arg3 << "=" << (Arg3) \
 << ", " << #Arg4 << "=" << (Arg4) \
 << ", " << #Arg5 << "=" << (Arg5) \
 << endl; } while (0)

#else

// Only keep functionality for _nullChk and _never_get_here
#define _assert(__f__)
#define _nullChk(__p__) do { if (NULL==(__p__))  { exit(14); } } while (0)
#define _messsage(__m__) 
#define _never_get_here do { exit(13); } while (0)
#define _you_are_here 
#define _show_args0()
#define _show_args1(Arg1)
#define _show_args2(Arg1,Arg2)
#define _show_args3(Arg1,Arg2,Arg3)
#define _show_args4(Arg1,Arg2,Arg3,Arg4)
#define _show_args5(Arg1,Arg2,Arg3,Arg4,Arg5)

#endif

// Enumerations declared globally

enum GameType {
    HUMAN_GAME,
    ASSISTED_GAME,
    COMPUTER_GAME
};

enum GameEnd {
    LOST_GAME, 
    WON_GAME,
    CANCEL_GAME
};

#endif
/*
$Header: /home/ckuklewicz/cvsroot/gminehunter/GMH.hh,v 2.8 2000/08/11 04:56:03 ckuklewicz Exp $
$Log: GMH.hh,v $
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

Revision 2.6  2000/08/07 03:22:32  ckuklewicz
Wow...tons and tons of bugs fixed.
Added the sendPing, received_ping functions.
The COMPUTER_GAME seems to actually work.

Revision 2.5  2000/07/31 04:22:15  ckuklewicz
Everything compiles and links.  Happy Day
Single user still works.
Need to implement guess_it() from java
The 2^29 limit in cln may need examining

Revision 2.4  2000/07/27 03:44:04  ckuklewicz
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
