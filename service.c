#include "nbb.h"

#define NUM_CHANNELS 5

static void on_new_connection(int slot_id)
{
    printf("Service got new connection on slot %d\n", slot_id);
}

int main() 
{
  char* service_name = (char*)malloc(sizeof(char)*50);

  strcpy(service_name, GUI);
  nbb_set_cb_new_connection(on_new_connection);

	if(nbb_init_service(NUM_CHANNELS, service_name)) {
		printf("Error initializing as service, failing!\n");
		return -1;
	}

	while(1) {
    sleep(1);
  }
}
