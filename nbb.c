#include "nbb.h"

//unsigned short ack_counter = 0;
//unsigned short last_ack_counter = 0;
//unsigned short update_counter = 0;
//unsigned short last_update_counter = 0;
//unsigned short recycle_counter = 0;
//struct obj* items[BUFFER_SIZE] = {0};

// Shared memory variables
//const char* SHARED_MEM_NAME = "nbb_v1"; // TODO: should not be hardcoded
//struct pollfd read_pollfd;
//int insert_fd = -1;

// list of channel pointers (to shared memory)
struct channel channel_list[NUM_CHANNELS];
// set the bool when corresponding channel is active
bool channel_active[NUM_CHANNELS];

// Called by everyone
int init_service() {
	// need to get NUM_PAGES * 2 shm pages
	//
	// FIXME: we only alloc the first one, since the nameserver needs to
	// be assigning us keys in the long run.
	int shmid;
	unsigned char * shm[4];

	for(int i=0; i<4; i++) {
		if(shmid = shmget(SERVICE_TEST+i, PAGE_SIZE, IPC_CREAT | 0666) < 0) {
			perror("shmget");
			exit(1);
		}
		if((shm[i] = shmat(shmid, NULL, 0)) == (unsigned char*) -1) {
			perror("shmat");
			exit(1);
		}
		// Make sure the page is zero'd out
		memset(shm[i], 0, PAGE_SIZE);
	}

	channel_list[0].read = (struct buffer*) shm[0];
	channel_list[0].read->data = (struct buffer*) shm[1];
	channel_list[0].write = (struct buffer*) shm[2];
	channel_list[0].write->data = (struct buffer*) shm[3];
}

// Called by clients connecting to a server
// Needs to map shm buffers into client's address space
int get_channel(int channel_id, int service) {
	// This needs some nameserver goodness too, not hardcoded
	if(service != SERVICE_TEST) {
		return -1;
	}

	int shmid;
	unsigned char * shm[2];

	// Get the read/write channels for SERVICE_TEST already allocated by
	// the master
	for(int i=0; i<2; i++) {
		if(shmid = shmget(SERVICE_TEST+(i*2), PAGE_SIZE, 0666) < 0) {
			perror("shmget");
			exit(1);
		}
		if((shm[i] = shmat(shmid, NULL, 0)) == (unsigned char*) -1) {
			perror("shmat");
			exit(1);
		}
	}

	channel_list[0].read = (struct buffer*) shm[0];
	channel_list[0].write = (struct buffer*) shm[1];

}

// Called by services

//int copy_channel_item(struct channel_item *obj1, struct channel_item *obj2) {
//	// Copy size field
//	obj2->size = obj1->size;
//	// Copy data contents
//	obj2->data = (unsigned char*)malloc((obj2->size)*sizeof(unsigned char));
//	for(int i=0; i<(obj2->size); i++) {
//		obj2->data[i] = obj1->data[i];
//	}
//
//	return 0;
//}
//
//int free_obj(struct obj *object) {
//	// Free data ptr
//	free(object->data);
//	// Free object
//	free(object);
//
//	return 0;
//}

// TODO: make this work again w/ new structs
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

int insert_item(int id, void* ptr_to_item, short size)
	//							struct obj** ptr_to_defunct_item)
{
	struct buffer *buf = channel_list[id].write;
  unsigned short temp_ac = buf->ack_counter;
 
  if (buf->last_update_counter - temp_ac == 2 * BUFFER_SIZE) {
    //ptr_to_defunct_item = NULL;
    return BUFFER_FULL;
  }


	// FIXME this requires some more thinking to handle overflow cases
	// TODO XXX XXX XXX 

  // Check if there is space in the data region for new item
  struct channel_item prev_item = 
				buf->items[((buf->last_update_counter/2)%BUFFER_SIZE)-1];
	// Can ignore 
	if(buf->last_update_counter != buf->last_ack_counter &&
				(prev_item.offset+prev_item.size+size) >  data_size) {
		return BUFFER_FULL;
	}

  if (buf->last_update_counter - temp_ac == (2 * BUFFER_SIZE) - 1) {
    //ptr_to_defunct_item = NULL;
    return BUFFER_FULL_CONSUMER_READING;
  }

  buf->update_counter = buf->last_update_counter + 1;

  // Get the old pointer, so it can be freed
  //*ptr_to_defunct_item = buf->objs[((last_update_counter / 2) % BUFFER_SIZE)];

	//struct obj* new_item = (struct obj*)malloc(sizeof(struct obj));
	//copy_obj(ptr_to_item, new_item);
  //items[(last_update_counter / 2) % BUFFER_SIZE] = new_item;

	// Construct objs appropriately and then copy into shared memory
	struct channel_item new_item;
	new_item.offset = -1; // XXX
	new_item.size = size;
	
	buf->items[(buf->last_update_counter/2)%BUFFER_SIZE] = new_item;

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
