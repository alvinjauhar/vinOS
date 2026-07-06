
struct gdt_entry {
	uint16_t limit;
	uint16_t base_low;
	uint8_t base_middle;
	uint8_t access;
	uint8_t flags;
	uint8_t base_high;
};

struct tss_entry {
	uint32_t reserved1;
	uint64_t rsp[3];
	uint64_t reserved2;
	uint64_t ist[7];
	uint64_t reserved3;
	uint16_t reserved4;
	uint16_t iopb;
} __attribute__((packed));

struct gdt_entry_high {
	uint32_t base_highest;
	uint32_t reserved;	
};

struct gdt_pointer {
	uint16_t size;
	uintptr_t base;
} __attribute__((packed));

struct gdt {
	struct gdt_entry entry[7];
	struct gdt_entry_high entry_high;
	struct tss_entry tss;
	struct gdt_pointer pointer;
};

extern struct gdt gdt;
