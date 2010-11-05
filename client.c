#include "nbb.h"

int main() 
{
	//int channel_id;
  char* service_name = (char*)malloc(sizeof(char)*50);

  strcpy(service_name, GUI);

	if(connect_service(service_name) < 0) {
		printf("Error getting channel!\n");
		return -1;
	}

	//char* service_name = (char*)malloc(50*sizeof(char));
	char* msg = (char*)malloc(50*sizeof(char));

  while(1) {
/*
    printf("Enter server name: ");
    scanf("%s", service_name);
*/

    printf("Enter message to send to server: ");
    scanf("%s", msg);

    client_send(service_name, msg); 
  }
}
