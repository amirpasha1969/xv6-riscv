#ifndef PINFO_H
#define PINFO_H

struct pinfo {
  int pid;
  int ppid;
  int state;
  uint64 sz;
  char name[16];
  int queue;
  int wait_time;
  int burst;
  int confidence;
};

#endif