#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H
#include "lib/user/syscall.h"
#include "userprog/process.h"
#include "threads/vaddr.h"
void syscall_init (void);

void halt(void);
void exit (int status);
pid_t exec(const char *file);
int wait(pid_t pid);
int read(int fd, void *buffer, unsigned size);
int write (int fd, const void *buffer, unsigned size);

/* additional */
int fibonacci(int n);
int max_of_four_int(int a, int b, int c, int d);
/* additional */
#endif /* userprog/syscall.h */
