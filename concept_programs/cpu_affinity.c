#define _GNU_SOURCE
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#define handle_error_en(en, msg) \
  do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)

int main(int argc, char *argv[])
{
  int s, j;
  cpu_set_t cpuset;
  pthread_t thread;

  thread = pthread_self();

  /* Set affinity mask to include CPUs 0 to 7 */
  CPU_ZERO(&cpuset);
  for (j = 0; j < 1; j++)
    CPU_SET(j, &cpuset);

  s = pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset);
  if (s != 0)
    handle_error_en(s, "pthread_setaffinity_np");

  /* Check the actual affinity mask assigned to the thread */
  s = pthread_getaffinity_np(thread, sizeof(cpu_set_t), &cpuset);
  if (s != 0)
    handle_error_en(s, "pthread_getaffinity_np");

  printf("Set returned by pthread_getaffinity_np() contained:\n");
  printf("CPU_SETSIZE: %d\n", CPU_SETSIZE);

  for (j = 0; j < CPU_SETSIZE; j++)
    if (CPU_ISSET(j, &cpuset))
      printf("    CPU %d\n", j);
  exit(EXIT_SUCCESS);
}
