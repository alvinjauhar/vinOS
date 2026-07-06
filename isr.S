
.globl isr_common
isr_common:

	push %rax
	push %rbx
	push %rcx
	push %rdx
	push %rsi
	push %rdi
	push %rbp

	push %r8
	push %r9
	push %r10
	push %r11
	push %r12
	push %r13
	push %r14
	push %r15

	mov %rsp,%rdi
	call isr_handler

	pop %r15
	pop %r14
	pop %r13
	pop %r12
	pop %r11
	pop %r10
	pop %r9
	pop %r8

	pop %rbp
	pop %rdi
	pop %rsi
	pop %rdx
	pop %rcx
	pop %rbx
	pop %rax

	add $16,%rsp

	iretq

.globl syscall_entry
syscall_entry:

	swapgs

	mov %rsp,%gs:8
	mov %gs:0,%rsp

	push $0x23
	push %gs:8
	push %r11
	push $0x2b
	push %rcx

	sub $16,%rsp

	push %rax
	push %rbx
	push $0
	push %rdx
	push %rsi
	push %rdi
	push %rbp

	push %r8
	push %r9
	push %r10
	push $0
	push %r12
	push %r13
	push %r14
	push %r15

	mov %rsp,%rdi
	call syscall_handler

	pop %r15
	pop %r14
	pop %r13
	pop %r12
	add $8,%rsp
	pop %r10
	pop %r9
	pop %r8

	pop %rbp
	pop %rdi
	pop %rsi
	pop %rdx
	add $8,%rsp
	pop %rbx
	pop %rax

	add $16,%rsp

	pop %rcx
	add $8,%rsp
	pop %r11
	pop %rsp

	swapgs
	sysretq

