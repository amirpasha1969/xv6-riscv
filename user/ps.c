#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"
#include "kernel/pinfo.h"

// Must match enum procstate in kernel/proc.h
static char *states[] = {
  "UNUSED", "USED", "SLEEPING", "RUNNABLE", "RUNNING", "ZOMBIE"
};

static struct pinfo buf[NPROC];

int
main(void)
{
  int n = getprocs(buf, NPROC);
  if(n < 0){
    printf("ps: getprocs failed\n");
    exit(1);
  }

  printf("PID\tPPID\tSTATE\t\tSIZE\tQ\tWAIT\tBURST\tCONF\tNAME\n");
  for(int i = 0; i < n; i++){
    char *s = "???";
    if(buf[i].state >= 0 && buf[i].state < 6)
      s = states[buf[i].state];
    printf("%d\t%d\t%s\t%d\t%d\t%d\t%d\t%d\t%s\n",
           buf[i].pid, buf[i].ppid, s, (int)buf[i].sz,
           buf[i].queue, buf[i].wait_time, buf[i].burst,
           buf[i].confidence, buf[i].name);
  }
  exit(0);
}