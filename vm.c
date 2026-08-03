
// vm.c
// Virtual memory management
// Implements 5-level paging, page table operations, and copy-on-write.

#include "types.h"
#include "defs.h"
#include "mmu.h"
#include "memlayout.h"
#include "x86.h"

extern char text[], data[];

static uintptr_t *kpml5t;

enum page_size { PG_SMALL, PG_LARGE, PG_HUGE};

struct page_list {
	uintptr_t *page;
	int ref;
	struct page_list *next;
};

struct page_list *used_page;

struct kmap {
	uintptr_t vaddr, pstart, pend;
	uint8_t flags;
	enum page_size pg_sz;
} kmap[] = {
	{ MMVIRTSTART, MMPHYSSTART, MMPHYSEND, PTE_P|PTE_W|PTE_PS, PG_LARGE},
	{ KERNBASE, 0, KERN_V2P(text), PTE_P|PTE_W, PG_SMALL},
	{ (uintptr_t)text, KERN_V2P(text), KERN_V2P(data), PTE_P, PG_SMALL},
	{ (uintptr_t)data, KERN_V2P(data), PHYSTOP, PTE_P|PTE_W, PG_SMALL},
	{ KERNDEV, DEVSPACE, DEVSPACE+0x1000000, PTE_P|PTE_W|PTE_PS, PG_LARGE}
};

void *walkpages(uintptr_t *pml5t, uintptr_t vaddr, bool alloc,
				enum page_size pg_sz){

	uintptr_t *pml4t, *pdpt, *pgdir, *pgtab;

	if (pml5t[PML5X(vaddr)] & PTE_P){
		pml4t = MM_P2V(PG_ADDR(pml5t[PML5X(vaddr)]));
	} else {
		if (!alloc || !(pml4t = kmalloc(PGSIZE)))
			return NULL;

		memset(pml4t, 0, PGSIZE);
		pml5t[PML5X(vaddr)] = MM_V2P(pml4t) | PTE_P | PTE_W | PTE_U;
	}

	if (pml4t[PML4X(vaddr)] & PTE_P){
		pdpt = MM_P2V(PG_ADDR(pml4t[PML4X(vaddr)]));
	} else {
		if (!alloc || !(pdpt = kmalloc(PGSIZE)))
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
		if (!alloc || !(pgdir = kmalloc(PGSIZE)))
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
		if (!alloc || !(pgtab = kmalloc(PGSIZE)))
			return NULL;

		memset(pgtab, 0, PGSIZE);
		pgdir[PDX(vaddr)] = MM_V2P(pgtab) | PTE_P | PTE_W | PTE_U;
	}

	return &pgtab[PTX(vaddr)];
}

int mappages(uintptr_t *pml5t, uintptr_t vaddr, uintptr_t paddr,
 			 size_t size, uint8_t flags, enum page_size pg_sz){

	 uintptr_t vend, *pte;

	 vend = vaddr + size;
	 while (vaddr < vend){

	 	if (!(pte = walkpages(pml5t, vaddr, true, pg_sz)))
	 		return -1;

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

void *setup_kernel_page_table(void){

	uintptr_t *pml5t;
	struct kmap *k;

	if (!(pml5t = kmalloc(PGSIZE)))
		return NULL;

	memset(pml5t, 0, PGSIZE);
	for (k = kmap; k < &kmap[NELEM(kmap)]; k++)
		if (mappages(pml5t, k->vaddr, k->pstart, k->pend-k->pstart,
		 k->flags, k->pg_sz) < 0){
			free_page_table(pml5t, 0);
		 	return NULL;
		 }

	return pml5t;
}

void switch_to_kvm(void){
	lcr3(MM_V2P(kpml5t));
}

void kvm_init(void){
	kpml5t = setup_kernel_page_table();
	switch_to_kvm();
}

void uvm_init(uintptr_t *pml5t, void *addr, size_t size){

	uintptr_t *page;

	page = alloc_page();
	memset(page, 0, PGSIZE);
	mappages(pml5t, 0, MM_V2P(page), PGSIZE, PTE_P|PTE_W|PTE_U, PG_SMALL);
	memmove(page, addr, size);
}

int copy_mem(uintptr_t *new_pml5t, uintptr_t *old_pml5t, size_t size){

	uintptr_t vaddr, *pte, *page;
	uint8_t flags;
	struct page_list *pg;

	for (vaddr = 0; vaddr < size; vaddr += PGSIZE){
		pte = walkpages(old_pml5t, vaddr, false, PG_SMALL);
		if (!pte) panic("copy_mem: page should exist");
		page = MM_P2V(PG_ADDR(*pte));
		flags = PG_FLAG(*pte) & ~PTE_W;
		*pte = MM_V2P(page) | flags;
		pg = search_page(page);
		if (!pg) panic("copy_mem: page not found");
		pg->ref++;
		if (mappages(new_pml5t, vaddr, MM_V2P(page), PGSIZE,
			flags, PG_SMALL) < 0) return -1;
	}
	lcr3(MM_V2P(old_pml5t));

	return 0;
}

int page_fault_no_page(uintptr_t *pml5t, uintptr_t addr){

	uintptr_t *page;

	page = alloc_page();
	if (!page) return -1;

	memset(page, 0, PGSIZE);
	if (mappages(pml5t, addr, MM_V2P(page), PGSIZE,
	 PTE_P|PTE_W|PTE_U, PG_SMALL) < 0) return -1;

	return 0;
}

int page_fault_wp_page(uintptr_t *pml5t, uintptr_t addr){

	uintptr_t *pte, *page, *new_page;
	struct page_list *pg;

	pte = walkpages(pml5t, addr, false, PG_SMALL);
	if (!pte) return -1;
	page = MM_P2V(PG_ADDR(*pte));

	pg = search_page(page);
	if (!pg) panic("page_fault_wp_page: page not found");
	if (pg->ref == 1){
		*pte |= PTE_W;
		return 0;
	}
	pg->ref--;

	if (!(new_page = alloc_page())) return -1;
	memset(new_page, 0, PGSIZE);
	if (mappages(pml5t, addr, MM_V2P(new_page), PGSIZE,
		PTE_P|PTE_W|PTE_U, PG_SMALL) < 0) return -1;
	memmove(new_page, page, PGSIZE);

	return 0;
}

void *alloc_page(void){

	void *page;
	struct page_list *pg;

	page = kmalloc(PGSIZE);
	if (!page) return NULL;

	pg = kmalloc(sizeof(struct page_list));
	if (!pg){
		kfree(page, PGSIZE);
		return NULL;
	};

	pg->page = page;
	pg->ref = 1;
	pg->next = used_page;
	used_page = pg;

	return page;
}

void free_page(void *addr){

	struct page_list *pg;

	pg = search_page(addr);
	if (!pg) panic("free_page: page not found");

	if (pg->ref == 0) panic("free_page: freeing free page");

	if (--pg->ref == 0){
		kfree(addr, PGSIZE);
		remove_page_list(pg);
		kfree(pg, sizeof(*pg));
	}
}

struct page_list *search_page(void *addr){

	struct page_list *pg;

	for (pg = used_page; pg; pg = pg->next){
		if (pg->page == addr){
			return pg;
		}
	}

	return NULL;
}

void remove_page_list(struct page_list *pg){

	struct page_list **ppg;

	for (ppg = &used_page; *ppg; ppg = &(*ppg)->next){
		if (*ppg == pg){
			*ppg = pg->next;
			return;
		}
	}
	panic("not found");
}

void free_user_page(uintptr_t *pml5t, size_t size){

	uintptr_t vaddr, *pte, *page;

	for (vaddr = 0; vaddr < size; vaddr += PGSIZE){
		pte = walkpages(pml5t, vaddr, false, PG_SMALL);
		if (!pte) panic("free_user_page: ");
		page = MM_P2V(PG_ADDR(*pte));
		*pte = 0;
		free_page(page);
	}
}

void free_page_table(uintptr_t *pml5t, size_t size){

	free_user_page(pml5t, size);

	uint16_t pml5x, pml4x, pdptx, pdx;
	uintptr_t *pml4t, *pdpt, *pgdir, *pgtab;

	for (pml5x = 0; pml5x < 512; pml5x++){
		if (!(pml5t[pml5x] & PTE_P)) continue;
		pml4t = MM_P2V(PG_ADDR(pml5t[pml5x]));

		for (pml4x = 0; pml4x < 512; pml4x++){
			if (!(pml4t[pml4x] & PTE_P)) continue;
			pdpt = MM_P2V(PG_ADDR(pml4t[pml4x]));

			for(pdptx = 0; pdptx < 512; pdptx++){
				if (!(pdpt[pdptx] & PTE_P)) continue;
				pgdir = MM_P2V(PG_ADDR(pdpt[pdptx]));

				for (pdx = 0; pdx < 512; pdx++){
					if (!(pgdir[pdx] & PTE_P)) continue;

					if (pgdir[pdx] & PTE_PS){
						continue;
					}

					pgtab = MM_P2V(PG_ADDR(pgdir[pdx]));

					kfree(pgtab, PGSIZE);
					pgdir[pdx] = 0;
				}
				kfree(pgdir, PGSIZE);
				pdpt[pdptx] = 0;
			}
			kfree(pdpt, PGSIZE);
			pml4t[pml4x] = 0;
		}
		kfree(pml4t, PGSIZE);
		pml5t[pml5x] = 0;
	}
	kfree(pml5t, PGSIZE);
}
