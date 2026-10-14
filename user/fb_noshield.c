#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(void)
{
  printf("=== Fork Bomb WITHOUT Shield ===\n");
  printf("Pid %d running fork bomb without any shield.\n", getpid());
  printf("This will rapidly saturate the entire process table (NPROC=64).\n");

  int count = 0;
  while (1) {
    int pid = fork();
    if (pid < 0) {
      printf("[PID %d] fork() failed: process table full! System is exhausted.\n", getpid());
      // Pause briefly so process slots remain occupied to observe saturation
      pause(50);
      exit(1);
    }
    if (pid > 0) {
      count++;
      printf("[PID %d] spawned child %d (pid %d)\n", getpid(), count, pid);
    }
  }
}
