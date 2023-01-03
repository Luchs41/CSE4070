#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "devices/shutdown.h"
#include "devices/input.h"
#include "userprog/process.h"
#include "filesys/filesys.h"
#include "filesys/file.h"
#include <string.h>
struct lock file_lock;
static void syscall_handler (struct intr_frame *);

/* syscall functions */
void halt() {
	shutdown_power_off();
}
void exit(int status) {
	printf("%s: exit(%d)\n", thread_name(), status);
	thread_current()->exit_status = status;
	thread_exit();
}
pid_t exec(const char *file) {
	pid_t pid = process_execute(file);
	//sema_down(&(thread_current()->sem_load));
	//if (pid == TID_ERROR) return -1;
	//if (!get_child_process(pid)->memory_flag) return -1;
	return pid;
}

int wait(pid_t pid) {
	int status = process_wait(pid);
	return status;
}

bool create(const char *file, unsigned initial_size) {	
	if(!file) exit(-1);
	return filesys_create(file, initial_size);
}

bool remove(const char *file) {
	return filesys_remove(file);	
}

int open(const char *file) {	
	if(!file) exit(-1);
	struct file *fp = filesys_open(file);
	if(!fp) return -1;
	
	lock_acquire(&file_lock);

	if(!strcmp(file, thread_current()->name)) file_deny_write(fp);
	int new_fd = process_add_file(fp);
	lock_release(&file_lock);
	return new_fd;
}

int filesize(int fd) {
	if(!process_get_file(fd)) exit(-1);
	return (int)(file_length(process_get_file(fd)));
}


int read(int fd, void *buffer, unsigned size) {
	if(!is_user_vaddr(buffer)) exit(-1);
	lock_acquire(&file_lock);
	if(fd == 0) {
		for (unsigned int i = 0; i < size; i++) {
			*(char *)(buffer + i) = input_getc();
		}
	}
	else {
		if(!process_get_file(fd)) {
			lock_release(&file_lock);
			exit(-1);
		}
		lock_release(&file_lock);
		return file_read(process_get_file(fd), buffer, size);
	}
	lock_release(&file_lock);
	return size;
}

int write(int fd, const void *buffer, unsigned size) {
	if(!is_user_vaddr(buffer)) exit(-1);
	lock_acquire(&file_lock);
	if(fd == 1) {
		putbuf(buffer, size);
		lock_release(&file_lock);
		return size;
	}
	else {
		if(!process_get_file(fd)) {
			lock_release(&file_lock);
			exit(-1);
		}
		lock_release(&file_lock);
		return file_write(process_get_file(fd), buffer, size);
	}
	return -1;
}

void seek(int fd, unsigned position) {
	if(!process_get_file(fd)) exit(-1);
	file_seek(process_get_file(fd), position);
}

unsigned tell(int fd) {
	if(!process_get_file(fd)) exit(-1);
	return file_tell(process_get_file(fd));
}

void close(int fd) {
	if(!process_get_file(fd)) exit(-1);
	file_close(process_get_file(fd));
	thread_current()->fd_table[fd] = NULL;
}
/* additional */

int fibonacci(int n) {
	if(n < 1) return 0;
	else if(n == 1 || n == 2) return 1;
	return fibonacci(n - 2) + fibonacci(n - 1);
	
}

int max_of_four_int(int a, int b, int c, int d) {
	int max = a;
	if(b > max) max = b;
	if(c > max) max = c;
	if(d > max) max = d;

	return max;
}

/* additional */
/* syscall functions */
void
syscall_init (void) 
{
	lock_init(&file_lock);
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f) 
{
	switch(*(int *)(f->esp)) {
		case SYS_HALT:
			halt();
			break;

		case SYS_EXIT:
			if(is_kernel_vaddr(f->esp + 4)) exit(-1);
			exit((int)*(int *)(f->esp + 4));
			break;
		
		case SYS_EXEC:
			if(is_kernel_vaddr(f->esp + 4)) exit(-1);
			f->eax = exec((char *)*(int *)(f->esp + 4));
			break;

		case SYS_WAIT:
			if(is_kernel_vaddr(f->esp + 4)) exit(-1);
			f->eax = wait((pid_t)*(int *)(f->esp + 4));	
			break;
		
		case SYS_CREATE:
			if(is_kernel_vaddr(f->esp + 4)) exit(-1);
			f->eax = create((const char *)*(int*)(f->esp + 4), (off_t)*(int*)(f->esp + 8));
			break;

		case SYS_REMOVE:
			if(is_kernel_vaddr(f->esp + 4)) exit(-1);
			f->eax = remove((const char*)*(int*)(f->esp + 4));
			break;

		case SYS_OPEN:
			if(is_kernel_vaddr(f->esp + 4)) exit(-1);
			f->eax = open((const char*)*(int*)(f->esp + 4));
			break;

		case SYS_FILESIZE:
			if(is_kernel_vaddr(f->esp + 4)) exit(-1);
			f->eax = filesize((int)*(int*)(f->esp + 4));
			break;

		case SYS_READ:
			if(is_kernel_vaddr(f->esp + 4)) exit(-1);
			f->eax = read((int)*(int *)(f->esp + 4), (void *)*(int *)(f->esp + 8), (unsigned)*(int *)(f->esp + 12));
			break;

		case SYS_WRITE:
			if(is_kernel_vaddr(f->esp + 4)) exit(-1);
			f->eax = write((int)*(int *)(f->esp + 4), (void *)*(int *)(f->esp + 8), (unsigned)*(int *)(f->esp + 12));
			break;

		case SYS_SEEK:
			if(is_kernel_vaddr(f->esp + 4)) exit(-1);
			seek((int)*(int *)(f->esp + 4), (unsigned)*(int *)(f->esp + 8));
			break;

		case SYS_TELL:
			if(is_kernel_vaddr(f->esp + 4)) exit(-1);
			f->eax = tell((int)*(int *)(f->esp + 4));
			break;

		case SYS_CLOSE:
			if(is_kernel_vaddr(f->esp + 4)) exit(-1);
			close((int)*(int *)(f->esp + 4));
			break;


		/* additional */
		case SYS_FIBO:
			if(is_kernel_vaddr(f->esp + 4)) exit(-1);
			f->eax = fibonacci((int)*(int*)(f->esp + 4));
			break;

		case SYS_MAX:
			if(is_kernel_vaddr(f->esp + 4)) exit(-1);
			f->eax = max_of_four_int((int)*(int*)(f->esp + 4), (int)*(int*)(f->esp + 8), (int)*(int*)(f->esp + 12), (int)*(int*)(f->esp + 16));
			break;
		/* additional */

	}
  //thread_exit ();
}
