
#include "types.h"
#include "defs.h"
#include "param.h"
#include "mmu.h"
#include "memlayout.h"
#include "x86.h"

extern char text[];
extern char data[];
extern char bss[];
extern char end[];

extern char stack[];

void screen_init(void){
	outb(0x3c9, 0x00);
	outb(0x3c9, 0x2c);
	outb(0x3c9, 0x3b);
}

void mem(void){
	cprintf("text %p \n", text);
	cprintf("data %p \n", data);
	cprintf("bss %p \n", bss);
	cprintf("end %p \n", end);
}

void mm_debug(void);

int main(void){

	screen_init();
	cls();
	uartinit();

	cprintf("                                     VinOS");
	cprintf("\n                                  Version 0.1     \n");

	mm_init((uint8_t*)PGUP(MMBASE), (uint8_t*)PGUP(MMBASE)+MMPHYSHALF);

	kvm_init();
	lapic_init();
	gdt_init();
	picinit();
	ioapicinit();
	kbd_init();
	timer_init();
	idt_init();

	mm_init((uint8_t*)PGUP(MMBASE)+MMPHYSHALF, (uint8_t*)PGUP(MMBASE)+MMPHYSSIZE);

	sti();

	tss_init();
	syscall_init();

	task_init();
	switch_to_user();
}

__attribute__((__aligned__(PGSIZE)))
uintptr_t pgdir[512] = {
	[0] = 0x00000000 + PTE_P + PTE_W + PTE_PS,
	[1] = 0x00200000 + PTE_P + PTE_W + PTE_PS,
};

__attribute__((__aligned__(PGSIZE)))
uintptr_t pdpt[512] = {
	[0] = KERN_V2P(pgdir) + PTE_P + PTE_W,
	[33] = MMPHYSSTART + PTE_P + PTE_W + PTE_PS,
	[510] = KERN_V2P(pgdir) + PTE_P + PTE_W,
};

__attribute__((__aligned__(PGSIZE)))
uintptr_t pml4t[512] = {
	[0] = KERN_V2P(pdpt) + PTE_P + PTE_W,
	[273] = KERN_V2P(pdpt) + PTE_P + PTE_W,
	[511] = KERN_V2P(pdpt) + PTE_P + PTE_W,
};

__attribute__((__aligned__(PGSIZE)))
uintptr_t pml5t[512] = {
	[0] = KERN_V2P(pml4t) + PTE_P + PTE_W,
	[392] = KERN_V2P(pml4t) + PTE_P + PTE_W,
	[511] = KERN_V2P(pml4t) + PTE_P + PTE_W,
};
