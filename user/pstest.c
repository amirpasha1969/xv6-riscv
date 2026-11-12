#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"
#include "kernel/pinfo.h"

static struct pinfo buf[NPROC];

int
main(void)
{
  int pids[3];
  int me = getpid();

  // 2 sleeping children + 1 busy child
  for(int i = 0; i < 3; i++){
    int pid = fork();
    if(pid < 0){
      printf("fork failed\n");
      exit(1);
    }
    if(pid == 0){
      if(i < 2)
        pause(500);
      else
        for(;;);
    }
    pids[i] = pid;
  }
  pause(10);

  printf("parent pid = %d, children = %d %d %d\n",
         me, pids[0], pids[1], pids[2]);

  // 1) full table: children must show ppid == parent pid
  int n = getprocs(buf, NPROC);
  int found = 0;
  for(int i = 0; i < n; i++)
    for(int j = 0; j < 3; j++)
      if(buf[i].pid == pids[j] && buf[i].ppid == me)
        found++;
  printf("[1] getprocs(NPROC) = %d, children found with correct ppid: %d/3 %s\n",
         n, found, found == 3 ? "OK" : "FAIL");

  // 2) max smaller than process count
  int r = getprocs(buf, 2);
  printf("[2] getprocs(max=2) = %d %s\n", r, r == 2 ? "OK" : "FAIL");

  // 3) max larger than NPROC
  r = getprocs(buf, 1000);
  printf("[3] getprocs(max=1000) = %d %s\n", r, r == n ? "OK" : "FAIL");

  // 4) invalid arguments
  r = getprocs(buf, 0);
  printf("[4] getprocs(max=0) = %d %s\n", r, r == -1 ? "OK" : "FAIL");
  r = getprocs(buf, -5);
  printf("[5] getprocs(max=-5) = %d %s\n", r, r == -1 ? "OK" : "FAIL");
  r = getprocs(0, 5);
  printf("[6] getprocs(NULL) = %d %s\n", r, r == -1 ? "OK" : "FAIL");
  r = getprocs((struct pinfo *)0x7fffffffUL, 5);
  printf("[7] getprocs(bad addr) = %d %s\n", r, r == -1 ? "OK" : "FAIL");

  // 5) show it with ps
  printf("\n--- ps output ---\n");
  int p = fork();
  if(p == 0){
    char *argv[] = { "ps", 0 };
    exec("ps", argv);
    printf("exec ps failed\n");
    exit(1);
  }
  wait(0);

  for(int i = 0; i < 3; i++)
    kill(pids[i]);
  for(int i = 0; i < 3; i++)
    wait(0);
  exit(0);
}