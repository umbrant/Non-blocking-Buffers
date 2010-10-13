CFLAGS=-Wall -g -ggdb -std=c99

all: nbb_multi shm_prototype

nbb_multi: nbb.o libnbb.a nbb_multi_thread.c
	gcc $(CFLAGS) nbb_multi_thread.c -o nbb_multi -L. -lnbb -lpthread -lrt 

#nbb_multi_thread.o: nbb_multi_thread.c nbb.o
#gcc $(CFLAGS) -c nbb_multi_thread.c

# shared library
libnbb.so.1.0.1: nbb.c
	gcc $(CFLAGS) -c -fPIC nbb.c
	gcc -shared -Wl,-soname,libnbb.so.1 -o libnbb.so.1.0.1 nbb.o

# static library
libnbb.a: nbb.o
	ar rcs libnbb.a nbb.o

nbb.o: nbb.c nbb.h
	gcc $(CFLAGS) -c nbb.c

shm_prototype: shm_prototype.c
	gcc $(CFLAGS) -c shm_prototype.c -o shm_prototype -lpthread -lrt

clean:
	rm -rf *.o nbb_multi libnbb.so.1.0.1 *.a
