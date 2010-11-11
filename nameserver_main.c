#include "nameserver.h"
#include "nbb.h"

/*
void data_available(int signum)
{
  char* recv;
  size_t recv_len;
  int retval = -1;
  int channel_id;

  channel_id = 0;

  retval = read_item(channel_id, (void*)&recv, &recv_len);
  printf("read: %s\n", recv);

  printf("recv: %s, after parsing: %s\n", recv, strtok(recv, " "));

  signal(SIGUSR1, data_available);
  //int type = strtok(recv, " ") - "0";
  //printf("type: %d\n", type);

  //handle_connection[type](strtok(NULL, " "));
}*/

int main() 
{
	if(init_nameserver()) {
		printf("Error initializing nameserver, failing!\n");
		return -1;
	}

  signal(SIGUSR1, data_available);

	while(1) {
    sleep(1);
	}
}
