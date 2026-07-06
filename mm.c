
#include "types.h"
#include "defs.h"
#include "mmu.h"
#include "kernel.h"

#define MIN_BLOCK_SIZE 8
#define MAX_ORDER 9

struct block {
	struct block *next;
};

static struct block *free_list[MAX_ORDER+1];

size_t log_2(size_t n){

	size_t order, val;

	if (n <= 1)
		return 0;

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

static size_t index_to_bytes(size_t i){

	size_t order = i + log_2(MIN_BLOCK_SIZE);

	return 1 << order;
}

static void list_push(void *addr, size_t order){

	struct block *block;

	block = addr;
	block->next = free_list[order];
	free_list[order] = block;
}

static void *list_pop(size_t order){

	struct block *block;

	if (free_list[order] == NULL)
		return NULL;

	block = free_list[order];
	free_list[order] = block->next;

	return block;
}

static void list_remove(void *addr, size_t order){

	struct block *block, **curr;

	block = addr;
	for (curr = &free_list[order]; *curr; curr = &(*curr)->next){
		if (*curr == block){
			*curr = block->next;
			return;
		}
	}
	
}

static bool list_contains(void *addr, size_t order){

	struct block *curr;

	for (curr = free_list[order]; curr; curr = curr->next){
		if (curr == (struct block*)addr){
			return true;
		}
	}

	return false;
}

void mm_init(void *start, void *end){

	uint8_t *p;
	size_t n;

	n = 0;
	for (p = start; p < (uint8_t*)end; p += PGSIZE, n++){
		list_push(p, MAX_ORDER);
	}
	cprintf("free %d pages\n", n);
}

void mm_debug(void){
	for (size_t i = 0; i <= MAX_ORDER; i++){
		cprintf("mm debug: %d %p \n", i, free_list[i]);
	}
}

void kfree(void *addr, size_t nbytes){

	size_t i, index;
	struct block *p, *q;
	bool freeing;

	index = bytes_to_index(nbytes);

	for (i = index, p = addr; i <= MAX_ORDER; i++, p = min(p, q)){
		freeing = true;
		q = (void*)((uintptr_t)p ^ index_to_bytes(i));

		if (i < MAX_ORDER && list_contains(q, i)){
			freeing = false;
			list_remove(q, i);
		}

		if (freeing){
			list_push(p, i);
			return;
		}
	}

}

void *kmalloc(size_t nbytes){

	size_t i, j, index;
	struct block *p, *q;

	if (nbytes >= 1 || nbytes <= MIN_BLOCK_SIZE)
		nbytes = (nbytes + MIN_BLOCK_SIZE-1) & ~(MIN_BLOCK_SIZE-1);

	index = bytes_to_index(nbytes);

	for (i = index; i <= MAX_ORDER; i++){
		if (free_list[i]){
			p = list_pop(i);
			if (i == index){
				return p;
			}

			j = i;

			while (j-- > index){
				q = (void*)((uintptr_t)p ^ index_to_bytes(j));
				list_push(q, j);

				if (j == index){
					return p;
				}
			}

		}
	}

	return NULL;
}

