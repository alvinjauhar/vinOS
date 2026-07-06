
#include "types.h"
#include "defs.h"
#include "idt.h"
#include "x86.h"
#include "msr.h"

extern void syscall_entry(void);

int sys_write(void);
int sys_fork(void);
int sys_getpid(void);

void syscall_init(void){
	wrmsr(MSR_EFER, (1 << 0));
	wrmsr(MSR_STAR, (uint64_t)0x8 << 32 | (uint64_t)0x1b << 48);
	wrmsr(MSR_LSTAR, (uintptr_t)syscall_entry);
}

int (*syscalls[])(void) = {
	[0] NULL,
	[1] sys_write,
	[2] sys_fork,
	[3] sys_getpid,
};

void syscall_handler(struct registers *r){

	size_t num;

	num = r->rax;
	if (num < NELEM(syscalls) && syscalls[num]){
		r->rax = syscalls[num]();
	} else {
		r->rax = -1;
	}
}
