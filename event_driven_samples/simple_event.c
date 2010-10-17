#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <setjmp.h>
#include <assert.h>

#if DEBUG
  #define printf(...) printf(__VA_ARGS__)
#else
  #define printf(...)
#endif /* DEBUG */

int channel;
int retval;
jmp_buf place;

void item_available();

int main()
{ 
  int child_pid;

  if ((child_pid = fork()) < 0) {
    perror("fork");
    exit(1);
  }

  // "Consumer" 
  if (child_pid == 0) {
    signal(SIGUSR1, item_available);
    retval = setjmp(place);

    printf("Consumer: current pid %d, parent id %d\n", getpid(), getppid());

    if(!retval) {
      printf("No item yet\n");
    }
    else {
      printf("Item is available\n");
      /* Do stuff */
    }
    /* Do stuff */
    for(;;);    
  }

  // "Producer" 
  else { 
    sleep(1);
    printf("Producer: current pid %d, parent id %d\n", getpid(), getppid());
    printf("Child pid: %d\n", child_pid);
    kill(child_pid,SIGUSR1);
    sleep(3);
  }
}

void item_available()
{ 
  printf("item_available() pid: %d, parent's pid: %d\n", getpid(), getppid());
  signal(SIGUSR1, item_available); 

  /* Checks which channel where the item is available */

  longjmp(place, 1);
  assert(0); /* Shouldn't reach here */
}
