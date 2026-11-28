#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

static void
worker(int nice_val, int duration_ticks)
{
  int pid = getpid();
  if (setnice(pid, nice_val) < 0) {
    printf("cfstest: setnice(%d, %d) failed\n", pid, nice_val);
    exit(1);
  }

  uint start = uptime();
  volatile uint64 counter = 0;
  while ((int)(uptime() - start) < duration_ticks) {
    counter++;
  }

  printf("pid=%d nice=%d counter=%lu\n", pid, nice_val, counter);
  exit(0);
}

int
main(int argc, char *argv[])
{
  int nices[] = {-5, 0, 5};
  int n = sizeof(nices) / sizeof(nices[0]);
  int duration = 60;

  if (argc > 1) {
    duration = atoi(argv[1]);
    if (duration <= 0)
      duration = 60;
  }

  printf("Starting cfstest for %d ticks...\n", duration);
  for (int i = 0; i < n; i++) {
    int pid = fork();
    if (pid < 0) {
      printf("cfstest: fork failed\n");
      exit(1);
    }
    if (pid == 0) {
      worker(nices[i], duration);
    }
  }

  for (int i = 0; i < n; i++) {
    wait(0);
  }

  printf("cfstest finished.\n");
  exit(0);
}