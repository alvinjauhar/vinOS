
#include "types.h"
#include "defs.h"
#include "memlayout.h"
#include "idt.h"
#include "list.h"
#include "sched.h"

#define LAPIC 0xfee00000

#define EOI 		(0x00b0/4)	// EOI

#define TIMER		(0x0320/4)	// local vector table 0 (TIMER)
#define X1			0x0000000b	// divide counts by 1
#define PERIODIC 	0x00020000	// periodic
#define TICR		(0x0380/4)	// timer initial count
#define TDCR		(0x03e0/4)	// timer divide configuration

volatile uint32_t *lapic;
uint64_t ticks;


void lapicw(uint32_t index, uint32_t value){
	lapic[index] = value;
}

void lapic_init(void){
	lapic = KERN_P2V_DEV(LAPIC);

	lapicw(TDCR, X1);
	lapicw(TIMER, PERIODIC | (T_IRQ0 + IRQ_TIMER));
	lapicw(TICR, 10000000);
}

void lapiceoi(void){
	lapicw(EOI, 0);
}
/*
void timer_handler(struct registers *){

	if (current && --current->counter > 0) {
		lapiceoi();
		return;
	}

	if (current){
		current->state = TASK_RUNNABLE;
		cprintf("timer %d \n", current->counter);
		schedule();
	}
	lapiceoi();
}
*/

void timer_handler(struct registers *){

	ticks++;

	if (current && current->counter-- == 0){
		current->counter = 0;
		current->state = TASK_RUNNABLE;
		schedule();
	}

	lapiceoi();
}

void timer_init(void){
	isr_install(T_IRQ0+IRQ_TIMER, timer_handler);
}
