#include "nameserver.h"

/*
void data_available()
{
	char* recv;
  size_t recv_len;
	int retval = -1;
	int channel_id;

	channel_id = 0; 

  signal(SIGUSR1, data_available);

	retval = read_item(channel_id, (void*)&recv, &recv_len);
  printf("read: %s\n", recv);
  
  printf("recv: %s, after parsing: %s\n", recv, strtok(recv, " "));

  //int type = strtok(recv, " ") - "0";
  //printf("type: %d\n", type);

  //handle_connection[type](strtok(NULL, " "));
}*/

void handle_client(char* arg)
{
  printf("**Handling client\n");
}

void handle_service(char* arg)
{
  printf("**Handling service\n");
}
