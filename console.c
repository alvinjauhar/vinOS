
#include "types.h"
#include "defs.h"
#include "memlayout.h"
#include "x86.h"
#include "va_list.h"

#define CRTPORT 0x3d4
#define BACKSPACE 0x100

static uint16_t *crt = KERN_P2V(0xb8000);

void consputc(int);

void cls(void){

	uint16_t pos = 0;

	memset(crt, 0, sizeof(crt[0])*25*80);

	outb(CRTPORT, 14);
	outb(CRTPORT+1, pos >> 8);
	outb(CRTPORT, 15);
	outb(CRTPORT+1, pos);
}

void printinteger(long xx, int base, bool sign, bool lonng){

	int n;

	if (base == 2 && lonng){
		n = 64;
	} else if (base == 2){
		n = 32;
	} else 
		n = 22;

	char digits[] = "0123456789abcdef";
	char buf[n];
	uint64_t x;
	int i;

	if (sign && (sign = xx < 0))
		x = -xx;
	else
		x = xx;

	i = 0;
	do {
		buf[i++] = digits[x % base];
	} while((x /= base));

	if (sign)
		buf[i++] = '-';
	else if (base == 16){
		buf[i++] = 'x';
		buf[i++] = '0';
	} else if (base == 2){
		buf[i++] = 'b';
		buf[i++] = '0';
	}

	while (--i >= 0)
		consputc(buf[i]);
}

void cprintf(const char *fmt, ...){

	int c;
	va_list va;

	va_start(va, fmt);
	for (; (c = *fmt & 0xff); fmt++){
		if (c != '%'){
			consputc(c);
			continue;
		}

		c = *++fmt & 0xff;
		switch (c){
		case 'd':
			printinteger(va_arg(va, int), 10, true, false);
			break;
		case 'x':
			printinteger(va_arg(va, uint32_t), 16, false, false);
			break;
		case 'p':
			printinteger(va_arg(va, uint64_t), 16, false, false);
			break;
		case 'b':
			printinteger(va_arg(va, uint32_t), 2, false, false);
			break;
		case 'l':
			c = *++fmt & 0xff;
			switch (c){
			case 'd':
				printinteger(va_arg(va, long), 10, true, false);
				break;
			}
			break;
		case 'c':
			char ch = va_arg(va, int);
			consputc(ch);
			break;
		case 's':
			char *s = va_arg(va, char*);
			if (s == NULL)
				s = "null";
			for (; *s; s++)
				consputc(*s);
			break;
		default:
			consputc('%');
			consputc(c);
			break;
		}
	}

	va_end(va);
}

void panic(const char *s){
	cprintf("panic: %s\n", s);
	for (;;);
}

void cgaputc(int c){

	uint16_t pos;

	outb(CRTPORT, 14);
	pos = inb(CRTPORT+1) << 8;
	outb(CRTPORT, 15);
	pos |= inb(CRTPORT+1);

	if (c == '\n')
		pos += 80 - pos%80;
	else if (c == BACKSPACE){
		if (pos > 0) pos--;
	} else
		crt[pos++] = 0x0f00 | c;

	if (pos >= 24*80){
		memmove(crt, crt+80, sizeof(crt[0])*23*80);
		pos -= 80;
		memset(crt+pos, 0, sizeof(crt[0])*(24*80-pos));
	}

	outb(CRTPORT, 14);
	outb(CRTPORT+1, pos >> 8);
	outb(CRTPORT, 15);
	outb(CRTPORT+1, pos);
	crt[pos] = 0x0f00 | ' ';
}

void consputc(int c){
	uartputc(c);
	cgaputc(c);
}

int consolewrite1(char *addr, size_t n){

	for (size_t i = 0; i < n; i++){
		consputc(addr[i] & 0xff);
	}

	return n;
}
