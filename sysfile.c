
#include "types.h"
#include "defs.h"
#include "list.h"
#include "idt.h"
#include "sched.h"

int sys_write(void){

	char *addr = (char*)current->regs->rdi;
	size_t n = current->regs->rsi;
	
	return consolewrite1(addr, n);
}
