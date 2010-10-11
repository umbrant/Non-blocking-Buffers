#include "nbb.h"

unsigned short ack_counter = 0;
unsigned short last_ack_counter = 0;
unsigned short update_counter = 0;
unsigned short last_update_counter = 0;
unsigned short recycle_counter = 0;
struct obj* items[BUFFER_SIZE] = {0};

// Shared memory variables
const char* SHARED_MEM_NAME = "nbb_v1"; // TODO: should not be hardcoded
struct pollfd read_pollfd;
int insert_fd = -1;

int init() {
  int read_fd = -1;

  memset(&read_pollfd, 0, sizeof(struct pollfd));

  insert_fd = shm_open(SHARED_MEM_NAME, O_RDWR | O_CREAT, 0666);
  if (insert_fd < 0) {
    perror("! insert fd\n");
    return -1;
  }

  read_fd = shm_open(SHARED_MEM_NAME, O_RDONLY, 0666);
  if (read_fd < 0) {
    perror("! read fd\n");
    return SHM_ERROR;
  }

  read_pollfd.fd = read_fd;
  read_pollfd.events = POLLIN;

  return 1;
}

int copy_obj(struct obj *obj1, struct obj *obj2) {
	// Copy size field
	obj2->size = obj1->size;
	// Copy data contents
	obj2->data = (unsigned char*)malloc((obj2->size)*sizeof(unsigned char));
	for(int i=0; i<(obj2->size); i++) {
		obj2->data[i] = obj1->data[i];
	}

	return 0;
}

int free_obj(struct obj *object) {
	// Free data ptr
	free(object->data);
	// Free object
	free(object);

	return 0;
}

struct obj* get_defunct_ptr() {
	// We can free up to the current ack_counter
	if((recycle_counter/2)%BUFFER_SIZE != ((ack_counter/2)%BUFFER_SIZE)) {
		struct obj* ret = items[(recycle_counter/2)%BUFFER_SIZE];
		// return and null if the slot is valid (not null)
		if(ret) {
			items[(recycle_counter/2)%BUFFER_SIZE] = NULL;
			recycle_counter += 2;
			return ret;
		}
	}
	return NULL;
}

int insert_item(struct obj* ptr_to_item, struct obj** ptr_to_defunct_item)
{
  unsigned short temp_ac = ack_counter;
 
  if (last_update_counter - temp_ac == 2 * BUFFER_SIZE) {
    ptr_to_defunct_item = NULL;
    return BUFFER_FULL;
  }

  if (last_update_counter - temp_ac == (2 * BUFFER_SIZE) - 1) {
    ptr_to_defunct_item = NULL;
    return BUFFER_FULL_CONSUMER_READING;
  }

  update_counter = last_update_counter + 1;

  // Get the old pointer, so it can be freed
  *ptr_to_defunct_item = items[((last_update_counter / 2) % BUFFER_SIZE)];

	// Copy the obj passed to insert_item, and set the pointer
	// TODO: Make this copy into shared memory
	
	struct obj* new_item = (struct obj*)malloc(sizeof(struct obj));
	copy_obj(ptr_to_item, new_item);
  items[(last_update_counter / 2) % BUFFER_SIZE] = new_item;

  //printf("insert_item pointer %x -> %d\n", ptr_to_item, ptr_to_item);
  update_counter = last_update_counter + 2;

  // Also increment the recycle counter if it's in step w/ update_counter
  if(recycle_counter == last_update_counter) {
  	recycle_counter = update_counter;
  }

  last_update_counter = update_counter;

  return OK;
}

int read_item(struct obj* ref_to_item)
{
  unsigned short temp_uc = update_counter;

  if (temp_uc == last_ack_counter) {
    return BUFFER_EMPTY; 
  }

  if ((temp_uc - last_ack_counter) == 1) {
    return BUFFER_EMPTY_PRODUCER_INSERTING;
  }

  ack_counter = last_ack_counter + 1;

  // Copy out the value pointed to by the pointer in the NBB
  struct obj* temp_ptr = items[((last_ack_counter / 2) % BUFFER_SIZE)];
  copy_obj(temp_ptr, ref_to_item);

  // Freeing the read item is taken care of in the write method

  //printf("read_item ret pointer %x\n", ref_to_item);
  //printf("read_item ret pointer %x -> %d\n", ref_to_item, *ref_to_item);
  ack_counter = last_ack_counter + 2;
  last_ack_counter = ack_counter;

  return OK;
}

int read_asynch(struct obj* ptr_to_item) {
	return read_item(ptr_to_item);
}

int readb(struct obj* ptr_to_item) {
	int ret;
	do {
		ret = read_item(ptr_to_item);
	} while(ret != OK);
	return ret;
}

int write_asynch(struct obj* ptr_to_item) {
	struct obj *ptr_to_defunct_item = NULL;
	int ret;

	// Try only once, we're non-blocking
	ret = insert_item(ptr_to_item, &ptr_to_defunct_item);

	// Free defunct pointer if not NULL
  if(ret == OK && ptr_to_defunct_item) {
  	free_obj(ptr_to_defunct_item);
  }

  return ret;
}

int writeb(struct obj* ptr_to_item) {
	struct obj *ptr_to_defunct_item = NULL;
	int ret;

	// Spin until success
	do {
		ret = insert_item(ptr_to_item, &ptr_to_defunct_item);
	} while(ret != OK);

	// Free defunct pointer if not NULL
  if(ptr_to_defunct_item) {
  	free_obj(ptr_to_defunct_item);
  }

  return ret;
}

int clean_mem() {
  int ret = shm_unlink(SHARED_MEM_NAME);

  if (ret < 0) {
    perror("! shm_unlink\n");
    return ret;
  }

  return 0;
}
