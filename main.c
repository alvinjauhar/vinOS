
#include "types.h"
#include "defs.h"
#include "param.h"
#include "mmu.h"
#include "memlayout.h"
#include "x86.h"

extern char text[], data[], bss[], end[];

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

	mm_init((uint8_t*)MMVIRTSTART, (uint8_t*)MMVIRTSTART+0x800000);	
	kvm_init();
	mm_init((uint8_t*)MMVIRTSTART+0x800000, (uint8_t*)MMVIRTEND);
	lapic_init();
	gdt_init();
	picinit();
	ioapicinit();
	kbd_init();
	timer_init();
	idt_init();
	tss_init();
	syscall_init();
	task_init();
	switch_to_user();

	for(;;);
}
