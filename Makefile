# From gtk-config

GTK_LIBDIR = -L/usr/lib -L/usr/X11R6/lib
GTK_INCLUDEDIR = -I/usr/lib/glib/include -I/usr/X11R6/include
GTK_LIBS = -lgtk -lgdk -rdynamic -lgmodule -lglib -ldl -lXi -lXext -lX11 -lm

# From gtkmm-config

GTKMM_INCLUDEDIR = -I/usr/lib/gtkmm/include -I/usr/include -I/usr/lib/glib/include -I/usr/X11R6/include -I/usr/lib/sigc++/include
GTKMM_LIBDIR = -rdynamic -L/usr/lib -L/usr/X11R6/lib 
GKTMM_LIBS = -lgtkmm -lgdkmm -lgtk -lgdk -lgmodule -lglib -ldl -lXi -lXext -lX11 -lm -lsigc -lpthread

# from gnomeConf.sh
#
# Configuration of the gnome-libs package
#

GNOME_LIBDIR = -L/usr/lib
GNOME_INCLUDEDIR = -I/usr/include -DNEED_GNOMESUPPORT_H -I/usr/lib/gnome-libs/include -I/usr/lib/glib/include -I/usr/X11R6/include
GNOME_LIBS = -lgnome -lgnomesupport -L/usr/lib -lesd -laudiofile -lm -L/usr/lib -laudiofile -lm -ldb1 -L/usr/lib -lglib -ldl
GNOMEUI_LIBS = -lgnomeui -lart_lgpl -L/usr/lib -lgdk_imlib -L/usr/lib -L/usr/X11R6/lib -lgtk -lgdk -rdynamic -lgmodule -lglib -ldl -lXi -lXext -lX11 -lm -lSM -lICE -L/usr/lib -L/usr/X11R6/lib -lgtk -lgdk -rdynamic -lgmodule -lglib -ldl -lXi -lXext -lX11 -lm -lgnome -lgnomesupport -L/usr/lib -lesd -laudiofile -lm -L/usr/lib -laudiofile -lm -ldb1 -L/usr/lib -lglib -ldl
GTKXMHTML_LIBS = -lgtkxmhtml -lXpm -ljpeg -lpng -lz -lz -lSM -lICE -L/usr/lib -L/usr/X11R6/lib -lgtk -lgdk -rdynamic -lgmodule -lglib -ldl -lXi -lXext -lX11 -lm


# from gnomemmConf.sh

GNOMEMM_LIBDIR = -L/usr/lib -rdynamic -L/usr/lib -L/usr/X11R6/lib
GNOMEMM_INCLUDEDIR = -I/usr/lib/gtkmm/include -I/usr/lib/gnomemm/include -I/usr/include -I/usr/include -DNEED_GNOMESUPPORT_H -I/usr/lib/gnome-libs/include -I/usr/lib/glib/include -I/usr/X11R6/include -I/usr/lib/sigc++/include
GNOMEMM_LIBS = -lgnomemm -lgtkmm -lgdkmm -lsigc -lpthread -lsigc -lpthread


#From gnome-config gnome gnomeui gtk

GNOME_ALL_LIBDIR  =  -rdynamic -L/usr/lib -L/usr/X11R6/lib
GNOME_ALL_INCLUDEDIR  =  -I/usr/include -DNEED_GNOMESUPPORT_H -I/usr/lib/gnome-libs/include -I/usr/lib/glib/include -I/usr/X11R6/include
GNOME_ALL_LIBS = -lgnomeui -lart_lgpl -lgdk_imlib -lSM -lICE -lgnome -lgnomesupport -lesd -laudiofile -ldb1 -lgtk -lgdk -lgmodule -lglib -ldl -lXi -lXext -lX11 -lm

# From /usr/lib/*.sh of gdk-pixbuf-devel:

GDK_PIXMAP_LIBDIR = -L/usr/lib  -L/usr/lib -L/usr/X11R6/lib 
GDK_PIXMAP_LIBS =  -lgdk_pixbuf -lgtk -lgdk -rdynamic -lgmodule -lglib -ldl -lXi -lXext -lX11 -lm
GDK_PIXMAP_INCLUDEDIR = -I/usr/lib/glib/include -I/usr/X11R6/include

GNOMECANVASPIXBUF_LIBDIR = -L/usr/lib
GNOMECANVASPIXBUF_LIBS = -L/usr/lib -rdynamic -lgmodule -lglib -ldl -lgdk_pixbuf -ltiff -ljpeg -lpng -lz -lgnomecanvaspixbuf
GNOMECANVASPIXBUF_INCLUDEDIR = -I/usr/include -I/usr/lib/glib/include


#CXXBUILD = c++ -g -O2 $< -o $@ $(GNOME_ALL_LIBDIR) $(GNOME_ALL_INCLUDEDIR) $(GNOME_ALL_LIBS) $(GNOMEMM_LIBDIR) $(GNOMEMM_INCLUDEDIR) $(GNOMEMM_LIBS) -lglade -lglade-gnome

#CXXCOMPILE = c++ -g -O2  -rdynamic -L/usr/lib -L/usr/X11R6/lib  -I/usr/include -DNEED_GNOMESUPPORT_H -I/usr/lib/gnome-libs/include -I/usr/lib/glib/include -I/usr/X11R6/include  -I/usr/lib/gtkmm/include -I/usr/lib/gnomemm/include -I/usr/lib/sigc++/include -I/usr/include/gnome-xml -I/usr/lib/sigc++/include

###################################

SHELL=/bin/bash
EXECUTABLE = GMinehunter
LINKCC = g++
CPPFLAGS = -I/usr/lib/gnome-libs/include -I/usr/lib/glib/include -I/usr/X11R6/include  -I/usr/lib/gtkmm/include -I/usr/lib/gnomemm/include -I/usr/lib/sigc++/include -I/usr/include/gnome-xml -I/usr/lib/sigc++/include -I/usr/include/g++-2
#DEBUG = -g
#OPT = -fno-exceptions
DEBUG = 
OPT = -O3 -fno-exceptions
BASEFLAGS = -Wall -W -DNEED_GNOMESUPPORT_H -rdynamic -pipe -march=pentium
CFLAGS = $(BASEFLAGS) $(DEBUG) $(OPT)
CXX = g++
CXXFLAGS = $(CFLAGS)
CXXINCL = 

CXXLIBS =  -v -L/usr/lib -L/usr/X11R6/lib -lgnomemm -lgtkmm -lgdkmm -lsigc -lgthread -lpthread -lgnomeui  -lgnome -lgnomesupport -lgtk -lgdk -lgmodule -lglib -lglade-gnome -lglade -lgdk_pixbuf -lgnomecanvaspixbuf  -L/usr/local/lib -lcln

#.SUFFIXES:
#.SUFFIXES: .S .c .cc .lo .o .s .hh

SRCS := $(wildcard *.cc)
HDRS := $(wildcard *.hh)
OBJS := $(patsubst %.cc,%.o,$(wildcard *.cc))
# MYOBJS := $(OBJS)
MYOBJS := BaseGnomeDialog.o BaseGtkWindow.o DataBuffer.o GMH_App.o GMH_Win.o Game.o GameData.o Location.o Minehunter.o ScanSet.o SignalBuffer.o main.o 
DEPFILE := MakeDepend

OTHER := gminehunter.glade

OTHERDIST := Doxyfile $(OTHER) $(wildcard *.xpm) $(EXECUTABLE) Makefile README COPYING COPYING.LIB

all: $(EXECUTABLE)

dist:
	rm -rf dist-temp
	mkdir dist-temp
	cp $(SRCS) $(HDRS) $(OTHERDIST) ./dist-temp/

$(EXECUTABLE) : depend $(MYOBJS) $(OTHER)
	$(LINKCC) $(CXXFLAGS) $(CXXLIBS) $(MYOBJS) -o $(EXECUTABLE) /usr/local/lib/libcln.a || (echo failed; beep; beep)
	@echo Done
	beep
	beep

$(DEPFILE): $(SRCS) $(HDRS)
	@-rm -f $(DEPFILE)
	@touch $(DEPFILE)
	makedepend  -Y -f $@ $(SRCS) $(HDRS) 2>> /dev/null
	@echo "Dependency file updated with 'makedepend'"

clean:
	-rm -v $(OBJS) $(EXECUTABLE) $(DEPS) *~

explain:
	@echo "The following information represents the program"
	@echo "Final executable: $(EXECUTABLE)"
	@echo "Sources:          $(SRCS)"
	@echo "Headers:          $(HDRS)"
	@echo "Object files:     $(OBJS)"
	@echo "Dependency info:  $(DEPFILE)"
	@echo "Other files:      $(OTHER)"

depend: $(DEPFILE)

-include $(DEPFILE)

# $History$
