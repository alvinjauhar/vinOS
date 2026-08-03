
#include "types.h"
#include "defs.h"
#include "list.h"
#include "idt.h"
#include "sched.h"

int sys_write(void){

	char *addr = (char*)current_task->regs->rdi;
	size_t n = current_task->regs->rsi;
	
	return consolewrite1(addr, n);
}
