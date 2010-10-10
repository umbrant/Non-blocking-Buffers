#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#define BUFFER_SIZE 10 
#define NUM_ITEMS 100 

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
int* items[BUFFER_SIZE] = {0};

int insert_item(void* ptr_to_item, void* ptr_to_defunct_item)
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
  items[(last_update_counter / 2) % BUFFER_SIZE] = (int*)ptr_to_item;
  //printf("insert_item pointer %x -> %d\n", (int*)ptr_to_item, *(int*)ptr_to_item);
  update_counter = last_update_counter + 2;
  last_update_counter = update_counter;

  return OK;
}

int read_item(int* ref_to_item)
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
  int* temp_ptr = items[((last_ack_counter / 2) % BUFFER_SIZE)];
  *ref_to_item = *temp_ptr;
  //printf("read_item ret pointer %x -> %d\n", ref_to_item, *ref_to_item);
  ack_counter = last_ack_counter + 2;
  last_ack_counter = ack_counter;

  return OK;
}

void* insert_items(void* ptr)
{
	int counter = 0;

  while(counter < NUM_ITEMS) {
  	// Initialize item ptr with new mem
  	int* item = (int*)calloc(1,sizeof(int));
  	int* ptr_to_defunct_item = NULL;
  	int ret;

    *item = counter;

    ret = insert_item((void*)item, (void*)ptr_to_defunct_item);

  	if(ret == OK) {
  		//printf("Inserted pointer %x -> %d\n", item, *item);

			// Free defunct pointer if not NULL
  		if(ptr_to_defunct_item) {
  			free(ptr_to_defunct_item);
  		}
  		counter++;
  	}

    //pthread_yield();
  }

  printf("function: %s is done\n", (char*)ptr);
}

void* read_items(void* ptr)
{
  int i = 0;
  int read_items[NUM_ITEMS];

  while(i < NUM_ITEMS) {

  	int retrieved_item;
    int ret = read_item(&retrieved_item);

    if(ret == OK) {
  		//printf("read_items pointer ptr %x\n", retrieved_item);
    	read_items[i] = retrieved_item;

      i++;

    }

    //pthread_yield();
  }

	// Print the results
	printf("items: ");
  for (i = 0; i < NUM_ITEMS; i++) {
    printf("%d ", read_items[i]);
  }

  // Print that we're done reading
  printf("\nfunction: %s is done\n", (char*)ptr);
}

int main()
{
  pthread_t thread1, thread2;
  char* insert = "insert";
  char* read = "read";
 
  pthread_create(&thread1, NULL, insert_items, (void*)insert);
  pthread_create(&thread2, NULL, read_items, (void*)read);

	void** value_ptr;
  pthread_join(thread1, value_ptr);
  pthread_join(thread2, value_ptr);

  return 0;
}
