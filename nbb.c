#include "nbb.h"

// list of channel pointers (to shared memory)
struct channel channel_list[NUM_CHANNELS];

// Called by nameserver at initialization
// Invariant: nameserver should be the first one that's starting
int init_nameserver()
{
  if(open_channel(NAMESERVER_READ, NAMESERVER_WRITE)) {
    return -1;
  }

  FILE* pFile;

  pFile = fopen(nameserver_pid_file,"w+");
  fprintf(pFile,"%d",(int)getpid());
  fclose(pFile);

  return 0;
}

// Called by every service at initialization
int init_service() 
{
  // Should be reversed since what's written by service is read by nameserver
  if(open_channel(NAMESERVER_WRITE, NAMESERVER_READ)) {
    return -1;
  }

  FILE* pFile;
  int nameserver_pid = 0;

  pFile = fopen(nameserver_pid_file, "r+"); 
  fscanf(pFile,"%d",&nameserver_pid); 
  fclose(pFile); 

  // TODO: Reserve the nameserver

  char request[strlen(service) + strlen(gui)];
  strcat(request, service);
  strcat(request, " \0");
  strcat(request, gui);

  insert_item(0, request, strlen(request));

  printf("pid: %d\n", nameserver_pid);
  kill(nameserver_pid, SIGUSR1);

  // Release the nameserver global channel
/*  pthread_mutex_lock(&nameserver_mutex); */
  int retval;
  char* recv;
  size_t recv_len;

  while(retval) {
    retval = read_item(0, (void*)&recv, &recv_len);
    printf("recv: %s\n", recv);

    sleep(1);
  }

/*
  pthread_cond_signal(&nameserver_mutex);
  pthread_mutex_unlock(&nameserver_mutex);*/

  return 0;
}


// Called by clients connecting to a server
// Needs to map shm buffers into client's address space
int get_channel(int* channel_id, int service) {

  // Should be reversed since what's written by service is read by nameserver
  if(open_channel(NAMESERVER_WRITE, NAMESERVER_READ)) {
    return -1;
  }

  FILE* pFile;
  int nameserver_pid;

  pFile = fopen(nameserver_pid_file,"r+"); 
  fscanf(pFile,"%d",&nameserver_pid); 
  fclose(pFile); 

  // Reserve the nameserver
/*
  pthread_mutex_lock(&nameserver_mutex);
  while(nameserver_state != EMPTY)
  {
    pthread_cond_wait(&nameserver_cond, &nameserver_mutex);
  }
  nameserver_state = SERVER_WRITING;
  pthread_mutex_unlock(&nameserver_mutex);
*/

  char request[strlen(client) + strlen(gui)];
  strcat(request, client);
  strcat(request, " \0");
  strcat(request, gui);

  insert_item(0, request, strlen(request));
  kill(nameserver_pid, SIGUSR1);

  // Release the nameserver global channel
/*  pthread_mutex_lock(&nameserver_mutex); */
  int retval;
  char* recv;
  size_t recv_len;

  while(retval) {
    retval = read_item(0, (void*)&recv, &recv_len);
    printf("recv: %s\n", recv);

    sleep(1);
  }

/*
  pthread_cond_signal(&nameserver_mutex);
  pthread_mutex_unlock(&nameserver_mutex);*/


	// FIXME: This needs some nameserver goodness too, not hardcoded
	if(service != SERVICE_TEST) {
		return -1;
	}
  
  /*
	int readbuf = SERVICE_TEST_READ;
	int writebuf = SERVICE_TEST_WRITE;

	int shmid;
	unsigned char * shm;

  // TODO: get free_channel()
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
*/
	return 0;
}

int open_channel(int shm_read_id, int shm_write_id)
{
  static int count = 0;
	int shmid;
	unsigned char * shm;

	// Allocate 4 pages, 1 meta + 1 data for each buffer
	// Read buffer
	// note that we use SERVICE_TEST_WRITE, not READ, since the service's
	// read is the client's write
	if((shmid = shmget(shm_read_id, PAGE_SIZE*2, IPC_CREAT | 0666)) < 0) {
		perror("shmget");
		return -1;
	}
	if((shm = shmat(shmid, NULL, 0)) == (unsigned char*) -1) {
		perror("shmat");
		return -1;
	}
	// Make sure the memory is zero'd out
	memset(shm, 0, PAGE_SIZE*2);

  // TODO: get free_channel()

	channel_list[count].read = (struct buffer*) shm;
	channel_list[count].read->data_size = PAGE_SIZE;
	channel_list[count].read->data_offset = PAGE_SIZE;
	channel_list[count].read_data = (unsigned char*) shm+PAGE_SIZE;

	// Write buffer. Same note as above about swapping read/write
	shmid = -1;
	shm = (unsigned char*) -1;
	if((shmid = shmget(shm_write_id, PAGE_SIZE*2, IPC_CREAT | 0666)) < 0) {
		perror("shmget");
		return -1;
	}
	if((shm = shmat(shmid, NULL, 0)) == (unsigned char*) -1) {
		perror("shmat");
		return -1;
	}
	// Make sure the memory is zero'd out
	memset(shm, 0, PAGE_SIZE*2);

	channel_list[count].write = (struct buffer*) (shm);
	channel_list[count].write->data_size = PAGE_SIZE;
	channel_list[count].write->data_offset = PAGE_SIZE;
	channel_list[count].write_data = (unsigned char*) shm+PAGE_SIZE;

  count++;

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
