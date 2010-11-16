#include "nbb.h"

int main() 
{
  char* service_name = (char*)malloc(sizeof(char)*50);
  char* service_name_other = (char*)malloc(sizeof(char)*50);
	char* msg = (char*)malloc(50*sizeof(char));
  int size;
  int option;
  char* array = (char*)malloc(sizeof(char));

  strcpy(service_name, GUI);
  strcpy(service_name_other, "GUI2");

	if(nbb_connect_service(service_name) < 0) {
		printf("Error getting channel!\n");
		return -1;
	}

  if(nbb_connect_service(service_name_other) < 0) {
		printf("Error getting channel!\n");
		return -1;
	}

  printf("Available server: (1) GUI (2) GUI2\n");

  while(1) {
    printf("Choose a server: ");
    scanf("%d", &option);

    printf("Enter message to send to server: ");
    scanf("%s", msg);

    if(option == 1) nbb_client_send(service_name, msg, strlen(msg)); 
    if(option == 2) nbb_client_send(service_name_other, msg, strlen(msg));

    printf("bytes available: %d\n", nbb_bytes_available(option));
    printf("How many bytes do you want to read: ");
    scanf("%d", &size);

    if(size == -1) {
      break;
    }

    if(size > nbb_bytes_available(option)) {
      continue;
    }

    memset(array, '\0', strlen(array));
    array = (char*)realloc(array, sizeof(char) * size);
    nbb_read_bytes(option, array, size);

    printf("read bytes: %s\n", array);
  }

  printf("\n********Statistic********\n");
  printf("bytes_available: %d\n", nbb_bytes_available(1));
  printf("bytes_read: %d\n", nbb_bytes_read(1));
  printf("bytes_written: %d\n", nbb_bytes_written(1));

  free(array);
  free(msg);
  free(service_name);
}
