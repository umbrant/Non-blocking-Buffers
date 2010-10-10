#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

// BUFFER_SIZE is limited to ~32,767 since it has to be represented by an unsigned short / 2
#define BUFFER_SIZE 100 
#define NUM_ITEMS 1000

enum { 
  BUFFER_FULL = 0, 
  BUFFER_FULL_CONSUMER_READING, 
  BUFFER_EMPTY,
  BUFFER_EMPTY_PRODUCER_INSERTING,
  OK
};

struct obj {
	size_t size;
	unsigned char* data;
};

// Copy contents of obj1 to obj2
int copy_obj(struct obj *obj1, struct obj *obj2);
int free_obj(struct obj *object);

// These are the functions that the user should normally be calling
//
// They handle blocking vs. async, and copying/freeing things from
// the buffer.

// Asynchronous
int read(struct obj* ptr_to_item);
int write(struct obj* ptr_to_item);
// Synchronous
int readb(struct obj* ptr_to_item);
int writeb(struct obj* ptr_to_item);

// Used to get defunct buffer slots

// Internal methods, better to make the above functions work instead
int insert_item(struct obj* ptr_to_item, struct obj** ptr_to_defunct_item);
int read_item(struct obj* ref_to_item);

struct obj* get_defunct_ptr();
