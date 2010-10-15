#include "nbb.h"
#include "nameserver.h"

int main() {
	init();
	int channel_id = 0;
	if(get_channel(channel_id, SERVICE_TEST) == -1) {
		printf("Error getting channel!\n");
		exit(-1);
	}
}
