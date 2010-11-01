#include "nbb.h"

#define NUM_CHANNELS 5

int main() 
{
  char* service_name = (char*)malloc(sizeof(char)*50);
  strcpy(service_name, GUI);

	if(init_service(NUM_CHANNELS, service_name)) {
		printf("Error initializing as service, failing!\n");
		return -1;
	}

	while(1) {
    sleep(1);
  }
}
