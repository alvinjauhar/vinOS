
#include "syscall.h"

.globl write
write:
	mov $SYS_write,%rax
	syscall
	ret

.globl fork
fork:
	mov $SYS_fork,%rax
	syscall
	ret

.globl exit
exit:
	mov $SYS_exit,%rax
	syscall
	ret

.globl wait
wait:
	mov $SYS_wait,%rax
	syscall
	ret

.globl getpid
getpid:
	mov $SYS_getpid,%rax
	syscall
	ret

.globl pause
pause:
	mov $SYS_pause,%rax
	syscall
	ret
