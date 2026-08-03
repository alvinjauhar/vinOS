
#include "types.h"
#include "defs.h"
#include "gdt.h"

struct gdt gdt;

void set_gdt(size_t i, uint8_t access, uint8_t flags){
	gdt.entry[i].access = access;
	gdt.entry[i].flags = flags;
}

void gdt_init(void){
	set_gdt(1, 0x9a, 0xa0);
	set_gdt(2, 0x92, 0x00);
	set_gdt(3, 0x9a, 0x00);
	set_gdt(4, 0xf2, 0x00);
	set_gdt(5, 0xfa, 0xa0);

	gdt.pointer.size = sizeof(gdt.entry) + sizeof(gdt.entry_high) - 1;
	gdt.pointer.base = (uintptr_t)gdt.entry;
	asm volatile("lgdt %0" :: "m" (gdt.pointer));
}

void tss_init(void){

	uintptr_t addr = (uintptr_t)&gdt.tss;

	gdt.entry[6].limit = sizeof(gdt.tss) - 1;
	gdt.entry[6].base_low = addr;
	gdt.entry[6].base_middle = addr >> 16;
	gdt.entry[6].access = 0x89;
	gdt.entry[6].base_high = addr >> 24;
	gdt.entry_high.base_highest = addr >> 32;

	asm volatile("ltr %%ax" :: "a" (0x30));
}
