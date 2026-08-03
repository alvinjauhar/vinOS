
.code16
.globl start
start:
	cli

	mov $2,%ah
	mov $2,%al
	mov $2,%cl
	mov $0x7e00,%bx
	int $0x13

	jmp stage2

.zero 510-(.-start)
.word 0xaa55

#; ah 02h
#; al sector to read count
#; ch cylinder
#; cl sector
#; dh head
#; dl drive
#; es:bx buffer address pointer
