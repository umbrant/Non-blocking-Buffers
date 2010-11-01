CFLAGS=-Wall -g -ggdb -std=c99

all: service client nameserver_main

service: nbb.o libnbb.a service.c
	gcc $(CFLAGS) service.c -o service -L. -lnbb -lpthread -lrt 

client: nbb.o libnbb.a client.c
	gcc $(CFLAGS) client.c -o client -L. -lnbb -lpthread -lrt 

nameserver_main: libnbb.a libnameserver.a nameserver_main.c
	gcc $(CFLAGS) nameserver_main.c -o nameserver -L. -lnbb -lnameserver -lpthread -lrt 

nbb_multi: nbb.o libnbb.a nbb_multi_thread.c
	gcc $(CFLAGS) nbb_multi_thread.c -o nbb_multi -L. -lnbb -lpthread -lrt 

# shared library
libnbb.so.1.0.1: nbb.c
	gcc $(CFLAGS) -c -fPIC nbb.c
	gcc -shared -Wl,-soname,libnbb.so.1 -o libnbb.so.1.0.1 nbb.o

libnameserver.so.1.0.1: nameserver.c
	gcc $(CFLAGS) -c -fPIC nameserver.c
	gcc -shared -Wl,-soname,libnameserver.so.1 -o libnameserver.so.1.0.1 nameserver.o

# static library
libnbb.a: nbb.o
	ar rcs libnbb.a nbb.o

libnameserver.a: nameserver.o 
	ar rcs libnameserver.a nameserver.o

nbb.o: nbb.c nbb.h
	gcc $(CFLAGS) -c nbb.c

nameserver.o: nameserver.c nameserver.h 
	gcc $(CFLAGS) -c nameserver.c

clean:
	rm -rf *.o nbb_multi libnbb.so.1.0.1 *.a
