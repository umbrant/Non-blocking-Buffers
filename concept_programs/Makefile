CFLAGS=-Wall -g -ggdb -std=c99 

all: simple_event cpu_affinity

cpu_affinity: cpu_affinity.c
	gcc $(CFLAGS) cpu_affinity.c -lpthread -o cpu_affinity

simple_event: simple_event.c 
	gcc $(CFLAGS) simple_event.c -o simple_event

debug: simple_event.c
	gcc $(CFLAGS) -DDEBUG simple_event.c -o simple_event
