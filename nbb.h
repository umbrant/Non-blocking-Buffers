#ifndef NBB_H
#define NBB_H

// Necessary for sys/ipc.h
#define _XOPEN_SOURCE

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
  OK = 0,
  BUFFER_FULL, 
  BUFFER_FULL_CONSUMER_READING, 
  BUFFER_EMPTY,
  BUFFER_EMPTY_PRODUCER_INSERTING,
  SHM_ERROR
};

// Simple channel abstraction
struct channel {
	struct buffer *read;
	unsigned char* read_data;
	struct buffer *write;
	unsigned char* write_data;
};

// Store offset within data region and size of message
struct channel_item {
	unsigned short offset;
	unsigned short size;
};

// This is for a unidirectional buffer
struct buffer {
	// NBB counters
	unsigned short ack_counter;
	unsigned short last_ack_counter;
	unsigned short update_counter;
	unsigned short last_update_counter;
	unsigned short recycle_counter;

	// Offset to data region from buffer*
	// It's probably good to put this on a page boundary
	unsigned short data_offset;
	unsigned short data_size;

	// Array of objs within data region
	struct channel_item items[BUFFER_SIZE];
};


// Initialize the shared memory
//int init();
int init_service();
int get_channel(int* channel_id, int service);

int insert_item(int channel_id, void* ptr_to_item, size_t size);
int read_item(int channel_id, void** ptr_to_item, size_t* size);

// Copy contents of obj1 to obj2
//int copy_obj(struct obj *obj1, struct obj *obj2);
//int free_obj(struct obj *object);

// These are the functions that the user should normally be calling
//
// They handle blocking vs. async, and copying/freeing things from
// the buffer.

/*
// Asynchronous
int read_asynch(struct obj* ptr_to_item);
int write_asynch(struct obj* ptr_to_item);
// Synchronous
int readb(struct obj* ptr_to_item);
int writeb(struct obj* ptr_to_item);

// Used to get defunct buffer slots


//struct obj* get_defunct_ptr();

// Internal function: clean up the shared memory
int clean_mem();
*/

#endif // NBB_H
