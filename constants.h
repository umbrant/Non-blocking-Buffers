#ifndef CONSTANTS_H
#define CONSTANTS_H

enum {
	SERVICE_TEST = 1000,
	SERVICE_TEST_READ = 1000,
	SERVICE_TEST_WRITE = 1002,
  NAMESERVER_WRITE = 2000,
  NAMESERVER_READ = 2001, 
};

static const char nameserver_pid_file[] = "nameserver_pid";

static const char service[] = "1";
static const char client[] = "2";
static const char gui[] = "GUI";

#endif // CONSTANTS_H
