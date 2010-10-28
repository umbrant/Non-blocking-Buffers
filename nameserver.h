#ifndef NAMESERVER_H
#define NAMESERVER_H

// Necessary for sys/ipc.h
#define _XOPEN_SOURCE

#include <pthread.h>

#include "constants.h"
#include "nbb.h"

#define NUM_CONNECTION_TYPE 2 // Server & client only for now

static pthread_mutex_t nameserver_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t nameserver_cond  = PTHREAD_COND_INITIALIZER;

//void data_available();

void handle_client(char* arg);
void handle_service(char* arg);

// Handle incoming requests (so far there's only service & client)
static void (*handle_connection[NUM_CONNECTION_TYPE])(char*) = 
{
  handle_service,
  handle_client,
};

#endif // NAMESERVER_H
