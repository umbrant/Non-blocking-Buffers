#include "nbb.h"

int main() {
	if(init_service()) {
		printf("Error initializing as service, failing!\n");
		return -1;
	}

	int channel_id;
	channel_id = 0; // This needs to be set dynamically in the future

	char * send = (char*)malloc(50*sizeof(char));

	int counter = 0;
	while(1) {
		char * recv;
		size_t recv_len;
		int retval = -1;
		while(retval) {
			retval = read_item(channel_id, (void*)&recv, &recv_len);
			//printf("read: %d\n", retval);
			//sleep(1);
		}
		//printf("Recieved message of len %zu: ", recv_len);
		/*
		while(recv_len) {
			putchar(*recv);
			recv++;
			recv_len--;
		}
		putchar('\n');
		*/

		
		size_t send_len;
		send_len = sprintf(send, "Pong: %d", counter);

		retval = -1;
		while(retval) {
			retval = insert_item(channel_id, send, send_len);
			//printf("insert: %d\n", retval);
			//sleep(1);
		}
		//printf("Sent message of len %zu\n", send_len);

		counter++;
	}

}
