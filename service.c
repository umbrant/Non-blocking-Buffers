#include "nbb.h"

#define NUM_CHANNELS 5 

int is_client = 0;

static void on_new_connection(int slot_id, void *arg)
{
  printf("GUI got new connection on slot %d\n", slot_id);
  is_client++;
}

static void on_new_conn_other(int slot_id, void *arg)
{
  printf("GUI2 got new connection on slot %d\n", slot_id);
  is_client++;
}

int main() 
{
  char* service_name = (char*)malloc(sizeof(char)*50);
  char* service_name_other = (char*)malloc(sizeof(char)*50);

  strcpy(service_name, GUI);
  strcpy(service_name_other, "GUI2");

	if(nbb_init_service(NUM_CHANNELS, service_name)) {
		printf("Error initializing as service, failing!\n");
		return -1;
	}

  sleep(1);

  if(nbb_init_service(NUM_CHANNELS, service_name_other)) {
 		printf("Error initializing as service, failing!\n");
		return -1;
	}

  nbb_set_cb_new_connection(service_name, on_new_connection, NULL);
  nbb_set_cb_new_connection(service_name_other, on_new_conn_other, NULL);

  int counter = 0;

  while(1) {
    sleep(1);

/*
    if(is_client == 2 && !(counter % 5)) {
      nbb_send("Client", "Test message", sizeof("Test message"));
      nbb_send("Client2", "Test message", sizeof("Test message"));
    }*/
    counter++;
  }
}
