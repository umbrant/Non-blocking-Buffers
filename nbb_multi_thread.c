#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#define BUFFER_SIZE 10
#define NUM_ITEMS 9 

enum { 
  BUFFER_FULL = 0, 
  BUFFER_FULL_CONSUMER_READING, 
  BUFFER_EMPTY,
  BUFFER_EMPTY_PRODUCER_INSERTING,
  OK
};

short ack_counter = 0;
short last_ack_counter = 0;
short update_counter = 0;
short last_update_counter = 0;
int items[BUFFER_SIZE] = {0};

int insert_item(void* ptr_to_item, void* ptr_to_defunct_item)
{
  int temp_ac = ack_counter;

  if (last_update_counter - temp_ac == 2 * BUFFER_SIZE) {
    ptr_to_defunct_item = NULL;
    return BUFFER_FULL;
  }

  if (last_update_counter - temp_ac == (2 * BUFFER_SIZE) - 1) {
    ptr_to_defunct_item = NULL;
    return BUFFER_FULL_CONSUMER_READING;
  }

  update_counter = last_update_counter + 1;
  ptr_to_defunct_item = &items[((last_update_counter / 2) % BUFFER_SIZE)];

  items[(last_update_counter / 2) % BUFFER_SIZE] = *(int*)ptr_to_item;
  update_counter = last_update_counter + 2;
  last_update_counter = update_counter;

  return OK;
}

int read_item(int* ref_to_item)
{
  int temp_uc = update_counter;

  if (temp_uc == last_ack_counter) {
    return BUFFER_EMPTY; 
  }

  if ((temp_uc - last_ack_counter) == 1) {
    return BUFFER_EMPTY_PRODUCER_INSERTING;
  }

  ack_counter = last_ack_counter + 1;
  *ref_to_item = items[((last_ack_counter / 2) % BUFFER_SIZE)];
  ack_counter = last_ack_counter + 2;
  last_ack_counter = ack_counter;

  return OK;
}

void* insert_items(void* ptr)
{
  int* item = (int*)calloc(1,sizeof(int));
  int* ptr_to_defunct_item;
  *item = 0;

  while(*item < NUM_ITEMS) {
    //printf("curr function: %s\n", (char*)ptr);

    insert_item((void*)item, (void*)ptr_to_defunct_item);
    (*item)++;

    //pthread_yield();
  }

  printf("function: %s is done\n", (char*)ptr);
}

void* read_items(void* ptr)
{
  int i = 0;
  int* retrieved_item = (int*)calloc(1,sizeof(int));
  int ret;
  int item[NUM_ITEMS];

  while(i < NUM_ITEMS) {
    //printf("curr function: %s\n", (char*)ptr); 

    ret = read_item(retrieved_item);
    if(ret == OK) {
      i++;
    }

    //printf("ret: %d\n", ret);
    item[i] = *retrieved_item;

    //pthread_yield();
  }

  for (i = 0; i < NUM_ITEMS; i++) {
    printf("item: %d ", item[i]);
  }
  printf("\nfunction: %s is done\n", (char*)ptr);
}

int main()
{
  pthread_t thread1, thread2;
  char* insert = "insert";
  char* read = "read";
 
  pthread_create(&thread1, NULL, insert_items, (void*)insert);
  pthread_create(&thread2, NULL, read_items, (void*)read);

  return 0;
}
