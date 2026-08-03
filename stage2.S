
#include "mmu.h"
#include "memlayout.h"

.code16
.globl stage2
stage2:

	lgdt gdtdesc

	mov %cr0,%eax
	or $(1<<0),%eax
	mov %eax,%cr0

	ljmp $0x8,$start32

.code32
start32:

	xor %edi,%edi
	mov $1024*4,%ecx
	xor %eax,%eax
	rep stosl

	;# pml5t
	xor %edi,%edi
	movl $(0x1000 + PTE_P|PTE_W),(%edi)	;# 0
	movl $(0x1000 + PTE_P|PTE_W),3136(%edi)	;# 392
	movl $(0x1000 + PTE_P|PTE_W),4088(%edi)	;# 511

	;# pml4t
	movl $0x1000,%edi
	movl $(0x2000 + PTE_P|PTE_W),(%edi)	;# 0
	movl $(0x2000 + PTE_P|PTE_W),2184(%edi)	;# 273
	movl $(0x2000 + PTE_P|PTE_W),4088(%edi)	;# 511

	;# pdpt
	movl $0x2000,%edi
	movl $(0x3000 + PTE_P|PTE_W),(%edi)	;# 0
	movl $(0x3000 + PTE_P|PTE_W),256(%edi)	;# 32
	movl $(0x3000 + PTE_P|PTE_W),4080(%edi)	;# 510

	;# pgdir
	movl $0x3000,%edi
	movl $(0x0 + PTE_P|PTE_W|PTE_PS),(%edi) ;# 0
	movl $(0x00200000 + PTE_P|PTE_W|PTE_PS),8(%edi) ;# 1
	movl $(0x10000000 + PTE_P|PTE_W|PTE_PS),1024(%edi)	;# 128
	movl $(0x10200000 + PTE_P|PTE_W|PTE_PS),1032(%edi)
	movl $(0x10400000 + PTE_P|PTE_W|PTE_PS),1040(%edi)
	movl $(0x10600000 + PTE_P|PTE_W|PTE_PS),1048(%edi)

	xor %eax,%eax
	mov %eax,%cr3

	mov %cr4,%eax
	or $(1<<5|1<<12),%eax
	mov %eax,%cr4

	mov $0xc0000080,%ecx
	rdmsr
	or $(1<<8),%eax
	wrmsr

	mov %cr0,%eax
	or $(1<<16|1<<31),%eax
	mov %eax,%cr0

	lgdt gdtdesc1

	ljmp $0x8,$start64

.code64
start64:

	mov $0x10,%ax
	mov %ax,%ds
	mov %ax,%es
	mov %ax,%ss

	call bootmain

spin:
	jmp spin

gdt:
	.quad 0
	.quad 0x00cf9a000000ffff
	.quad 0x00cf92000000ffff
gdtdesc:
	.word gdtdesc - gdt - 1
	.long gdt

gdt1:
	.quad 0
	.quad 0x00a09a0000000000
	.quad 0x00a0920000000000
gdtdesc1:
	.word gdtdesc1 - gdt1 - 1
	.quad gdt1
