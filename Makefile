LIB ?= /usr/local/lib
INC = /usr/local/faust

DESTDIR ?=
PREFIX ?= 

prefix := $(DESTDIR)$(PREFIX)

all: midiPlayer simplePlayer2 mcMidiPlayer
midiPlayer: simpleLecteur.cpp
	$(CXX)  -std=c++17 -O3 simpleLecteur.cpp   -ljack -lsndfile -lfaust -llo -I/usr/include/gtk-2.0 -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -I/usr/include/cairo -I/usr/include/pango-1.0 -I/usr/lib/x86_64-linux-gnu/gtk-2.0/include -I/usr/include/gdk-pixbuf-2.0 -I/usr/include/atk-1.0 -lgtk-x11-2.0 -lgdk-x11-2.0 -lpangocairo-1.0 -latk-1.0 -lcairo -lgdk_pixbuf-2.0 -lgio-2.0 -lpangoft2-1.0 -lpango-1.0 -lgobject-2.0 -lglib-2.0 -lfontconfig -lfreetype -lpthread `pkg-config  ` -lOSCFaust -o midiPlayer

simplePlayer2: simpleLecteur2.cpp
	$(CXX)  -std=c++17 -O3 simpleLecteur2.cpp   -ljack -lsndfile -lfaust -llo -I/usr/include/gtk-2.0 -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -I/usr/include/cairo -I/usr/include/pango-1.0 -I/usr/lib/x86_64-linux-gnu/gtk-2.0/include -I/usr/include/gdk-pixbuf-2.0 -I/usr/include/atk-1.0 -lgtk-x11-2.0 -lgdk-x11-2.0 -lpangocairo-1.0 -latk-1.0 -lcairo -lgdk_pixbuf-2.0 -lgio-2.0 -lpangoft2-1.0 -lpango-1.0 -lgobject-2.0 -lglib-2.0 -lfontconfig -lfreetype -lpthread `pkg-config  ` -lOSCFaust -o simplePlayer2

 
mcMidiPlayer: main.cpp 
	$(CXX)  -std=c++17 -O3 parametres.cpp parametres.h info.cpp info.h selectFile.cpp selectFile.h Application.cpp main.cpp Application.h   -I$(INC) -lboost_system -lboost_filesystem -ljack -lsndfile -lfaust -lsfml-graphics -lsfml-window -lsfml-system -llo -I/usr/include/gtk-2.0 -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -I/usr/include/cairo -I/usr/include/pango-1.0 -I/usr/lib/x86_64-linux-gnu/gtk-2.0/include -I/usr/include/gdk-pixbuf-2.0 -I/usr/include/atk-1.0 -lGL -lGLU  -lGL  -DGL_GLEXT_PROTOTYPES  -lgtk-x11-2.0 -lgdk-x11-2.0 -lpangocairo-1.0 -latk-1.0 -lcairo -lgdk_pixbuf-2.0 -lgio-2.0 -lpangoft2-1.0 -lpango-1.0 -lgobject-2.0 -lglib-2.0 -lfontconfig -lfreetype -lpthread `pkg-config  ` -lOSCFaust -o mcMidiPlayer


install: 
	([ -e mcMidiPlayer]) && cp mcMidiPlayer $(prefix)/bin


clean:
	rm -f mcMidiPlayer


