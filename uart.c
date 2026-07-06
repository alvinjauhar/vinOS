

#include "types.h"
#include "defs.h"
#include "x86.h"

#define COM1 0x3f8

int uart;

void uartinit(void){
	outb(COM1+2, 0);

	outb(COM1+3, 0x80);
	outb(COM1+0, 115200/9600);
	outb(COM1+1, 0);
	outb(COM1+3, 0x03);
	outb(COM1+4, 0);
	outb(COM1+1, 0x01);

	if (inb(COM1+5)==0xff)
		return;
	uart = 1;

	inb(COM1+2);
	inb(COM1+0);
}

void uartputc(int c){

	int i;

	if (!uart)
		return;

	for (i=0;i<128&&!(inb(COM1+5)&0x20);i++){
		
	}

	outb(COM1+0, c);
}

int uartgetc(void){
	if (!uart)
		return -1;
	if (!(inb(COM1+5)&0x01))
		return -1;
	return inb(COM1+0);		
}

void uartintr(void){
//	consintr(uartgetc);
}
