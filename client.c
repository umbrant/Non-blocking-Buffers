#include "nbb.h"

int main() 
{
  char* service_name = (char*)malloc(sizeof(char)*50);
	char* msg = (char*)malloc(50*sizeof(char));
  int size;
  char* array = (char*)malloc(sizeof(char));

  strcpy(service_name, GUI);

	if(connect_service(service_name) < 0) {
		printf("Error getting channel!\n");
		return -1;
	}

  while(1) {
    printf("Enter message to send to server: ");
    scanf("%s", msg);

    client_send(service_name, msg); 

    printf("bytes available: %d\n", bytes_available(1)); //hardcoded, xD
    printf("How many bytes do you want to read: ");
    scanf("%d", &size);

    if(size == -1) {
      break;
    }

    if(size > bytes_available(1)) {
      continue;
    }

    memset(array, '\0', strlen(array));
    array = (char*)realloc(array, sizeof(char) * size);
    read_bytes(1, size, array);

    printf("read bytes: %s\n", array);
  }

  printf("\n********Statistic********\n");
  printf("bytes_available: %d\n", bytes_available(1));
  printf("bytes_read: %d\n", bytes_read(1));
  printf("bytes_written: %d\n", bytes_written(1));

  free(array);
  free(msg);
  free(service_name);
}
