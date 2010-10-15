#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

void sighup(); /* routines child will call upon sigtrap */
void sigusr();
void sigquit();

int main()
{ 
  int pid;

  /* get child process */
  
   if ((pid = fork()) < 0) {
        perror("fork");
        exit(1);
    }
    
   if (pid == 0)
     { /* child */
       signal(SIGHUP,sighup); /* set function calls */
       signal(SIGUSR1,sigusr);
       signal(SIGQUIT, sigquit);
       for(;;); /* loop for ever */
     }
  else /* parent */
     {  /* pid hold id of child */
       printf("\nPARENT: sending SIGHUP\n\n");
       kill(pid,SIGHUP);
       sleep(3); /* pause for 3 secs */
       printf("\nPARENT: sending user defined signal\n\n");
       kill(pid,SIGUSR1);
       sleep(3); /* pause for 3 secs */
       printf("\nPARENT: sending SIGQUIT\n\n");
       kill(pid,SIGQUIT);
       sleep(3);
     }
}

void sighup()
{  
  signal(SIGHUP,sighup); /* reset signal */
  printf("CHILD: I have received a SIGHUP\n");
}

void sigusr()
{ 
  signal(SIGUSR1, sigusr); /* reset signal */
  printf("CHILD: I have received a user defined signal\n");
}

void sigquit()
{
  printf("My DADDY has Killed me!!!\n");
  exit(0);
}
