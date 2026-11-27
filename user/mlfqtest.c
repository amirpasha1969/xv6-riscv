#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"
#include "kernel/pinfo.h"

#define HOG_TICKS 1000   // must be > AGING_LIMIT (800)
#define RUN_TICKS 300

static struct pinfo buf[NPROC];

static int
myqueue(int pid)
{
  int n = getprocs(buf, NPROC);
  for(int i = 0; i < n; i++)
    if(buf[i].pid == pid)
      return buf[i].queue;
  return -1;
}

static void
worker(int burst)
{
  uint64 cnt = 0;
  printf("worker burst=%d starts running in queue %d\n",
         burst, myqueue(getpid()));
  int end = uptime() + RUN_TICKS;
  while(uptime() < end)
    cnt++;
  printf("worker burst=%d done, counter=%d\n", burst, (int)cnt);
  exit(0);
}

int
main(void)
{
  int bursts[3] = {4, 8, 12};

  int hog = fork();
  if(hog == 0){
    int end = uptime() + HOG_TICKS;
    while(uptime() < end)
      ;
    exit(0);
  }

  // let one tick pass so the workers enter queue 3 after the hog (FCFS)
  int t = uptime();
  while(uptime() == t)
    ;

  for(int i = 0; i < 3; i++){
    int pid = fork();
    if(pid == 0)
      worker(bursts[i]);
    setsched(pid, bursts[i], 50);
  }

  int n = getprocs(buf, NPROC);
  printf("--- initial state (expect sh in Q1, everything else in Q3) ---\n");
  for(int i = 0; i < n; i++)
    printf("pid=%d ppid=%d Q=%d wait=%d burst=%d conf=%d %s\n",
           buf[i].pid, buf[i].ppid, buf[i].queue, buf[i].wait_time,
           buf[i].burst, buf[i].confidence, buf[i].name);

  for(int i = 0; i < 4; i++)
    wait(0);
  printf("mlfqtest done\n");
  exit(0);
}