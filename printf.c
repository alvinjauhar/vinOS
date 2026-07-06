
#include "types.h"
#include "user.h"
#include "va_list.h"

void putc(char c){
	write(&c, 1);
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
		putc(buf[i]);
}

void printf(const char *fmt, ...){

	int c;
	va_list va;

	va_start(va, fmt);
	for (; (c = *fmt & 0xff); fmt++){
		if (c != '%'){
			putc(c);
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
			putc(ch);
			break;
		case 's':
			char *s = va_arg(va, char*);
			if (s == NULL)
				s = "null";
			for (; *s; s++)
				putc(*s);
			break;
		default:
			putc('%');
			putc(c);
			break;
		}
	}

	va_end(va);
}
