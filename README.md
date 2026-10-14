# xv6 RISC-V with Fork Bomb Shield

An enhanced implementation of MIT's **xv6 RISC-V** operating system featuring kernel-level **Fork Bomb Protection (Fork Bomb Shield)** via the `shield()` system call.

---

## 📌 Overview

In standard Unix systems and vanilla xv6, an uncontrolled process spawning children infinitely (a **fork bomb**, e.g., `:(){ :|:& };:`) rapidly exhausts the system's process table (`NPROC = 64` in xv6). Once all process slots are occupied, no new processes can be created, rendering the shell unresponsive and freezing the operating system until a reboot.

This project introduces a **per-process child limit mechanism** and the `shield()` system call to safeguard the kernel against process table starvation caused by rogue or runaway fork bombs.

---

## 🛡️ How the Fork Bomb Shield Works

1. **System Call (`shield(int max_children)`)**:
   - A process calls `shield(N)` to set the maximum number of concurrent active children it is allowed to spawn.
   - Passing `N >= 0` sets the cap.
   - Passing a negative number (`N < 0`) returns `-1` (invalid argument).

2. **Kernel Enforcement (`kfork()` in `kernel/proc.c`)**:
   - When a process calls `fork()`, the kernel checks if `p->max_children >= 0`.
   - If enabled, the kernel counts all active non-`UNUSED` child processes currently belonging to `p`.
   - If `count >= p->max_children`, the fork request is rejected immediately with `-1`, preventing unbounded process creation.

3. **Child Inheritance**:
   - When a shielded parent creates a child, `np->max_children = p->max_children`.
   - Children inherit their parent's shield limit, preventing multi-generational recursive tree fork bombs from bypassing limits.

4. **Dynamic Reaping**:
   - If a shielded process reaps a terminated child using `wait()`, that child transitions to `UNUSED`.
   - The active child count decrements, allowing the process to safely spawn another child within its quota.

---

## 🛠️ Modified & New Files

| File | Purpose |
|------|---------|
| [`kernel/proc.h`](file:///kernel/proc.h) | Added `max_children` field to `struct proc`. |
| [`kernel/proc.c`](file:///kernel/proc.c) | Initialized/reset `max_children` in `allocproc()` and `freeproc()`, and enforced child limit & inheritance in `kfork()`. |
| [`kernel/syscall.h`](file:///kernel/syscall.h) | Defined system call number `SYS_shield 23`. |
| [`kernel/syscall.c`](file:///kernel/syscall.c) | Registered `sys_shield` handler in the syscall table. |
| [`kernel/sysproc.c`](file:///kernel/sysproc.c) | Implemented `sys_shield()` to validate and assign `max_children`. |
| [`user/user.h`](file:///user/user.h) | Declared user-level stub `int shield(int)`. |
| [`user/usys.pl`](file:///user/usys.pl) | Added entry for `shield` syscall generation. |
| [`user/shieldtest.c`](file:///user/shieldtest.c) | Automated test suite validating shield behavior and edge cases. |
| [`user/fb_shield.c`](file:///user/fb_shield.c) | Interactive demonstration of a fork bomb contained by `shield()`. |
| [`user/fb_noshield.c`](file:///user/fb_noshield.c) | Demonstration of an unshielded fork bomb exhausting the process table. |
| [`Makefile`](file:///Makefile) | Added `_shieldtest`, `_fb_shield`, and `_fb_noshield` to `UPROGS`. |

---

## 🚀 Building and Running xv6

### Prerequisites
You need a RISC-V cross-compiler toolchain and QEMU:
- **RISC-V toolchain**: `riscv64-unknown-elf-gcc` or `riscv64-linux-gnu-gcc`
- **QEMU**: `qemu-system-riscv64`

### Compiling and Launching
Run:
```bash
make clean
make qemu
```
To exit QEMU at any time, press `Ctrl+A` followed by `X`.

---

## 🧪 How to Execute the Fork Bomb Shield

Once inside the xv6 interactive shell `$ `, you have three programs to test and observe:

### 1. Run Automated Unit Tests (`shieldtest`)
Runs the full regression test suite covering argument validation, zero-limit enforcement, active child caps, wait-reaping recycling, inheritance, and multi-fork containment:
```bash
$ shieldtest
=== Starting shield tests ===
test_invalid_args: OK
test_zero_limit: OK
test_max_children_limit: OK
test_child_inheritance: OK
test_fork_bomb_mitigation: OK
=== All shield tests PASSED ===
```

### 2. Run Fork Bomb With Shield (`fb_shield`)
Executes a fork bomb attempt with the shield active. You can specify a custom child limit (default is 3):
```bash
$ fb_shield
=== Fork Bomb WITH Shield (limit = 3) ===
[PID 4] Shield active with limit=3. Starting fork bomb loop...
[PID 4] created child 1 (pid 5)
[PID 4] created child 2 (pid 6)
[PID 4] created child 3 (pid 7)
[PID 4] Shield activated! fork() rejected after 3 active children.
[PID 4] Fork bomb safely thwarted by shield.
[PID 4] Cleaning up children...
=== Process 4 finished cleanly. System remains healthy! ===
```
You can also pass a custom limit as an argument:
```bash
$ fb_shield 5
```
Notice that even after the fork bomb executes, the system stays completely responsive and the shell remains ready for input.

### 3. Run Fork Bomb Without Shield (`fb_noshield`)
To contrast shielded execution with traditional vulnerability, run the unshielded fork bomb:
```bash
$ fb_noshield
=== Fork Bomb WITHOUT Shield ===
Pid 4 running fork bomb without any shield.
This will rapidly saturate the entire process table (NPROC=64).
...
[PID 4] fork() failed: process table full! System is exhausted.
```
This demonstrates process table exhaustion where all 64 slots are consumed.

---

## 📜 License & Acknowledgments
xv6 is MIT-licensed and maintained by MIT PDOS (Frans Kaashoek, Robert Morris, and contributors). Fork Bomb Shield extension developed for OS research and resource isolation.
