
C_OBJS := \
	console.o \
	gdt.o \
	idt.o \
	ioapic.o \
	kbd.o \
	lapic.o \
	main.o \
	mm.o \
	sched.o \
	string.o \
	syscall.o \
	sysfile.o \
	systask.o \
	uart.o \
	vm.o \

S_OBJS := entry.o isr.o vectors.o

OBJS := $(S_OBJS) $(C_OBJS)

CC = /opt/x86_64-elf/bin/x86_64-elf-gcc
LD = /opt/x86_64-elf/bin/x86_64-elf-ld
OBJCOPY = /opt/x86_64-elf/bin/x86_64-elf-objcopy
OBJDUMP = /opt/x86_64-elf/bin/x86_64-elf-objdump

CFLAGS = -fno-builtin -Wall -fno-omit-frame-pointer -Werror -fPIC
LDFLAGS = 

vin.img: boot kernel
	dd if=/dev/zero of=vin.img count=10000
	dd if=boot of=vin.img conv=notrunc
	dd if=kernel of=vin.img seek=27 conv=notrunc

%.o: %.c
	$(CC) $(CFLAGS) -O -I. -c $< -o $@

%.o: %.S
	$(CC) $(CFLAGS) -I. -c $< -o $@

boot: stage1.o stage2.o bootmain.o
	$(LD) $(LDFLAGS) -N -e start -Ttext 0x7c00 -o boot.out stage1.o stage2.o bootmain.o
	$(OBJCOPY) -S -O binary -j .text boot.out boot

vectors.S: vectors.pl
	./vectors.pl > vectors.S

initcode: init.o usys.o printf.o
	$(LD) $(LDFLAGS) -N -e start -Ttext 0 -o init.out init.o usys.o printf.o
	$(OBJCOPY) -S -O binary init.out initcode
	

kernel: $(OBJS) initcode kernel.ld
	$(LD) $(LDFLAGS) -T kernel.ld -o kernel $(OBJS) -b binary initcode

qemu: vin.img
	qemu-system-x86_64 -cpu qemu64,+la57 -serial mon:stdio -drive file=vin.img,index=0,media=disk,format=raw -m 2G

clean:
	rm -f *.o *.out boot vectors.S initcode kernel *.img
