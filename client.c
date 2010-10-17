#include "nbb.h"

int main() {
	int channel_id;
	if(get_channel(&channel_id, SERVICE_TEST) < 0) {
		printf("Error getting channel!\n");
		return -1;
	}

	char * send = (char*)malloc(50*sizeof(char));

	int counter = 0;
	while(counter < 1000000) {
		// Ping pong! We start, since the server needs to know we've connected.
		// Ideally, the server would serve though. Need some kind of
		// notification method for setting a channel active

		int retval = -1;

		size_t send_len;
		send_len = sprintf(send, "Ping: %d", counter);
		while(retval) {
			retval = insert_item(channel_id, send, send_len);
			//printf("insert: %d\n", retval);
			//sleep(1);
		}
		//printf("Sent message of len %zu\n", send_len);

		char * recv;
		size_t recv_len;
		retval = -1;
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

		counter++;
	}
}
