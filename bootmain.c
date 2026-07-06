
#include "types.h"
#include "elf.h"
#include "x86.h"

#define SECTSIZE 512

void readsegment(uint8_t *, size_t, size_t );

void *memset(void *addr, int c, size_t n);

void bootmain(void){

	struct elfhdr *elf = (void*)0x10000;
	readsegment((void*)elf, 0, sizeof(*elf));
	if (elf->magic != ELF_MAGIC)
		return;

	struct proghdr *ph, *eph;
	ph = (void*)((uint8_t*)elf + elf->phoff);
	eph = ph + elf->phnum;
	for (;ph < eph; ph++){
		readsegment((void*)ph->paddr, ph->offset, ph->filesz);
		if (ph->memsz > ph->filesz)
			memset((uint8_t*)ph->paddr+ph->filesz, 0, ph->memsz-ph->filesz);
	}

	void (*entry)(void) = (void*)(elf->entry);
	entry();
}

void waitdisk(void){
	while ((inb(0x1f7) & 0xc0) != 0x40);
}

void readsector(void *addr, size_t sector){

	outb(0x1f2, 1);
	outb(0x1f3, sector);
	outb(0x1f4, sector >> 8);
	outb(0x1f5, sector >> 16);
	outb(0x1f6, 0xe0 | sector >> 24);
	outb(0x1f7, 0x20);

	waitdisk();
	insl(0x1f0, addr, SECTSIZE/4);
}

void readsegment(uint8_t *pa, size_t offset, size_t size){

	uint8_t *epa = pa + size;
	size_t sector = 27 + offset/SECTSIZE;

	for (; pa < epa; pa += SECTSIZE, sector++)
		readsector(pa, sector);
}

void *memset(void *addr, int c, size_t n){

	c &= 0xff;

	if ((uintptr_t)addr % 8 == 0 && n % 8 == 0){
		uint64_t fill = c;
		fill = fill << 56 | fill << 48 | fill << 40 | fill << 32
			 | fill << 24 | fill << 16 | fill << 8 | fill;
		stosq(addr, fill, n / 8);
	} else if ((uintptr_t)addr % 4 == 0 && n % 4 == 0){
		uint32_t fill = c;
		fill = fill << 24 | fill << 16 | fill << 8 | fill;
		stosl(addr, fill, n / 4);
	} else if ((uintptr_t)addr % 2 == 0 && n % 2 == 0){
		uint16_t fill = c;
		fill = fill << 8 | fill;
		stosw(addr, fill, n / 2);
	} else
		stosb(addr, c, n);

	return addr;
}
