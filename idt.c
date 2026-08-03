
#include "types.h"
#include "defs.h"
#include "mmu.h"
#include "idt.h"
#include "kernel.h"
#include "list.h"
#include "sched.h"
#include "x86.h"

#define IO_PIC1 0x20
#define IO_PIC2 0xa0

extern uintptr_t vectors[];

static struct idt_entry idt[256];

void (*handlers[48])(struct registers *r);

struct idt_pointer pointer = {
	sizeof(idt), (uintptr_t)idt
};

void picinit(void){
	outb(IO_PIC1+1, 0xff);
	outb(IO_PIC2+1, 0xff);
}

const char *exceptions[32] = {
	"Divide Error",
	"Debug Exception",
	"NMI Interrupt",
	"Breakpoint",
	"Overflow",
	"BOUND Range Exceeded",
	"Invalid Opcode",
	"Device Not Available",
	"Double Fault",
	"Coprocessor Segment Overrun",
	"Invalid TSS",
	"Segment Not Present",
	"Stack Segment Fault",
	"General Protection",
	"Page Fault",
	"Reserved",
	"x87 FPU Floating-Point Error",
	"Alignment Check",
	"Machine Check",
	"SIMD Floating-Point Exception",
	"Virtualization Protection Exception",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
};

void set_idt(size_t i, uintptr_t offset, uint8_t type){
	idt[i].offset_low = offset;
	idt[i].segment_selector = 0x8;
	idt[i].gate_type = type;
	idt[i].offset_middle = offset >> 16;
	idt[i].offset_high = offset >> 32;
}

void exception_handler(struct registers *r){

	if ((r->cs & DPL_USER) == 0){
		cprintf("exception %d: %s \n", r->trap_no, exceptions[r->trap_no]);
		cprintf("cs %p ss %p rcr2 %p error code %p\n", r->cs, r->ss, rcr2(), r->error_code);
		cprintf("rsp %p rip %p rflags %p \n", r->rsp, r->rip, r->rflags);
		for (;;);
	}

	cprintf("id %d \n", current_task->pid);
	cprintf("exception %d: %s \n", r->trap_no, exceptions[r->trap_no]);
	cprintf("cs %p ss %p rcr2 %p error code %p\n", r->cs, r->ss, rcr2(), r->error_code);
	cprintf("rsp %p rip %p rflags %p \n", r->rsp, r->rip, r->rflags);

	sys_exit();
}

void page_fault_handler(struct registers *r){

	if (!(r->error_code & PTE_U)){
		cprintf("exception %d: %s at kernel mode\n", r->trap_no, exceptions[r->trap_no]);
		cprintf("cs %p ss %p rcr2 %p error code %p\n", r->cs, r->ss, rcr2(), r->error_code);
		cprintf("rsp %p rip %p rflags %p \n", r->rsp, r->rip, r->rflags);
		for (;;);		
	}

	if (r->error_code & PTE_P){
		if (page_fault_wp_page(current_task->pml5t, rcr2())){
			cprintf("rcr2 %p \n", rcr2());
			panic("page_fault handler: wp page\n");			
		}

	} else if (!(r->error_code & PTE_P)) {
		if (page_fault_no_page(current_task->pml5t, rcr2())){
			cprintf("rcr2 %p \n", rcr2());
			panic("page fault handler: no page\n");		
		}
	}
}

void isr_handler(struct registers *r){

	if (handlers[r->trap_no]){
		handlers[r->trap_no](r);
	}
}

void isr_install(size_t i, void (*handler)(struct registers*)){
	handlers[i] = handler;
}

void idt_init(void){

	for (size_t i = 0; i < 256; i++){
		set_idt(i, vectors[i], 0x8e);
	}

	for (size_t i = 0; i < 32; i++){
		isr_install(i, exception_handler);
	}

	isr_install(14, page_fault_handler);
	
	asm volatile("lidt %0" :: "m" (pointer));
}
