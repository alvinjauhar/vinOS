
struct list {
	struct list *next, *prev;	
};

static inline void list_add(struct list *new, struct list *head){
	head->next->prev = new;
	new->next = head->next;
	new->prev = head;
	head->next = new;
}

static inline void list_del(struct list *entry){
	entry->next->prev = entry->prev;
	entry->prev->next = entry->next;
}

#define list_entry(ptr, type, member) \
	(type*)((uintptr_t)ptr - offsetof(type, member))
