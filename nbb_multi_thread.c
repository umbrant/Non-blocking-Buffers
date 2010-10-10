#include "nbb.h"


void* insert_items(void* ptr)
{
	int counter = 1;

  while(counter <= NUM_ITEMS) {
  	// Initialize item ptr with new mem
  	struct obj *item = (struct obj*)malloc(sizeof(struct obj));
  	//int* item = (int*)calloc(1,sizeof(int));
  	struct obj* ptr_to_defunct_item = NULL;
  	int ret;

    item->data = (unsigned char*)malloc(counter*sizeof(char));
    for(int i=0; i<counter; i++) {
    	item->data[i] = (unsigned char)i;
    }
    item->size = counter*sizeof(char);

    ret = insert_item(item, ptr_to_defunct_item);

  	if(ret == OK) {
  		//printf("Inserted pointer %x size %zu\n", item, item->size);

			// Free defunct pointer if not NULL
  		if(ptr_to_defunct_item) {
  			free(ptr_to_defunct_item);
  			// TODO: free struct data too
  		}
  		counter++;
  	} else {
			free(item);
  	}

    //pthread_yield();
  }

  printf("function: %s is done\n", (char*)ptr);
  return 0;
}

void* read_items(void* ptr)
{
  int i = 0;
  struct obj read_items[NUM_ITEMS];

  while(i < NUM_ITEMS) {

  	struct obj retrieved_item;
    int ret = read_item(&retrieved_item);

    if(ret == OK) {
  		//printf("read_items pointer ptr %x\n", retrieved_item);
    	read_items[i] = retrieved_item;

      i++;

    }

    //pthread_yield();
  }

	// Print the results
	printf("items\n");
  for (i = 0; i < NUM_ITEMS; i++) {
    printf("  %02zu: ", read_items[i].size);
    for(int k=0; k<read_items[i].size; k++) {
    	printf("%02d ", read_items[i].data[k]);
    }
    printf("\n");
  }

  // Print that we're done reading
  printf("\nfunction: %s is done\n", (char*)ptr);

  return 0;
}

int main()
{
  pthread_t thread1, thread2;
  char* insert = "insert";
  char* read = "read";
 
  pthread_create(&thread1, NULL, insert_items, (void*)insert);
  pthread_create(&thread2, NULL, read_items, (void*)read);

	void** value_ptr=0;
  pthread_join(thread1, value_ptr);
  pthread_join(thread2, value_ptr);

  return 0;
}
