
enum taskstate { TASK_EMBRYO, TASK_RUNNABLE, TASK_RUNNING, TASK_SLEEPING, TASK_ZOMBIE };

struct context {
	uintptr_t rip;	// 0
	uintptr_t rsp;	// 8
	uintptr_t rbp;	// 16
	uintptr_t rbx;	// 24
//	uintptr_t rflags;
	uintptr_t r12;	// 32
	uintptr_t r13;	// 40
	uintptr_t r14;	// 48
	uintptr_t r15;	// 56
};

struct task {
	uint32_t pid;
	long priority;
	long counter;
	enum taskstate state;
	uintptr_t *pml5t;
	size_t size;
	void *kstack;
	struct context context;
	struct registers *regs;
	struct task *parent;
	struct list task;
};

extern struct task *current;
