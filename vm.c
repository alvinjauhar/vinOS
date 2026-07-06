
#include "types.h"
#include "defs.h"
#include "mmu.h"
#include "memlayout.h"
#include "x86.h"

extern char data[];

uintptr_t *kpml5t;

enum page_size { PG_SMALL, PG_LARGE, PG_HUGE};

struct kmap {
	uintptr_t vaddr, pstart, pend;
	uint8_t flags;
	enum page_size page_size;
} kmap[] = {
	{ MMBASE, MMPHYSSTART, MMPHYSEND, PTE_P|PTE_W|PTE_PS, PG_HUGE},
	{ KERNBASE, 0, EXTMEM, PTE_P|PTE_W, PG_SMALL},
	{ KERNBASE+EXTMEM, EXTMEM, KERN_V2P(data), PTE_P, PG_SMALL},
	{ (uintptr_t)data, KERN_V2P(data), PHYSTOP, PTE_P|PTE_W, PG_SMALL},
	{ KERNDEV, DEVSPACE, DEVSPACE+0x1000000, PTE_P|PTE_W|PTE_PS, PG_LARGE}
};

void *walkpages(uintptr_t *pml5t, uintptr_t vaddr, bool alloc,
				enum page_size pg_sz){

	uintptr_t *pml4t, *pdpt, *pgdir, *pgtab;

	if (pml5t[PML5X(vaddr)] & PTE_P){
		pml4t = MM_P2V(PG_ADDR(pml5t[PML5X(vaddr)]));
	} else {
		if (alloc == false || (pml4t = kmalloc(PGSIZE)) == NULL)
			return NULL;

		memset(pml4t, 0, PGSIZE);
		pml5t[PML5X(vaddr)] = MM_V2P(pml4t) | PTE_P | PTE_W | PTE_U;
	}

	if (pml4t[PML4X(vaddr)] & PTE_P){
		pdpt = MM_P2V(PG_ADDR(pml4t[PML4X(vaddr)]));
	} else {
		if (alloc == false || (pdpt = kmalloc(PGSIZE)) == NULL)
			return NULL;

		memset(pdpt, 0, PGSIZE);
		pml4t[PML4X(vaddr)] = MM_V2P(pdpt) | PTE_P | PTE_W | PTE_U;
	}

	if (pg_sz == PG_HUGE){
		return &pdpt[PDPTX(vaddr)];		
	}

	if (pdpt[PDPTX(vaddr)] & PTE_P){
		pgdir = MM_P2V(PG_ADDR(pdpt[PDPTX(vaddr)]));
	} else {
		if (alloc == false || (pgdir = kmalloc(PGSIZE)) == NULL)
			return NULL;

		memset(pgdir, 0, PGSIZE);
		pdpt[PDPTX(vaddr)] = MM_V2P(pgdir) | PTE_P | PTE_W | PTE_U;
	}

	if (pg_sz == PG_LARGE){
		return &pgdir[PDX(vaddr)];
	}

	if (pgdir[PDX(vaddr)] & PTE_P){
		pgtab = MM_P2V(PG_ADDR(pgdir[PDX(vaddr)]));
	} else {
		if (alloc == false || (pgtab = kmalloc(PGSIZE)) == NULL)
			return NULL;

		memset(pgtab, 0, PGSIZE);
		pgdir[PDX(vaddr)] = MM_V2P(pgtab) | PTE_P | PTE_W | PTE_U;
	}

	return &pgtab[PTX(vaddr)];
}

int mappages(uintptr_t *pml5t, uintptr_t vaddr, uintptr_t paddr,
 			 size_t size, uint8_t flags, enum page_size pg_sz){

	 size_t vend;
	 uintptr_t *pte;
	 
	 vend = vaddr + size;
	 while (vaddr < vend){
	 	if ((pte = walkpages(pml5t, vaddr, true, pg_sz)) == NULL)
	 		return -1;

	 	if (*pte & PTE_P)
	 		panic("remap");

	 	*pte = paddr | flags;

	 	if (pg_sz == PG_HUGE){
	 		vaddr += PGSIZE_HUGE, paddr += PGSIZE_HUGE;
	 	} else if (pg_sz == PG_LARGE){
	 		vaddr += PGSIZE_LARGE, paddr += PGSIZE_LARGE;
	 	} else if (pg_sz == PG_SMALL){
	 		vaddr += PGSIZE, paddr += PGSIZE;
	 	}
	 }

	return 0;
}

void *setupkvm(void){

	uintptr_t *pml5t;
	struct kmap *k;

	if ((pml5t = kmalloc(PGSIZE)) == NULL)
		return NULL;

	memset(pml5t, 0, PGSIZE);
	for (k = kmap; k < &kmap[NELEM(kmap)]; k++){
		mappages(pml5t, k->vaddr, k->pstart, k->pend-k->pstart,
		 k->flags, k->page_size);
	}

	return pml5t;
}

void switch_to_kvm(void){
	lcr3(MM_V2P(kpml5t));
}

void kvm_init(void){
	kpml5t = setupkvm();
	switch_to_kvm();
}

void *uvm_init(void *addr, size_t size){

	uintptr_t *pml5t, *paddr;

	pml5t = setupkvm();

	paddr = kmalloc(PGSIZE);
	memset(paddr, 0, PGSIZE);
	mappages(pml5t, 0, MM_V2P(paddr), PGSIZE, PTE_P|PTE_W|PTE_U, PG_SMALL);
	memmove(paddr, addr, size);

	return pml5t;
}

int copy_mem(uintptr_t *new_pml5t, uintptr_t *old_pml5t, size_t size){

	uintptr_t vaddr, *new_paddr, *old_paddr, *pte;

	for (vaddr = 0; vaddr < size; vaddr += PGSIZE){
		pte = walkpages(old_pml5t, vaddr, false, PG_SMALL);
		if (!pte) panic("copy_mem: page should exist");
		old_paddr = MM_P2V(PG_ADDR(*pte));
		new_paddr = kmalloc(PGSIZE);
		if (!new_paddr) return -1;
		memset(new_paddr, 0, PGSIZE);
		memmove(new_paddr, old_paddr, PGSIZE);
		if (mappages(new_pml5t, vaddr, MM_V2P(new_paddr), PGSIZE,
			PTE_P|PTE_W|PTE_U, PG_SMALL) < 0) return -1;
	}

	return 0;
}
