#include "nbb.h"
#include <assert.h>

int main() 
{
  char* service_name = (char*)malloc(sizeof(char)*50);
  char* service_name_other = (char*)malloc(sizeof(char)*50);

  char* client_name = (char*)malloc(sizeof(char)*50);
  char* client_name_other = (char*)malloc(sizeof(char)*50);

	char* msg = (char*)malloc(50*sizeof(char));
  int msg_len;
  int size;
  int option;
  char* array = (char*)malloc(sizeof(char) * MAX_MSG_LEN);
  int array_size = MAX_MSG_LEN;

  strcpy(service_name, GUI);
  strcpy(service_name_other, "GUI2");

  strcpy(client_name, "Client");
  strcpy(client_name_other, "Client2");

	if(nbb_connect_service(client_name, service_name) < 0) {
		printf("Error getting channel!\n");
		return -1;
	}

  if(nbb_connect_service(client_name_other, service_name_other) < 0) {
		printf("Error getting channel!\n");
		return -1;
	}

  printf("Available server: (1) GUI (2) GUI2\n");

  while(1) {
    printf("Choose a server: ");
    scanf("%d", &option);

    printf("Enter message to send to server: ");
    scanf("%s", msg);

    msg_len = strlen(msg);
    printf("User entered '%s' (%d bytes)\n", msg, msg_len);

    if(option == 1) nbb_send(service_name, msg, msg_len); 
    if(option == 2) nbb_send(service_name_other, msg, msg_len);
    else continue;

    printf("bytes available: %d\n", nbb_bytes_available(option));
    printf("How many bytes do you want to read: ");
    scanf("%d", &size);

    if(size == -1) {
      break;
    }

    if(size > nbb_bytes_available(option)) {
      continue;
    }

    if (size >= array_size) {
      // Grow the array size and ignore old contents (no need for realloc)
      free(array);
      array = (char *) malloc(sizeof(char) * 2 * array_size);
      assert(array != NULL);
      array_size = 2 * array_size;
    }

    nbb_read_bytes(option, array, size);

    // array might be not null-terminated
    printf("read %d bytes: '%.*s'\n", size, size, array);
  }

  printf("\n********Statistic********\n");
  printf("bytes_available: %d\n", nbb_bytes_available(1));
  printf("bytes_read: %d\n", nbb_bytes_read(1));
  printf("bytes_written: %d\n", nbb_bytes_written(1));

  free(array);
  free(msg);
  free(service_name);
}
