
.globl write
write:
	mov $1,%rax
	syscall
	ret

.globl fork
fork:
	mov $2,%rax
	syscall
	ret

.globl getpid
getpid:
	mov $3,%rax
	syscall
	ret
