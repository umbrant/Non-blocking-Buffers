CFLAGS=-Wall -g -ggdb -std=c99 

all: simple_event

simple_event: simple_event.c 
	gcc $(CFLAGS) simple_event.c -o simple_event

debug: simple_event.c
	gcc $(CFLAGS) -DDEBUG simple_event.c -o simple_event
