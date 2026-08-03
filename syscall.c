
#include "types.h"
#include "defs.h"
#include "idt.h"
#include "list.h"
#include "sched.h"
#include "syscall.h"
#include "x86.h"
#include "msr.h"

extern void syscall_entry(void);

int sys_write(void);
int sys_fork(void);
int sys_exit(void);
int sys_wait(void);
int sys_getpid(void);
int sys_pause(void);

void syscall_init(void){
	wrmsr(MSR_EFER, (1 << 0));
	wrmsr(MSR_STAR, (uint64_t)0x8 << 32 | (uint64_t)0x1b << 48);
	wrmsr(MSR_LSTAR, (uintptr_t)syscall_entry);
}

int (*syscalls[])(void) = {
	[0] NULL,
	[SYS_write] sys_write,
	[SYS_fork] sys_fork,
	[SYS_exit] sys_exit,
	[SYS_wait] sys_wait,
	[SYS_getpid] sys_getpid,
	[SYS_pause] sys_pause,
};

void syscall_handler(struct registers *r){

	size_t num;

	num = r->rax;
	if (num < NELEM(syscalls) && syscalls[num]){
		r->rax = syscalls[num]();
	} else {
		cprintf("unknown syscall\n");
		r->rax = -1;
	}
}
