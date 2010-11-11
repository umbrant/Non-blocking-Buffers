CFLAGS=-Wall -g -ggdb -std=c99
CC=gcc

all: service client nameserver_main

service: nbb.o libnbb.a service.c
	$(CC) $(CFLAGS) service.c -o service -L. -lnbb -lpthread -lrt 

client: nbb.o libnbb.a client.c
	$(CC) $(CFLAGS) client.c -o client -L. -lnbb -lpthread -lrt 

nameserver_main: libnbb.a libnameserver.a nameserver_main.c
	$(CC) $(CFLAGS) nameserver_main.c -o nameserver -L. -lnbb -lnameserver -lpthread -lrt 

nbb_multi: nbb.o libnbb.a nbb_multi_thread.c
	$(CC) $(CFLAGS) nbb_multi_thread.c -o nbb_multi -L. -lnbb -lpthread -lrt 

# shared library
libnbb.so.1.0.1: nbb.c
	$(CC) $(CFLAGS) -c -fPIC nbb.c
	$(CC) -shared -Wl,-soname,libnbb.so.1 -o libnbb.so.1.0.1 nbb.o

libnameserver.so.1.0.1: nameserver.c
	$(CC) $(CFLAGS) -c -fPIC nameserver.c
	$(CC) -shared -Wl,-soname,libnameserver.so.1 -o libnameserver.so.1.0.1 nameserver.o

# static library
libnbb.a: nbb.o
	ar rcs libnbb.a nbb.o

libnameserver.a: nameserver.o 
	ar rcs libnameserver.a nameserver.o

nbb.o: nbb.c nbb.h
	$(CC) $(CFLAGS) -c nbb.c

nameserver.o: nameserver.c nameserver.h 
	$(CC) $(CFLAGS) -c nameserver.c

clean:
	rm -rf *.o nbb_multi libnbb.so.1.0.1 *.a
