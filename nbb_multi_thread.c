#include "nbb.h"

void* insert_items(void* ptr)
{
	int counter = 1;

	int size = 10;

  while(counter <= NUM_ITEMS) {
  	// Initialize item ptr with new mem
  	struct obj *item = (struct obj*)malloc(sizeof(struct obj));
  	//int* item = (int*)calloc(1,sizeof(int));
  	//struct obj* ptr_to_defunct_item = NULL;
  	//int ret;

    item->data = (unsigned char*)malloc(size*sizeof(unsigned char));
    for(int i=0; i<size; i++) {
    	item->data[i] = (unsigned char)(i);
    }
    item->size = size;

    writeb(item);

		// Can free here, since the obj has been copied into the buffer
  	free_obj(item);

  	counter++;

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

  	struct obj* retrieved_item = (struct obj*)malloc(sizeof(struct obj));
  	readb(retrieved_item);

  	//printf("read_items pointer ptr %x\n", retrieved_item);
    read_items[i] = *retrieved_item;

		free_obj(retrieved_item);

    i++;
  }

	// Print the results
	//printf("items\n");
  //for (i = 0; i < NUM_ITEMS; i++) {
  //  printf("  %02zu: ", read_items[i].size);
  //  for(int k=0; k<read_items[i].size; k++) {
  //  	printf("%02d ", read_items[i].data[k]);
  //  }
  //  printf("\n");
  //}

  // Print that we're done reading
  printf("\nfunction: %s is done\n", (char*)ptr);

  return 0;
}

int main()
{
  pthread_t thread1, thread2;
  char* insert = "insert";
  char* read = "read";
  int init_ret = -1;
 
  init_ret = init();
  if(init_ret < 0) {
    return -1;
  }
 
  pthread_create(&thread1, NULL, insert_items, (void*)insert);
  pthread_create(&thread2, NULL, read_items, (void*)read);

	void** value_ptr=0;
  pthread_join(thread1, value_ptr);
  pthread_join(thread2, value_ptr);

	// Free remaining buffer items
	struct obj* defunct_obj = NULL;
	do {
		defunct_obj = get_defunct_ptr();
		if(defunct_obj) {
			free_obj(defunct_obj);
		}
	} while(defunct_obj != NULL); 

  clean_mem();

  return 0;
}
