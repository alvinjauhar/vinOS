
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
#include "signal.h"

struct task head_task, idle_task;
struct task *current_task;

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
	list_add(&idle_task.task, &head_task.task);

	idle_task.pid = 0;
	idle_task.kstack = kmalloc(PGSIZE);
	idle_task.size = PGSIZE;
	idle_task.pml5t = setup_kernel_page_table();
	uvm_init(idle_task.pml5t, 
	_binary_initcode_start, (size_t)_binary_initcode_size);

	wrmsr(KERNEL_GS_BASE, (uintptr_t)&syscall_stack);
	stack = (uintptr_t)idle_task.kstack + KSTACKSIZE;
	syscall_stack.kern_stack = gdt.tss.rsp[0] = stack;

	stack -= sizeof(struct registers);
	idle_task.regs = (void*)stack;
	idle_task.regs->ss = (4 << 3) | DPL_USER;  
	idle_task.regs->rsp = PGSIZE;
	idle_task.regs->rflags = FL_IF;
	idle_task.regs->cs = (5 << 3) | DPL_USER;
	idle_task.regs->rip = 0;

	current_task = &idle_task;
}

void switch_to_user(void){

	asm volatile("\n"
				 "mov %0,%%cr3\n" 
				 "push %1\n" 
				 "push %2\n" 
				 "push %3\n" 
				 "push %4\n" 
				 "push %5\n" 
				 "iretq\n" :: 
				 "r" (MM_V2P(idle_task.pml5t)) ,"r" (idle_task.regs->ss), 
				 "r" (idle_task.regs->rsp), "r" (idle_task.regs->rflags),
				 "r" (idle_task.regs->cs), "r" (idle_task.regs->rip));
}

void fork_return(void){
	if (rdmsr(KERNEL_GS_BASE) != (uintptr_t)&syscall_stack){
		asm volatile("swapgs");
	}
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

int sys_fork(void){

	struct task *newtask, *oldtask;
	uintptr_t stack;
	uint32_t pid;

	oldtask = current_task;
	if (!(newtask = kmalloc(sizeof(struct task)))) return -1;

	list_add(&newtask->task, &head_task.task);
	pid = newtask->pid = next_pid++;
	newtask->priority = 5;
	newtask->size = oldtask->size;

	if (!(newtask->kstack = kmalloc(PGSIZE))) return -1;
	if (!(newtask->pml5t = setup_kernel_page_table())) return -1;

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

	newtask->parent = oldtask->pid;
	newtask->state = TASK_RUNNABLE;

	return pid;
}

int sys_wait(void){

	struct task *p, *curr = current_task;
	bool have_child = false;
	uint32_t pid;

	while (true){
		have_child = false;
		for_all_task_except(p, &idle_task){
			if (p->parent != curr->pid) continue;

			have_child = true;
			if (p->state == TASK_ZOMBIE){
			 	pid = p->pid;
			 	p->state = 0;
			 	free_page_table(p->pml5t, p->size);
			 	kfree(p->kstack, PGSIZE);
			 	list_del(&p->task);
			 	kfree(p, sizeof(*p));

			 	return pid;
			 }
		}

		if (!have_child){
			return -1;
		}

		sys_pause();
		curr->signal &= ~(1 << (SIGCHLD-1)); 
	}
}

int sys_exit(void){

	struct task *p;

	if (current_task == &idle_task)
		panic("idle exiting");

	kill(current_task->parent, SIGCHLD);

	for_all_task_except(p, &idle_task){
		if (p->parent == current_task->pid){
			p->parent = 1;
		 	kill(1, SIGCHLD);
		 }
	}

	current_task->state = TASK_ZOMBIE;
	schedule();
	return -1;
}

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

	struct task *p, *next, *curr = current_task;
	long highest_counter;
	uintptr_t stack;

	for_all_task_except(p, &idle_task){
		if (p->signal && p->state == TASK_INTERRUPTIBLE){
			p->state = TASK_RUNNABLE;
		}
	}

	while (true){
		highest_counter = -1;
		next = NULL;
		for_all_task_except(p, &idle_task){
			if (p->state == TASK_RUNNABLE &&
			 p->counter > highest_counter){
				highest_counter = p->counter;
				next = p;
			}
		}
		if (highest_counter){
			break;
		}

		for_all_task_except(p, &idle_task){
			p->counter += p->priority;
		}
	}

	if (!next) next = &idle_task;

	stack = (uintptr_t)next->kstack + KSTACKSIZE;
	syscall_stack.kern_stack = gdt.tss.rsp[0] = stack;
	lcr3(MM_V2P(next->pml5t));

	current_task = next;
	current_task->state = TASK_RUNNING;	
	switch_to(&curr->context, &next->context);
}

void show_all_task(void){

	struct task *p;

	char *state[] = {
		[TASK_EMBRYO] "embryo", 
		[TASK_RUNNABLE] "runnable",
		[TASK_RUNNING] "running",
		[TASK_INTERRUPTIBLE] "interruptible",
		[TASK_UNINTERRUPTIBLE] "uninterruptible",
		[TASK_ZOMBIE] "zombie"
	};
	cprintf("current pid %d \n", current_task->pid);
	for_all_task(p){
		cprintf("pid %d state %s counter %d rsp %p rip %p\n",
		 p->pid, state[p->state], p->counter,
		  p->regs->rsp, p->regs->rip);
	}
	cprintf("\n");
}

void send_signal(struct task *p, uint32_t signal){

	if (!p || signal < 1 || signal > 32) return;

	p->signal |= 1 << (signal - 1);
}

void kill(uint32_t pid, uint32_t signal){

	struct task *p;

	for_all_task_except(p, &idle_task){
		if (p->pid == pid){
			send_signal(p, signal);
		}
	}
}

int sys_getpid(void){
	return current_task->pid;
}

int sys_pause(void){
	current_task->state = TASK_INTERRUPTIBLE;
	schedule();
	return 0;
}
