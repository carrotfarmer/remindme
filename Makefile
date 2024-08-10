CFLAGS = -Wall -pedantic -std=c99 -lnotify

all: remindme remindd 

remindme: remindme.c shared.c
	mkdir -p build
	gcc $(CFLAGS) `pkg-config --cflags --libs glib-2.0 gdk-pixbuf-2.0` remindme.c shared.c -o build/remindme

remindd: remindd.c shared.c
	mkdir -p build
	gcc $(CFLAGS) `pkg-config --cflags --libs glib-2.0 gdk-pixbuf-2.0` remindd.c shared.c -o build/remindd
