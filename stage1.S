
.code16
.globl start
start:
	cli

	mov $0x2,%ah
	mov $26,%al
	mov $2,%cl
	mov $0x7e00,%bx
	int $0x13

	jmp stage2

print:
	mov (%si),%al
	inc %si
	or %al,%al
	jz print_end
	mov $0xe,%ah
	int $0x10
	jmp print
print_end:
	ret

.zero 510-(.-start)
.word 0xaa55

#; ah 02h
#; al sector to read count
#; ch cylinder
#; cl sector
#; dh head
#; dl drive
#; es:bx buffer address pointer
