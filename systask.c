
#include "types.h"
#include "defs.h"
#include "list.h"
#include "idt.h"
#include "sched.h"

int sys_fork(void){
	return fork();
}

int sys_getpid(void){
	return current->pid;
}
