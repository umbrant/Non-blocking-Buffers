#include "nbb.h"


// list of channel pointers (to shared memory)
struct channel channel_list[NUM_CHANNELS];

// Called by everyone
int init_service() 
{
	// need to get NUM_PAGES * 2 shm pages
	//
	// FIXME: we only alloc the first one, since the nameserver needs to
	// be assigning us keys in the long run.
	
	int shmid;
	unsigned char * shm;

	// Allocate 4 pages, 1 meta + 1 data for each buffer
	// Read buffer
	// note that we use SERVICE_TEST_WRITE, not READ, since the service's
	// read is the client's write
	if((shmid = shmget(SERVICE_TEST_WRITE, PAGE_SIZE*2, IPC_CREAT | 0666)) < 0) {
		perror("shmget");
		return -1;
	}
	if((shm = shmat(shmid, NULL, 0)) == (unsigned char*) -1) {
		perror("shmat");
		return -1;
	}
	// Make sure the memory is zero'd out
	memset(shm, 0, PAGE_SIZE*2);

	channel_list[0].read = (struct buffer*) shm;
	channel_list[0].read->data_size = PAGE_SIZE;
	channel_list[0].read->data_offset = PAGE_SIZE;
	channel_list[0].read_data = (unsigned char*) shm+PAGE_SIZE;

	// Write buffer. Same note as above about swapping read/write
	shmid = -1;
	shm = (unsigned char*) -1;
	if((shmid = shmget(SERVICE_TEST_READ, PAGE_SIZE*2, IPC_CREAT | 0666)) < 0) {
		perror("shmget");
		return -1;
	}
	if((shm = shmat(shmid, NULL, 0)) == (unsigned char*) -1) {
		perror("shmat");
		return -1;
	}
	// Make sure the memory is zero'd out
	memset(shm, 0, PAGE_SIZE*2);
	channel_list[0].write = (struct buffer*) (shm);
	channel_list[0].write->data_size = PAGE_SIZE;
	channel_list[0].write->data_offset = PAGE_SIZE;
	channel_list[0].write_data = (unsigned char*) shm+PAGE_SIZE;

	return 0;
}

// Called by clients connecting to a server
// Needs to map shm buffers into client's address space
int get_channel(int* channel_id, int service) {

	// FIXME: This needs some nameserver goodness too, not hardcoded
	if(service != SERVICE_TEST) {
		return -1;
	}
	int readbuf = SERVICE_TEST_READ;
	int writebuf = SERVICE_TEST_WRITE;

	int shmid;
	unsigned char * shm;

	// FIXME: right now this is hardcoded to use channel[0]. Need a more
	// dynamic way of finding a free slot and assigning it to that one,
	// and also tracking which are in use.
	int id = 0;
	*channel_id = id;

	// Get the read/write buffers for SERVICE_TEST already allocated by
	// the master
	if((shmid = shmget(readbuf, PAGE_SIZE*2, 0666)) < 0) {
		perror("shmget");
		return -1;
	}
	if((shm = shmat(shmid, NULL, 0)) == (unsigned char*) -1) {
		perror("shmat");
		return -1;
	}
	// Make sure the memory is zero'd out
	//memset(shm, 0, PAGE_SIZE*2);

	channel_list[id].read = (struct buffer*) shm;
	channel_list[id].read_data = (unsigned char*) shm+PAGE_SIZE;

	shmid = -1;
	shm = (unsigned char*) -1;
	// Write channel
	if((shmid = shmget(writebuf, PAGE_SIZE*2, 0666)) < 0) {
		perror("shmget");
		return -1;
	}
	if((shm = shmat(shmid, NULL, 0)) == (unsigned char*) -1) {
		perror("shmat");
		return -1;
	}
	channel_list[id].write = (struct buffer*) shm;
	channel_list[id].write_data = (unsigned char*) shm+PAGE_SIZE;
	// Make sure the memory is zero'd out
	//memset(shm, 0, PAGE_SIZE*2);

	return 0;
}

int insert_item(int channel_id, void* ptr_to_item, size_t size)
	//							struct obj** ptr_to_defunct_item)
{
	struct buffer *buf = channel_list[channel_id].write;
	unsigned char *data_buf = channel_list[channel_id].write_data;

  unsigned short temp_ac = buf->ack_counter;
 
  if (buf->last_update_counter - temp_ac == 2 * BUFFER_SIZE) {
  	//ptr_to_defunct_item = NULL;
    return BUFFER_FULL;
  }

  if (buf->last_update_counter - temp_ac == (2 * BUFFER_SIZE) - 1) {
    return BUFFER_FULL_CONSUMER_READING;
  }

  // Check if there is space in the data region for new item
  // This works by checking how far the previous item extends,
  // and then if our size will fit in the remaining space
  // 
  // If it doesn't fit at the end, check at the head of the list too.
  struct channel_item* prev_item = 
				&(buf->items[(((buf->last_update_counter/2)-1)%BUFFER_SIZE)]);

	int item_offset;

	if(buf->last_update_counter == 0) {
		item_offset = 0;
	}
	else if((prev_item->offset+prev_item->size+size) <  buf->data_size) {
		item_offset = prev_item->offset + prev_item->size;
	}
	// Check if there's space at the head of the list for our item instead
	// This is done by checking the offset of the 
	// oldest unread item (at the ack counter)
	else if(buf->items[((buf->last_ack_counter)/2)%BUFFER_SIZE].offset > size) {
		item_offset = 0;
	} 
	// Couldn't fit at the end or the beginning. Sad.
	else {
		printf("else...\n");
		printf("poff: %d psize: %d size: %zu bsize: %d\n", prev_item->offset,
					prev_item->size, size, buf->data_size);
		return BUFFER_FULL;
	}

	// Update our new item in items[], say that we're writing
  buf->update_counter = buf->last_update_counter + 1;

	// Copy the item into the buffer's shm data region at offset
	memcpy(data_buf+item_offset, ptr_to_item, size);

	// Set the offset based on our above calculations
	buf->items[((buf->last_update_counter/2)%BUFFER_SIZE)].offset = item_offset;
	buf->items[((buf->last_update_counter/2)%BUFFER_SIZE)].size = size;

  // Done writing
  buf->update_counter = buf->last_update_counter + 2;

  // Also increment the recycle counter if it's in step w/ update_counter
  //if(recycle_counter == last_update_counter) {
  //	recycle_counter = update_counter;
  //}

  buf->last_update_counter = buf->update_counter;

  return OK;
}

int read_item(int channel_id, void** ptr_to_item, size_t* size)
{
	struct buffer *buf = channel_list[channel_id].read;
	unsigned char *data_buf = channel_list[channel_id].read_data;
  unsigned short temp_uc = buf->update_counter;

  if (temp_uc == buf->last_ack_counter) {
    return BUFFER_EMPTY; 
  }

  if ((temp_uc - buf->last_ack_counter) == 1) {
    return BUFFER_EMPTY_PRODUCER_INSERTING;
  }

  buf->ack_counter = buf->last_ack_counter + 1;

  // Copy out the value to malloc'd mem in our address space
  struct channel_item* tmp = 
				&(buf->items[((buf->last_ack_counter / 2) % BUFFER_SIZE)]);
  *ptr_to_item = malloc(tmp->size);
  memcpy(*ptr_to_item, data_buf+tmp->offset, tmp->size);
	*size = tmp->size;

  //printf("read_item ret pointer %x\n", ref_to_item);
  //printf("read_item ret pointer %x -> %d\n", ref_to_item, *ref_to_item);
  buf->ack_counter = buf->last_ack_counter + 2;
  buf->last_ack_counter = buf->ack_counter;

  return OK;
}









// DEPRECATED BELOW



/*







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
*/
