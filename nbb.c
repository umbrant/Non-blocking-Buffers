#include "nbb.h"

short ack_counter = 0;
short last_ack_counter = 0;
short update_counter = 0;
short last_update_counter = 0;
struct obj* items[BUFFER_SIZE] = {0};

int insert_item(struct obj* ptr_to_item, struct obj* ptr_to_defunct_item)
{
  short temp_ac = ack_counter;

  if (last_update_counter - temp_ac == 2 * BUFFER_SIZE) {
    ptr_to_defunct_item = NULL;
    return BUFFER_FULL;
  }

  if (last_update_counter - temp_ac == (2 * BUFFER_SIZE) - 1) {
    ptr_to_defunct_item = NULL;
    return BUFFER_FULL_CONSUMER_READING;
  }

  update_counter = last_update_counter + 1;
  ptr_to_defunct_item = items[((last_update_counter / 2) % BUFFER_SIZE)];

  // Store a pointer to the data item in the NBB
  // The pointee's value has been set by the caller
  items[(last_update_counter / 2) % BUFFER_SIZE] = ptr_to_item;
  //printf("insert_item pointer %x -> %d\n", ptr_to_item, ptr_to_item);
  update_counter = last_update_counter + 2;
  last_update_counter = update_counter;

  return OK;
}

int read_item(struct obj* ref_to_item)
{
  short temp_uc = update_counter;

  if (temp_uc == last_ack_counter) {
    return BUFFER_EMPTY; 
  }

  if ((temp_uc - last_ack_counter) == 1) {
    return BUFFER_EMPTY_PRODUCER_INSERTING;
  }

  ack_counter = last_ack_counter + 1;

  // Copy out the value pointed to by the pointer in the NBB
  struct obj* temp_ptr = items[((last_ack_counter / 2) % BUFFER_SIZE)];
  *ref_to_item = *temp_ptr;
  //printf("read_item ret pointer %x\n", ref_to_item);
  //printf("read_item ret pointer %x -> %d\n", ref_to_item, *ref_to_item);
  ack_counter = last_ack_counter + 2;
  last_ack_counter = ack_counter;

  return OK;
}
