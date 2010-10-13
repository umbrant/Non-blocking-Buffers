#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h> 
#include <poll.h>

#define MAX_MSG_LEN 1024

enum {
  SYNCH,
  ASYNCH
};

typedef struct data {
  size_t size;
  char* data;
} data_t;

const char *shared_mem_name = "shm_tesselation";
int reader_fd, writer_fd;
int ret;
struct pollfd read_pollfd;

int init_write() 
{
  writer_fd = shm_open(shared_mem_name, O_WRONLY | O_CREAT | O_NONBLOCK, 0666);
  if (writer_fd < 0) {
    perror("shm_open");
    return -1;
  }
  return 1;
}

int init_read() 
{
  reader_fd = shm_open(shared_mem_name, O_RDONLY | O_TRUNC, 0666);
  if (reader_fd < 0) {
    perror("shm_open");
    return -1;
  }

  memset(&read_pollfd, 0, sizeof(struct pollfd));
  read_pollfd.fd = reader_fd;
  read_pollfd.events = POLLIN;

  return 1;
}

int write_asynch(data_t* item) 
{
  return write(writer_fd, &(item->data), sizeof(char*));
}

int write_synch(data_t* item) {
  do {
    ret = write(writer_fd, &(item->data), sizeof(char*));
  } while (ret < 0);

  return ret;
}

int read_data(data_t* item, int mode) 
{

SYNCH_LABEL:
  ret = read(read_pollfd.fd, &(item->data), sizeof(char*));
  if (ret < 0) {
    perror("read");
    return -1;
  } else if (ret == 0) {
    if (mode == SYNCH) goto SYNCH_LABEL; // No data is available yet
    return -1;
  }
  return ret;
}

int read_asynch(data_t* item) 
{
  ret = poll(&read_pollfd, 1, 0);
  if (ret < 0) {
    perror("poll");
    return -1;
  }

  return read_data(item, ASYNCH);
}

int read_synch(data_t* item) {
  ret = poll(&read_pollfd, 1, -1);
  if (ret < 0) {
    perror("poll");
    return -1;
  }

  return read_data(item, SYNCH);
}

int clean_mem() 
{
  ret = shm_unlink(shared_mem_name);
  if (ret < 0) {
    perror("shm_unlink");
    return -1;
  }

  return 1;
}
void* read_items(void* ptr) {
  void* buf = malloc(MAX_MSG_LEN);
  data_t* item = (data_t*)malloc(sizeof(data_t));

  init_read();
  item->data = buf;

  printf("Waiting to receive message...\n");
  while (1) {
    read_synch(item);
    printf("buffer read: %s\n", (char*)item->data);
  }
}

void* write_items(void* ptr) 
{
  const char writer_msg[] = "Incoming message";
  data_t* item = (data_t*)malloc(sizeof(data_t));
  int len = strlen(writer_msg);

  item->data = (char*)malloc(len * sizeof(char));
  strcpy(item->data, writer_msg);
  item->size = len;

  init_write();

  printf("Press Enter to send message\n");
  while(1) {
    getchar();
    write_asynch(item);
  }
}

int main() 
{
  pthread_t read_thread, write_thread;
  char* read = "read";
  char* write = "write";
  void** value_ptr = 0;

  pthread_create(&read_thread, NULL, read_items, (void*)read);
  pthread_create(&write_thread, NULL, write_items, (void*)write);

  pthread_join(read_thread, value_ptr);
  pthread_join(write_thread, value_ptr);
 
  clean_mem();

  return 0;
}
