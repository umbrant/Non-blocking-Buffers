#ifndef NBB_H
#define NBB_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>           
#include <poll.h>
#include <unistd.h>

#include <sys/ipc.h>
#include <sys/shm.h>

#include "nameserver.h"

// BUFFER_SIZE is limited to ~32,767 since it has to be represented by an unsigned short / 2
#define BUFFER_SIZE 256 
#define NUM_ITEMS 500000

// This should be dynamic in the future
#define NUM_CHANNELS 10
#define PAGE_SIZE 4096 // This should probably be found programatically

// Betting that it's some kind of malloc/mem allocation error...
//
// This crashes erratically
//#define NUM_ITEMS   523909
// This crashes always
//#define NUM_ITEMS   524000

enum { 
  BUFFER_FULL = 0, 
  BUFFER_FULL_CONSUMER_READING, 
  BUFFER_EMPTY,
  BUFFER_EMPTY_PRODUCER_INSERTING,
  OK,
  SHM_ERROR
};

// Simple channel abstraction
struct channel {
	struct buffer *read;
	struct buffer *write;
}

// This is for a unidirectional buffer
struct buffer {
	// NBB counters
	unsigned short ack_counter = 0;
	unsigned short last_ack_counter = 0;
	unsigned short update_counter = 0;
	unsigned short last_update_counter = 0;
	unsigned short recycle_counter = 0;

	// Pointer to data region
	unsigned char* data;
	unsigned short data_size;

	// Array of objs within data region
	struct channel_item items[BUFFER_SIZE];
}

// Store offset within data region and size of message
struct channel_item {
	unsigned short offset;
	unsigned short size;
};

// Initialize the shared memory
//int init();

// Copy contents of obj1 to obj2
int copy_obj(struct obj *obj1, struct obj *obj2);
int free_obj(struct obj *object);

// These are the functions that the user should normally be calling
//
// They handle blocking vs. async, and copying/freeing things from
// the buffer.

// Asynchronous
int read_asynch(struct obj* ptr_to_item);
int write_asynch(struct obj* ptr_to_item);
// Synchronous
int readb(struct obj* ptr_to_item);
int writeb(struct obj* ptr_to_item);

// Used to get defunct buffer slots

// Internal methods, better to make the above functions work instead
int insert_item(struct obj* ptr_to_item, struct obj** ptr_to_defunct_item);
int read_item(struct obj* ref_to_item);

struct obj* get_defunct_ptr();

// Internal function: clean up the shared memory
int clean_mem();

#endif // NBB_H
