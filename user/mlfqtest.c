#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

// usage: cfstest [cfs|rr]
//   (no arg) just print the results
//   cfs      also check that counters follow the weight ratios
//            (build with SCHEDULER=CFS CPUS=1)
//   rr       also check that counters are about equal
//            (build with SCHEDULER=RR CPUS=1)
// xv6's printf has no width/precision support (%5d, %.2f), so none is used.

#define NCHILD   3
#define DURATION 600      // ticks every child keeps computing

struct res { int id; uint64 cnt; };

static int failures = 0;

static void
check(char *what, int ok)
{
  printf("  [%s] %s\n", ok ? "PASS" : "FAIL", what);
  if(!ok)
    failures++;
}

// prints r/100 with two decimals
static void
print_ratio(char *label, uint64 r)
{
  printf("  %s = %d.%d%d\n", label, (int)(r / 100), (int)(r / 10 % 10),
         (int)(r % 10));
}

int
main(int argc, char *argv[])
{
  char *mode = argc > 1 ? argv[1] : "";
  int nice[NCHILD] = {0, 5, 10};
  int fd[2];
  struct res r;
  uint64 cnt[NCHILD];
  uint64 r01, r12;
  int end;

  printf("== setnice argument checks ==\n");
  check("setnice(self, 0) succeeds", setnice(getpid(), 0) == 0);
  check("nice 20 is rejected", setnice(getpid(), 20) == -1);
  check("nice -21 is rejected", setnice(getpid(), -21) == -1);
  check("unknown pid is rejected", setnice(99999, 0) == -1);

  printf("== CPU share by nice (children compute for %d ticks) ==\n",
         DURATION);
  pipe(fd);
  end = uptime() + DURATION;            // same deadline for every child
  for(int i = 0; i < NCHILD; i++){
    if(fork() == 0){
      uint64 c = 0;
      volatile uint x = 0;
      if(setnice(getpid(), nice[i]) < 0){
        printf("setnice failed\n");
        exit(1);
      }
      while(uptime() < end){
        for(int k = 0; k < 1000; k++)
          x++;
        c++;
      }
      r.id = i;
      r.cnt = c;
      write(fd[1], &r, sizeof(r));
      exit(0);
    }
  }
  for(int i = 0; i < NCHILD; i++){
    read(fd[0], &r, sizeof(r));
    cnt[r.id] = r.cnt;
  }
  while(wait(0) > 0)
    ;
  close(fd[0]);
  close(fd[1]);

  for(int i = 0; i < NCHILD; i++)
    printf("  nice=%d counter=%d\n", nice[i], (int)cnt[i]);
  r01 = cnt[1] ? cnt[0] * 100 / cnt[1] : 0;
  r12 = cnt[2] ? cnt[1] * 100 / cnt[2] : 0;
  print_ratio("counter(nice 0) / counter(nice 5)", r01);
  print_ratio("counter(nice 5) / counter(nice 10)", r12);
  printf("  expected with CFS: about 3.06 and 3.05 (weights 1024/335, 335/110)\n");
  printf("  expected with RR : about 1.00 and 1.00\n");

  if(strcmp(mode, "cfs") == 0){
    check("nice 0 gets 2.2x-4.0x of nice 5", r01 >= 220 && r01 <= 400);
    check("nice 5 gets 2.2x-4.0x of nice 10", r12 >= 220 && r12 <= 400);
  } else if(strcmp(mode, "rr") == 0){
    check("nice 0 / nice 5 is about 1 (0.7-1.4)", r01 >= 70 && r01 <= 140);
    check("nice 5 / nice 10 is about 1 (0.7-1.4)", r12 >= 70 && r12 <= 140);
  }

  if(failures)
    printf("\nSOME CHECKS FAILED (%d)\n", failures);
  else
    printf("\nDONE\n");
  exit(failures ? 1 : 0);
}