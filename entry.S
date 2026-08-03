
#include "memlayout.h"
#include "param.h"

.globl _start
_start = entry

entry:

	mov $(stack + KSTACKSIZE),%rsp

	mov $main,%rax
	jmp *%rax

.comm stack, KSTACKSIZE
