
#include "types.h"
#include "x86.h"

void *memset(void *addr, int c, size_t n){

	c &= 0xff;

	if ((uintptr_t)addr % 8 == 0 && n % 8 == 0){
		uint64_t fill = c;
		fill = fill << 56 | fill << 48 | fill << 40 | fill << 32
			 | fill << 24 | fill << 16 | fill << 8 | fill;
		stosq(addr, fill, n / 8);
	} else if ((uintptr_t)addr % 4 == 0 && n % 4 == 0){
		uint32_t fill = c;
		fill = fill << 24 | fill << 16 | fill << 8 | fill;
		stosl(addr, fill, n / 4);
	} else if ((uintptr_t)addr % 2 == 0 && n % 2 == 0){
		uint16_t fill = c;
		fill = fill << 8 | fill;
		stosw(addr, fill, n / 2);
	} else
		stosb(addr, c, n);

	return addr;
}

void *memmove(void *dst, const void *src, size_t n){

	uint8_t *d = dst;
	const uint8_t *s = src;

	if (d == s){
		return d;
	}

	if (s < d && s + n > d){
		d += n, s += n;

		if ((uintptr_t)d % 8 == (uintptr_t)s % 8){

			while ((uintptr_t)s % 8){

				if (n-- == 0){
					return dst;
				}

				*--d = *--s;
			}

			while (n >= 8){
				n -= 8, d -= 8, s -= 8;
				*(uintptr_t*)d = *(uintptr_t*)s;
			}
		}

		while (n-- > 0)
			*--d = *--s;
	} else {

		if ((uintptr_t)d % 8 == (uintptr_t)s % 8){

			while ((uintptr_t)s % 8){

				if (n-- == 0){
					return dst;
				}

				*d++ = *s++;
			}

			for (; n >= 8; n -= 8, d += 8, s += 8)
				*(uintptr_t*)d = *(uintptr_t*)s;
		}

		while (n-- > 0)
			*d++ = *s++;
	}

	return dst;
}
