// user/usyscall.h
#ifndef USYSCALL_H
#define USYSCALL_H

// Hardcode the user-space virtual address of the usyscall page.
// This must match the kernel definition (TRAPFRAME - PGSIZE).
#define USYSCALL 0x3fffffe000L

struct usyscall {
  int pid;
};

#endif

