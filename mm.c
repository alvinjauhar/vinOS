
// mm.c
// Kernel memory allocator
// Implements kmalloc and kfree using the buddy algorithm.

#include "types.h"
#include "defs.h"
#include "mmu.h"
#include "kernel.h"

#define MIN_BLOCK_SIZE 8
#define MAX_INDEX 18

struct block {
	struct block *next;
};

static struct block *free_list[MAX_INDEX+1];

size_t log_2(size_t n){

	if (n == 0) panic("log_2: invalid input");

	size_t order, val;

	order = 0;
	val = 1;

	while (val < n){
		val <<= 1;
		order++;
	}

	return order;
}

static size_t bytes_to_index(size_t bytes){
	return log_2(bytes) - log_2(MIN_BLOCK_SIZE);
}

static size_t index_to_bytes(size_t index){
	return 1 << (index + log_2(MIN_BLOCK_SIZE));
}

static void list_push(void *addr, size_t index){

	if ((uintptr_t)addr % index_to_bytes(index)) panic("list_push");

	struct block *block;

	block = addr;
	block->next = free_list[index];
	free_list[index] = block;
}

static void *list_pop(size_t index){

	struct block *block;

	if (!(block = free_list[index]))
		return NULL;

	free_list[index] = block->next;

	return block;
}

static void list_remove(void *addr, size_t index){

	struct block **curr;

	for (curr = &free_list[index]; *curr; curr = &(*curr)->next){
		if (*curr == (struct block*)addr){
			*curr = (*curr)->next;
			return;
		}
	}
	panic("list remove: list not found");
}

static bool list_contains(void *addr, size_t index){

	struct block *curr;

	for (curr = free_list[index]; curr; curr = curr->next)
		if (curr == (struct block*)addr)
			return true;

	return false;
}

void mm_init(void *start, void *end){

	uint8_t *p;
	size_t n;

	n = 0;
	for (p = start; p < (uint8_t*)end; p += PGSIZE_LARGE, n++)
		list_push(p, MAX_INDEX);

	cprintf("free %d large pages\n", n);
}

void mm_debug(void){
	for (size_t i = 0; i <= MAX_INDEX; i++){
		cprintf("mm debug: %d %p \n", i, free_list[i]);
	}
}

void kfree(void *addr, size_t nbytes){

	size_t i;
	struct block *p, *q;

	i = bytes_to_index(nbytes);
	p = addr;

	for (; i <= MAX_INDEX; i++, p = min(p, q)){

		q = (void*)((uintptr_t)p ^ index_to_bytes(i));

		if (i == MAX_INDEX || !list_contains(q, i)){
			list_push(p, i);
			return;
		}

		list_remove(q, i);
	}
}

void *kmalloc(size_t nbytes){

	size_t index, i;
	struct block *p, *q;

	if (nbytes >= 1 || nbytes <= MIN_BLOCK_SIZE)
		nbytes = (nbytes + MIN_BLOCK_SIZE-1) & ~(MIN_BLOCK_SIZE-1);

	index = bytes_to_index(nbytes);
	p = NULL;

	for (i = index; i <= MAX_INDEX; i++)
		if ((p = list_pop(i))) break;

	while (p && i-- > index){
		q = (void*)((uintptr_t)p ^ index_to_bytes(i));
		list_push(q, i);
	}

	return p;
}
