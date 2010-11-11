#ifndef NAMESERVER_H
#define NAMESERVER_H

#define _XOPEN_SOURCE

#include "nbb.h"
#include "constants.h"

//TODO: Anything could be automated?
#define NUM_CONNECTION_TYPE 2 // Server & client only for now
#define NUM_SERVICES 5 // # of services this nameserver could hold
#define TOTAL_CHANNELS 50 //Total # of channels that could be opened

typedef struct service
{
  char* name;
  int* channel_ids;
  int* is_channel_busy;
  int num_channels;

  int pid; // So that it could be interrupted
  int is_use;
} service_t;

// Called by nameserver at initialization
// Invariant: nameserver should be the first one that's starting
int init_nameserver();

// Finds whether the nameserver is able to store any more services
int reserve_service_slot();

// Control jumps to this function when nameserver receives new data
// via signal
void data_available(int signum);

// Allocate new channel to a particular service
int reserve_channel(int slot);

// Deallocate the resources to the service
void dealloc_service(int slot);

// Find 
void handle_client(char* arg);

// Arg consists of the name of the service, the number of channels 
// it want to reserve & the service's pid
void handle_service(char* arg);

// Find the pointer to service with the name in the argument if it exist
int find_service(char* name);

// Find a free channel slot in the server
int bind_client_service(int service_id);

// Handle incoming requests (so far there's only service & client)
static void (*handle_connection[NUM_CONNECTION_TYPE])(char*) = 
{
  handle_service,
  handle_client,
};

#endif // NAMESERVER_H
