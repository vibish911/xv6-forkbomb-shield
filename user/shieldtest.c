#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void
test_invalid_args(void)
{
  printf("test_invalid_args: ");
  if (shield(-1) != -1) {
    printf("FAIL: shield(-1) should return -1\n");
    exit(1);
  }
  printf("OK\n");
}

void
test_zero_limit(void)
{
  printf("test_zero_limit: ");
  int pid = fork();
  if (pid < 0) {
    printf("FAIL: initial fork failed\n");
    exit(1);
  }
  if (pid == 0) {
    // Child sets shield to 0
    if (shield(0) < 0) {
      printf("FAIL: shield(0) failed\n");
      exit(1);
    }
    int child = fork();
    if (child >= 0) {
      printf("FAIL: fork succeeded when shield was 0\n");
      exit(1);
    }
    exit(0);
  }
  int status;
  wait(&status);
  if (status != 0) {
    printf("FAIL: child exited with status %d\n", status);
    exit(1);
  }
  printf("OK\n");
}

void
test_max_children_limit(void)
{
  printf("test_max_children_limit: ");
  int pid = fork();
  if (pid < 0) {
    printf("FAIL: fork failed\n");
    exit(1);
  }
  if (pid == 0) {
    if (shield(2) < 0) {
      printf("FAIL: shield(2) failed\n");
      exit(1);
    }
    int c1 = fork();
    if (c1 < 0) {
      printf("FAIL: fork 1 failed\n");
      exit(1);
    }
    if (c1 == 0) {
      pause(20);
      exit(0);
    }

    int c2 = fork();
    if (c2 < 0) {
      printf("FAIL: fork 2 failed\n");
      exit(1);
    }
    if (c2 == 0) {
      pause(20);
      exit(0);
    }

    // 3rd fork should fail because limit is 2
    int c3 = fork();
    if (c3 >= 0) {
      printf("FAIL: fork 3 succeeded despite limit of 2\n");
      exit(1);
    }

    // Wait for one child to exit
    wait(0);

    // Now forking another should succeed since active children dropped to 1
    int c4 = fork();
    if (c4 < 0) {
      printf("FAIL: fork after wait failed\n");
      exit(1);
    }
    if (c4 == 0) {
      exit(0);
    }

    wait(0);
    wait(0);
    exit(0);
  }
  int status;
  wait(&status);
  if (status != 0) {
    printf("FAIL: child exited with status %d\n", status);
    exit(1);
  }
  printf("OK\n");
}

void
test_child_inheritance(void)
{
  printf("test_child_inheritance: ");
  int pid = fork();
  if (pid < 0) {
    printf("FAIL: fork failed\n");
    exit(1);
  }
  if (pid == 0) {
    if (shield(1) < 0) {
      printf("FAIL: shield(1) failed\n");
      exit(1);
    }
    int c1 = fork();
    if (c1 < 0) {
      printf("FAIL: fork failed\n");
      exit(1);
    }
    if (c1 == 0) {
      // Child inherits shield(1)
      int g1 = fork();
      if (g1 < 0) {
        printf("FAIL: child fork 1 failed\n");
        exit(1);
      }
      if (g1 == 0) {
        exit(0);
      }

      // Grandchild 2 should be rejected
      int g2 = fork();
      if (g2 >= 0) {
        printf("FAIL: child was able to fork beyond inherited limit\n");
        exit(1);
      }
      wait(0);
      exit(0);
    }
    wait(0);
    exit(0);
  }
  int status;
  wait(&status);
  if (status != 0) {
    printf("FAIL: child exited with status %d\n", status);
    exit(1);
  }
  printf("OK\n");
}

void
test_fork_bomb_mitigation(void)
{
  printf("test_fork_bomb_mitigation: ");
  int pid = fork();
  if (pid < 0) {
    printf("FAIL: fork failed\n");
    exit(1);
  }
  if (pid == 0) {
    // Shield limits process to 3 children
    shield(3);
    int count = 0;
    for (int i = 0; i < 20; i++) {
      int c = fork();
      if (c == 0) {
        exit(0);
      } else if (c > 0) {
        count++;
      }
    }
    // Only 3 should have succeeded before children are reaped
    if (count != 3) {
      printf("FAIL: spawned %d children, expected 3\n", count);
      exit(1);
    }
    for (int i = 0; i < count; i++) {
      wait(0);
    }
    exit(0);
  }
  int status;
  wait(&status);
  if (status != 0) {
    printf("FAIL: child exited with status %d\n", status);
    exit(1);
  }
  printf("OK\n");
}

int
main(int argc, char *argv[])
{
  printf("=== Starting shield tests ===\n");
  test_invalid_args();
  test_zero_limit();
  test_max_children_limit();
  test_child_inheritance();
  test_fork_bomb_mitigation();
  printf("=== All shield tests PASSED ===\n");
  exit(0);
}
