#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#define BUFFER_SIZE 10 
#define NUM_ITEMS 25 

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

int insert_item(struct obj* ptr_to_item, struct obj* ptr_to_defunct_item);
int read_item(struct obj* ref_to_item);
