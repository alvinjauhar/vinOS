
#include "memlayout.h"
#include "param.h"

.globl _start
_start = KERN_V2P_WO(entry)

.code32
entry:
	mov %cr0,%eax
	and $~(1<<31),%eax
	mov %eax,%cr0

	mov $KERN_V2P_WO(pml5t),%eax
	mov %eax,%cr3

	mov %cr4,%eax
	or $(1<<12),%eax
	mov %eax,%cr4

	mov %cr0,%eax
	or $(1<<31),%eax
	mov %eax,%cr0

	ljmp $0x8,$KERN_V2P_WO(entry64)

.code64
entry64:

	mov $(stack + KSTACKSIZE),%rsp

	mov $main,%rax
	jmp *%rax

spin:
	jmp spin

.comm stack, KSTACKSIZE
