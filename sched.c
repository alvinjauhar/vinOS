
#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "gdt.h"
#include "kernel.h"
#include "list.h"
#include "sched.h"
#include "idt.h"
#include "x86.h"
#include "msr.h"

struct task head_task, init_task, *current;

static struct {
	uintptr_t kern_stack;
	uintptr_t user_stack;
} syscall_stack;

static uint32_t next_pid = 1;

void task_init(void){

	extern char _binary_initcode_start[], _binary_initcode_size[];
	uintptr_t stack;

	head_task.task.next = &head_task.task;
	head_task.task.prev = &head_task.task;
	list_add(&init_task.task, &head_task.task);

	init_task.pid = next_pid++;
	init_task.priority = 5;
	init_task.pml5t = uvm_init(_binary_initcode_start, (size_t)_binary_initcode_size);

	init_task.kstack = kmalloc(PGSIZE);

	init_task.size = PGSIZE;

	wrmsr(KERNEL_GS_BASE, (uintptr_t)&syscall_stack);

	stack = (uintptr_t)init_task.kstack + KSTACKSIZE;
	syscall_stack.kern_stack = gdt.tss.rsp[0] = stack;
	
	stack -= sizeof(struct registers);

	init_task.regs = (void*)stack;
	init_task.regs->ss = (4 << 3) | DPL_USER;  
	init_task.regs->rsp = PGSIZE;
	init_task.regs->rflags = FL_IF;
	init_task.regs->cs = (5 << 3) | DPL_USER;
	init_task.regs->rip = 0;

	current = &init_task;
	init_task.state = TASK_RUNNING;
}

void switch_to_user(void){

	asm volatile("mov %0,%%cr3\n" 
				 "push %1\n" 
				 "push %2\n" 
				 "push %3\n" 
				 "push %4\n" 
				 "push %5\n" 
				 "iretq" 
				 :: "r" (MM_V2P(init_task.pml5t)) ,"r" (init_task.regs->ss), 
				 "r" (init_task.regs->rsp), "r" (init_task.regs->rflags),
				 "r" (init_task.regs->cs), "r" (init_task.regs->rip));
}

void fork_return(void){
	lapiceoi();
}

void fork_return1(void){

	asm volatile("pop %r15\n"
				 "pop %r14\n"
				 "pop %r13\n"
				 "pop %r12\n"
				 "pop %r11\n"
				 "pop %r10\n"
				 "pop %r9\n"
				 "pop %r8\n"

				 "pop %rbp\n"
				 "pop %rdi\n"
				 "pop %rsi\n"
				 "pop %rdx\n"
				 "pop %rcx\n"
				 "pop %rbx\n"
				 "pop %rax\n"
				 
				 "add $16,%rsp\n"
				 "iretq\n"
				 );
}

int fork(void){

	struct task *newtask, *oldtask;
	uintptr_t stack;
	uint32_t pid;

	oldtask = current;
	newtask = kmalloc(sizeof(struct task));
	if (!newtask) return -1;

	list_add(&newtask->task, &head_task.task);
	pid = newtask->pid = next_pid++;
	newtask->priority = oldtask->priority;

	newtask->kstack = kmalloc(PGSIZE);
	if (!newtask->kstack) return -1;

	newtask->size = oldtask->size;

	newtask->pml5t = setupkvm();
	if (!newtask->pml5t) return -1;

	if (copy_mem(newtask->pml5t, oldtask->pml5t, oldtask->size) < 0)
		return -1;

	stack = (uintptr_t)newtask->kstack + KSTACKSIZE;
	stack -= sizeof(struct registers);

	newtask->regs = (void*)stack;
	memmove(newtask->regs, oldtask->regs, sizeof(struct registers));

	newtask->regs->rax = 0;

	stack -= 8;
	*(uintptr_t*)stack = (uintptr_t)fork_return1;

	newtask->context.rsp = (uintptr_t)stack;
	newtask->context.rip = (uintptr_t)fork_return;

	newtask->parent = oldtask;
	newtask->state = TASK_RUNNABLE;

	return pid;
}

int wait(void){

	return 0;
}

void exit(void){

	if (current == &init_task)
		panic("init exiting");

	current->state = TASK_ZOMBIE;

	schedule();
}

void switch_to(struct context *, struct context *);

void switch_to(struct context *, struct context *){
	asm volatile("pop 0(%rdi)\n" \
				 "mov %rsp,8(%rdi)\n" \
				 "mov %rbp,16(%rdi)\n" \
				 "mov %rbx,24(%rdi)\n" \
				 "mov %r12,32(%rdi)\n" \
				 "mov %r13,40(%rdi)\n" \
				 "mov %r14,48(%rdi)\n" \
				 "mov %r15,56(%rdi)\n" \

				 "mov 56(%rsi),%r15\n" \
				 "mov 48(%rsi),%r14\n" \
				 "mov 40(%rsi),%r13\n" \
				 "mov 32(%rsi),%r12\n" \
				 "mov 24(%rsi),%rbx\n" \
				 "mov 16(%rsi),%rbp\n" \
				 "mov 8(%rsi),%rsp\n" \
				 "push 0(%rsi)");
}

void schedule(void){

	struct task *p, *next, *temp;
	long highest_counter;
	uintptr_t stack;

	while (true){

		highest_counter = -1;
		next = NULL;
		for (p = list_entry(head_task.task.next, struct task, task.next);
			 p != &head_task;
			 p = list_entry(p->task.next, struct task, task.next)){

			if (p->state == TASK_RUNNABLE && p->counter > highest_counter){
	
				highest_counter = p->counter;
				next = p;
			}
		}
		if (highest_counter){
			break;
		}

		for (p = list_entry(head_task.task.next, struct task, task.next);
			 p != &head_task;
			 p = list_entry(p->task.next, struct task, task.next)){
		 
			p->counter += p->priority;
		}
	}

	stack = (uintptr_t)next->kstack + KSTACKSIZE;
	syscall_stack.kern_stack = gdt.tss.rsp[0] = stack;

	asm volatile("mov %0,%%cr3" :: "r" (MM_V2P(next->pml5t)));

	next->state = TASK_RUNNING;
	temp = current;
	current = next;
	switch_to(&temp->context, &next->context);
}

void show_all_task(void){

	struct task *p;

	char *state[] = {
		[TASK_EMBRYO] "embryo", 
		[TASK_RUNNABLE] "runnable",
		[TASK_RUNNING] "running",
		[TASK_SLEEPING] "sleeping",
		[TASK_ZOMBIE] "zombie"
	};

	for (p = (list_entry(head_task.task.next, struct task, task.next));
		 p != &head_task;
	 	 p = list_entry(p->task.next, struct task, task.next)){

		cprintf("pid %d state %s counter %d \n",
		 p->pid, state[p->state], p->counter);
	}
}
