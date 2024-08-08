CFLAGS = -Wall -pedantic -std=c99 -lnotify

all: remindme remindd 

remindme:
	gcc $(CFLAGS) `pkg-config --cflags --libs glib-2.0 gdk-pixbuf-2.0` remindme.c shared.c -o build/remindme

remindd:
	gcc $(CFLAGS) `pkg-config --cflags --libs glib-2.0 gdk-pixbuf-2.0` remindd.c shared.c -o build/remindd 
