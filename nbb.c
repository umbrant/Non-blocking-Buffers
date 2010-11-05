#include "nbb.h"

// list of channel pointers (to shared memory)
struct channel channel_list[SERVICE_MAX_CHANNELS] = {};
struct service_used services_used[SERVICE_MAX_CHANNELS] = {};

sem_t *sem_id;   // POSIX semaphore

int init_nameserver()
{
  FILE* pFile;

  // XXX: Initial semaphore value? Use 1 for now...
  sem_id = sem_open(SEM_KEY, O_CREAT | O_EXCL, 0666, 1);
  if(sem_id == SEM_FAILED) {
    perror("! Unable to obtain semaphore\n");
    return -1;
  }

  if(open_channel(NAMESERVER_READ, NAMESERVER_WRITE, IPC_CREAT)) {
    perror("! Unable to open chanel\n");
    return -1;
  }

  pFile = fopen(NAMESERVER_PID_FILE,"w+");
  fprintf(pFile,"%d",(int)getpid());
  fclose(pFile);

  return 0;
}

int init_service(int num_channels, char* name) 
{
  char request[50];
  FILE* pFile;
  int nameserver_pid = 0;
  char num_channel[2]; // TODO: Make it constants?
  char pid[5];

  //struct sembuf operations[1]; // Array of one operation for the semaphore

  sem_id = sem_open(SEM_KEY, 0);
  if(sem_id == SEM_FAILED) {
    perror("! Unable to obtain semaphore\n");
    return -1;
  }

  // BEGIN CRITICAL SECTION
  sem_wait(sem_id);

  //operations[0].sem_num = 0;
  //operations[0].sem_

  // Should be reversed since what's written by service is read by nameserver
  if(open_channel(NAMESERVER_WRITE, NAMESERVER_READ, !IPC_CREAT)) {
  	sem_post(sem_id);
    return -1;
  }

  sprintf(num_channel, "%d", num_channels);
  sprintf(pid, "%d", getpid());

  strcpy(request, SERVICE);
  strcat(request, " ");
  strcat(request, name);
  strcat(request, " ");
  strcat(request, num_channel); 
  strcat(request, " ");
  strcat(request, pid);

  pFile = fopen(NAMESERVER_PID_FILE, "r+"); 
  fscanf(pFile,"%d",&nameserver_pid); 
  fclose(pFile); 

  insert_item(0, request, strlen(request));

  kill(nameserver_pid, SIGUSR1);

  int retval = 1;
  char* recv; 
  size_t recv_len;

  do{ 
    retval = read_item(0, (void*)&recv, &recv_len);
  } while (retval == BUFFER_EMPTY || retval == BUFFER_EMPTY_PRODUCER_INSERTING);
  
  if(!strcmp(recv, NAMESERVER_CHANNEL_FULL)) {
    // TODO: anything else to do? 
    printf("! Reserving channel unsuccessful\n");
  
    sem_post(sem_id);
    return -1;
  }
  else {
    printf("** Acquired the following channels: %s\n", recv);

    int i;
    int channel;
    char* tmp;

    tmp = strtok(recv, " ");
    for(i = 0;i < num_channels;i++) { 
      channel = atoi(tmp);
      if(open_channel(channel, channel + READ_WRITE_CONV, IPC_CREAT) == -1) {
        //TODO: service_exit();
        printf("! Failed to open the %d-th channel\n", i);
        sem_post(sem_id);
        return -1;
      }
      tmp = strtok(NULL, " ");
    }

    signal(SIGUSR1, recv_client_data);

    sem_post(sem_id);
    return 0;
  }
  // END CRITICAL SECTION
}


// Called by clients connecting to a server
// Needs to map shm buffers into client's address space
int connect_service(char* service_name) 
{
  char request[strlen(CLIENT) + strlen(service_name)];
  FILE* pFile;
  int nameserver_pid = 0;
  int ret_code;

  sem_id = sem_open(SEM_KEY, 0);
  if(sem_id == SEM_FAILED) {
    perror("! Unable to obtain semaphore\n");
    return -1;
  }
  // BEGIN CRITICAL SECTION
  sem_wait(sem_id);


  // Should be reversed since what's written by client is read by nameserver
  if(open_channel(NAMESERVER_WRITE, NAMESERVER_READ, !IPC_CREAT)) {
  	sem_post(sem_id);
    return -1;
  }

  // TODO: Reserve the nameserver with semaphore

  strcpy(request, CLIENT);
  strcat(request, " ");
  strcat(request, service_name);

  pFile = fopen(NAMESERVER_PID_FILE, "r+"); 
  fscanf(pFile,"%d",&nameserver_pid); 
  fclose(pFile); 

  insert_item(0, request, strlen(request));

  kill(nameserver_pid, SIGUSR1);

  int retval = 1;
  char* recv; 
  size_t recv_len;

  do{ 
    retval = read_item(0, (void*)&recv, &recv_len);
  } while (retval == BUFFER_EMPTY || retval == BUFFER_EMPTY_PRODUCER_INSERTING);

  // TODO: anything else to do? 
  if(!strcmp(recv, UNKNOWN_SERVICE)) {
    printf("** Invalid service\n");
    ret_code = -1;
  }
 
  else if(!strcmp(recv, SERVICE_BUSY)) {
    printf("** Service too busy, not enough channel\n"); 
    ret_code = -1;
  }

  else {
    char* tmp; 
    int slot;
    int channel_id;
    int service_pid;

    tmp = strtok(recv, " ");
    channel_id = atoi(tmp);
    tmp = strtok(NULL, " ");
    service_pid = atoi(tmp);

    slot = open_channel(channel_id + READ_WRITE_CONV, channel_id, !IPC_CREAT);

    services_used[slot].service_name = (char*)malloc(sizeof(char)*50);
    strcpy(services_used[slot].service_name, service_name);
    services_used[slot].pid = service_pid;
   
    ret_code = slot;
 
    printf("** Connecting to service successful, channel: %d service pid: %d\n", channel_id, service_pid);
  }

  // END CRITICAL SECTION
  sem_post(sem_id);

	return ret_code;
}

int client_send(char* service_name, char* msg)
{
  int i;
  char* new_msg = (char*)calloc(strlen(msg), sizeof(char));
  char* recv;
  size_t recv_len;
  int retval;

  // FIXME: Since i = 0 is already reserved for nameserver, should we change?
  for(i = 1;i < SERVICE_MAX_CHANNELS;i++) {
    if(channel_list[i].in_use && 
       !strcmp(service_name, services_used[i].service_name)) {
      break;
    }
  }

  if(i == SERVICE_MAX_CHANNELS) {
    perror("! Service not found\n");
    return -1;
  }

  strcpy(new_msg, msg); 
  insert_item(i, new_msg, strlen(new_msg)+1); 
  kill(services_used[i].pid, SIGUSR1);

  printf("** Send %s to %s\n", msg, service_name);

  do{ 
    retval = read_item(i, (void*)&recv, &recv_len);
  } while (retval == BUFFER_EMPTY || retval == BUFFER_EMPTY_PRODUCER_INSERTING);

  printf("** Received %s from the service\n", recv);


  return 0;
}

void recv_client_data()
{
  int i;
  char* recv;
  size_t recv_len = 0;
  int retval = -1;
  char* reply_msg = (char*)calloc(50,sizeof(char));

  // FIXME: Since i = 0 is already reserved for nameserver, should we change?
  for(i = 1;channel_list[i].in_use && i < SERVICE_MAX_CHANNELS;i++) {
    retval = read_item(i, (void*)&recv, &recv_len);
    recv[recv_len] = '\0';

    if(retval == OK) {
      printf("** Received %s from shm id %d\n", recv, channel_list[i].read_id);

      strcpy(reply_msg, "acknowledged the message: ");
      strcat(reply_msg, recv);

      insert_item(i, reply_msg, strlen(reply_msg));

      memset(recv, '\0',recv_len);
      recv_len = 0;
    }
  }

  free(reply_msg);

  signal(SIGUSR1, recv_client_data);
}

int open_channel(int shm_read_id, int shm_write_id, int is_ipc_create)
{
	int shmid;
	unsigned char * shm;
  int free_slot;

  free_slot = free_channel_slot();
  if(free_slot == -1) {
    return -1;
  }

	// Allocate 4 pages, 1 meta + 1 data for each buffer
	// Read buffer
	// note that we use SERVICE_TEST_WRITE, not READ, since the service's
	// read is the client's write
	if((shmid = shmget(shm_read_id, PAGE_SIZE*2, is_ipc_create | 0666)) < 0) {
		perror("shmget");
		return -1;
	}
	if((shm = shmat(shmid, NULL, 0)) == (unsigned char*) -1) {
		perror("shmat");
		return -1;
	}
	// Make sure the memory is zero'd out
	memset(shm, 0, PAGE_SIZE*2);

	channel_list[free_slot].read = (struct buffer*) shm;
	channel_list[free_slot].read->data_size = PAGE_SIZE;
	channel_list[free_slot].read->data_offset = PAGE_SIZE;
	channel_list[free_slot].read_data = (unsigned char*) shm+PAGE_SIZE;
  channel_list[free_slot].read_id = shm_read_id;

	// Write buffer. Same note as above about swapping read/write
	shmid = -1;
	shm = (unsigned char*) -1;
	if((shmid = shmget(shm_write_id, PAGE_SIZE*2, is_ipc_create | 0666)) < 0) {
		perror("shmget");
		return -1;
	}
	if((shm = shmat(shmid, NULL, 0)) == (unsigned char*) -1) {
		perror("shmat");
		return -1;
	}
	// Make sure the memory is zero'd out
	memset(shm, 0, PAGE_SIZE*2);

	channel_list[free_slot].write = (struct buffer*) (shm);
	channel_list[free_slot].write->data_size = PAGE_SIZE;
	channel_list[free_slot].write->data_offset = PAGE_SIZE;
	channel_list[free_slot].write_data = (unsigned char*) shm+PAGE_SIZE;
  channel_list[free_slot].write_id = shm_write_id;

  channel_list[free_slot].in_use = 1;

  return free_slot;
}

int close_channel(int index)
{
  //TODO: Most probably buggy, needs to be checked some more

  shmdt((char*)channel_list[index].read);
  if(shmctl(channel_list[index].read_id, IPC_RMID, 0) == -1) {
    return -1;
  }

  shmdt((char*)channel_list[index].write);
  if(shmctl(channel_list[index].write_id, IPC_RMID, 0) == -1) {
    return -1;
  }

  channel_list[index].in_use = 0;
  return 0;
}

int free_channel_slot()
{
  int i;

  for(i = 0;i < SERVICE_MAX_CHANNELS;i++) {
    if(!channel_list[i].in_use) {
      return i;
    }
  }

  return -1;
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
