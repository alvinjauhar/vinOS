
enum taskstate { TASK_EMBRYO, TASK_RUNNABLE, TASK_RUNNING,
				 TASK_INTERRUPTIBLE, TASK_UNINTERRUPTIBLE, TASK_ZOMBIE };

struct context {
	uintptr_t rip;	// 0
	uintptr_t rsp;	// 8
	uintptr_t rbp;	// 16
	uintptr_t rbx;	// 24
	uintptr_t r12;	// 32
	uintptr_t r13;	// 40
	uintptr_t r14;	// 48
	uintptr_t r15;	// 56
};

struct task {
	uint32_t pid;
	uint32_t parent;
	uint32_t priority;
	uint32_t counter;
	uint32_t signal;
	enum taskstate state;
	uintptr_t *pml5t;
	size_t size;
	void *kstack;
	struct context context;
	struct registers *regs;
	struct list task;
};

extern struct task *current_task, idle_task;

#define for_all_task(p)	\
	for (p = list_entry(head_task.task.next, struct task, task.next); \
		 p != &head_task; \
		 p = list_entry(p->task.next, struct task, task.next))

#define for_all_task_except(p, q) \
	for (p = list_entry(head_task.task.next, struct task, task.next); \
		 p != &head_task && p != q; \
		 p = list_entry(p->task.next, struct task, task.next))
