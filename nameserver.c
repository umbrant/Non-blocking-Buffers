#include "nameserver.h"

#define CHANNEL_ID 0

service_t service_lists[NUM_SERVICES] = {}; 
int free_lists[TOTAL_CHANNELS] = {};

void data_available(int signum)
{
	char* recv;
  size_t recv_len;
	int retval = -1;
  int request_type;

	retval = nbb_read_item(CHANNEL_ID, (void**)&recv, &recv_len);

	// This is kind of a hack
	// Do an extra resize and copy to null terminate
	char* recv_str = (char*)calloc(recv_len+1, sizeof(char));
	memcpy(recv_str, recv, recv_len);
	recv_str[recv_len] = '\0';
 
printf("nameserver recv: %s\n", recv_str);

  request_type = atoi(strtok(recv_str, " "));
  handle_connection[request_type](strtok(NULL, ""));

  signal(SIGUSR1, data_available);
  free(recv);
  free(recv_str);
}

//TODO: arg is somehow messed up on first connection, ><
void handle_client(char* arg)
{
  printf("\n** Handling client\n");

  char* msg = (char*)malloc(50*sizeof(char));
  int service_id;
  int channel_id; 

  service_id = find_service(arg);

  if(service_id == -1) {
    printf("** Unable to find service: %s\n", arg);
    strcpy(msg, UNKNOWN_SERVICE);

    nbb_insert_item(CHANNEL_ID, msg, strlen(msg));
    free(msg);
    return;
  }

  printf("** Able to find service: %s\n", arg);

  channel_id = bind_client_service(service_id);
  if(channel_id == -1) {
    printf("** Service has no channel free\n");
    strcpy(msg, SERVICE_BUSY);

    nbb_insert_item(CHANNEL_ID, msg, strlen(msg));
    free(msg);
    return;
  }

  printf("** Service could accept connection\n");

  char tmp[2]; 
  sprintf(tmp, "%d", service_lists[service_id].pid); 

  sprintf(msg, "%d", channel_id);
  strcat(msg, " "); 
  strcat(msg, tmp);

  nbb_insert_item(CHANNEL_ID, msg, strlen(msg));
  free(msg);
  return;
}

void handle_service(char* arg)
{
  printf("** Handling service\n");

  char* service_name = strtok(arg, " ");
  int num_channels = atoi(strtok(NULL, " "));
  int service_pid = atoi(strtok(NULL, " "));
  int slot;
  char* msg = (char*)calloc(50,sizeof(char));
  int i;

  slot = reserve_service_slot();

//printf("test\n");
  if(slot == -1) {
    return;
  }

  service_lists[slot].name = (char*)malloc(strlen(service_name));
  strcpy(service_lists[slot].name, service_name);
  service_lists[slot].num_channels = num_channels;
  service_lists[slot].channel_ids = (int*)malloc(sizeof(int)*num_channels);
  service_lists[slot].is_channel_busy = (int*)calloc(num_channels,sizeof(int));
  service_lists[slot].pid = service_pid;

  printf("** Name: %s, num_channel:%d, pid: %d\n", service_lists[slot].name, num_channels, service_pid);

  if(reserve_channel(slot)) { 
    printf("** Unable to reserve %d channels\n", num_channels);
    strcpy(msg, NAMESERVER_CHANNEL_FULL);
  }
  else {
    printf("** Able to reserve %d channels\n", num_channels);
    for(i = 0;i < service_lists[slot].num_channels;i++) {
      char tmp[5] = "\0"; 
      sprintf(tmp, "%d", service_lists[slot].channel_ids[i]);
      strcat(msg, tmp);
      strcat(msg, " "); 
    }
  }

  nbb_insert_item(CHANNEL_ID, msg, strlen(msg));
  free(msg); 
  return;
}

int find_service(char* name)
{
  int i;

  for(i = 0;i < NUM_SERVICES;i++) {
    if(service_lists[i].is_use && !strcmp(service_lists[i].name, name)) {
      return i;
    }
  }

  return -1;
}

int bind_client_service(int service_id)
{
  int i;

  for(i = 0;i < service_lists[service_id].num_channels;i++) {
    if(!service_lists[service_id].is_channel_busy[i]) {
      service_lists[service_id].is_channel_busy[i] = 1;
      return service_lists[service_id].channel_ids[i];
    }
  }

  return -1;
}

int reserve_channel(int slot)
{
  int i;
  int count = 0;

  for(i = 0;i < TOTAL_CHANNELS;i++) {
    if(free_lists[i] == 0) {
      free_lists[i] = 1;
      service_lists[slot].channel_ids[count] = i + 1; // Since nameserver is bind to id 0 
      count++;
    }

    if(count == service_lists[slot].num_channels) {
      break;
    }
  }
  
  // Couldn't reserve all of the channels requested
  if(count != service_lists[slot].num_channels) {
    dealloc_service(slot);
    return -1;
  }

  return 0;
}

int reserve_service_slot()
{
  int i;

  for(i = 0;i < NUM_SERVICES;i++) {
    if(!service_lists[i].is_use) {
      service_lists[i].is_use = 1;
      return i;
    }
  }

  return -1;
}

void dealloc_service(int slot)
{
  int i;

  for(i = 0;i < service_lists[slot].num_channels;i++) {
    free_lists[service_lists[slot].channel_ids[i]] = 0;
  }

  service_lists[slot].is_use = 0;  
  free(service_lists[slot].name);
  free(service_lists[slot].channel_ids);
}
