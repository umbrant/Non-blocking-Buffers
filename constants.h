#ifndef CONSTANTS_H
#define CONSTANTS_H

// TODO: Set dynamically
#define SERVICE_MAX_CHANNELS 10 // Total # of channels / service
#define CLIENT_MAX_CHANNELS 5 // Total # of channels / client
#define MAX_MSG_LEN 50
#define READ_WRITE_CONV 1000 // Read id always differ by 1000 from write id 
#define SEM_KEY "/1337" // POSIX semaphore identifier to be used by everyone

enum {
	SERVICE_TEST = 3000,
	SERVICE_TEST_READ = 3000,
	SERVICE_TEST_WRITE = 3002,
  NAMESERVER_WRITE = 4000,
  NAMESERVER_READ = 4001, 
};

enum {
  CREATE = 0,
  OPEN,
};

static const char NAMESERVER_PID_FILE[] = "nameserver_pid";
static const char NAMESERVER_CHANNEL_FULL[] = "Nameserver Full";
static const char UNKNOWN_SERVICE[] = "Unknown Service";
static const char SERVICE_BUSY[] = "Service Too Busy";

static const char SERVICE[] = "0";
static const char CLIENT[] = "1";
static const char GUI[] = "GUI";

#endif // CONSTANTS_H
