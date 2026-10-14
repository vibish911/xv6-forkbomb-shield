#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  int limit = 3;
  if (argc > 1) {
    limit = atoi(argv[1]);
  }

  printf("=== Fork Bomb WITH Shield (limit = %d) ===\n", limit);
  if (shield(limit) < 0) {
    printf("shield(%d) failed!\n", limit);
    exit(1);
  }
  printf("[PID %d] Shield active with limit=%d. Starting fork bomb loop...\n", getpid(), limit);

  int spawned = 0;
  while (1) {
    int pid = fork();
    if (pid < 0) {
      printf("[PID %d] Shield activated! fork() rejected after %d active children.\n", getpid(), spawned);
      printf("[PID %d] Fork bomb safely thwarted by shield.\n", getpid());
      break;
    }
    if (pid == 0) {
      // Child process inherits shield limit and continues loop
      pause(20);
      exit(0);
    }
    spawned++;
    printf("[PID %d] created child %d (pid %d)\n", getpid(), spawned, pid);
  }

  // Parent cleans up spawned children
  printf("[PID %d] Cleaning up children...\n", getpid());
  for (int i = 0; i < spawned; i++) {
    wait(0);
  }

  printf("=== Process %d finished cleanly. System remains healthy! ===\n", getpid());
  exit(0);
}
