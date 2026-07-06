
#include "types.h"
#include "defs.h"
#include "kbd.h"
#include "idt.h"
#include "x86.h"

#define BACKSPACE 0x100

#define C(x) ((x) - '@')

int kbdgetc(void){

	uint32_t stat, data, c;

	stat = inb(KBSTATP);
	if (!(stat & KBS_DIB))
		return -1;

	data = inb(KBDATAP);

	if (data & 0x80){
		return -1;
	}

	c = normalmap[data];

	return c;
}

void kbdintr1(int (*getc)(void)){

	int c;
	bool show_task = false;

	while ((c = getc()) >= 0){
		switch (c){
		case C('H'):
			consputc(BACKSPACE);
			break;
		case '=':
			show_task = true;
			break;
		default:
			consputc(c);
			break;
		}
	}

	if (show_task){
		show_all_task();
	}
}

void kbdintr(struct registers*){
	kbdintr1(kbdgetc);
	lapiceoi();
}

void kbd_init(void){
	ioapicenable(IRQ_KBD, 0);
	isr_install(T_IRQ0+IRQ_KBD, kbdintr);
}
