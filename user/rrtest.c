// rrtest: scheduler demo.
// Forks NCHILD CPU-bound children. Each child busy-loops and prints one
// line every time it notices that the global tick counter changed.
// It exits after printing LINES lines, i.e. after ~LINES ticks of its own run.
//
//  - RR queue (quantum 5): you see blocks of 5 consecutive ticks per child,
//    then the next child, round-robin.
//  - FCFS queue: one child prints all its lines, then the next one starts.
//  - With CPUS>1 the children run in parallel and lines interleave.

#include "kernel/types.h"
#include "user/user.h"

#define NCHILD 3
#define LINES  15

static void
child(int id)
{
  int pid = getpid();
  int last = uptime();
  int n = 0;

  printf("[child %d pid=%d] start at tick %d\n", id, pid, last);

  while (n < LINES) {
    int t = uptime();
    if (t != last) {
      last = t;
      n++;
      printf("[child %d pid=%d] tick=%d  (%d/%d)\n", id, pid, t, n, LINES);
    }
  }

  printf("[child %d pid=%d] DONE at tick %d\n", id, pid, uptime());
  exit(0);
}

int
main(void)
{
  int i;

  printf("rrtest: parent pid=%d, starting %d children at tick %d\n",
         getpid(), NCHILD, uptime());

  for (i = 0; i < NCHILD; i++) {
    int pid = fork();
    if (pid < 0) {
      printf("rrtest: fork failed\n");
      exit(1);
    }
    if (pid == 0)
      child(i);
  }

  for (i = 0; i < NCHILD; i++) {
    int pid = wait(0);
    printf("rrtest: child pid=%d exited at tick %d\n", pid, uptime());
  }

  printf("rrtest: all children finished\n");
  exit(0);
}