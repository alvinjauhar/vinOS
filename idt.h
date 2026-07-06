
#define T_IRQ0 32

#define IRQ_TIMER 	0
#define IRQ_KBD		1
#define IRQ_COM1	4
#define IRQ_IDE	   14
#define IRQ_ERROR  19
#define IRQ_SPURIOUS 31

struct idt_entry {
	uint16_t offset_low;
	uint16_t segment_selector;
	uint8_t ist;
	uint8_t gate_type;
	uint16_t offset_middle;
	uint32_t offset_high;
	uint32_t reserved;
};

struct idt_pointer {
	uint16_t size;
	uintptr_t base;	
} __attribute__((packed));

struct registers {
	uintptr_t r15, r14, r13, r12, r11, r10, r9, r8;
	uintptr_t rbp, rdi, rsi, rdx, rcx, rbx, rax;
	uintptr_t trap_no, error_code;
	uintptr_t rip, cs, rflags, rsp, ss;	
};
