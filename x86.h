
static inline uint8_t inb(uint16_t port){
	uint8_t data;
	asm volatile("inb %1,%0" : "=a" (data) : "d" (port));
	return data;
}

static inline void outb(uint16_t port, uint8_t data){
	asm volatile("outb %1,%0" :: "d" (port), "a" (data));
}

static inline void insl(uint16_t port, void *addr, size_t n){
	asm volatile("rep insl" :: "d" (port), "D" (addr), "c" (n));
}

static inline void stosb(void *addr, uint8_t data, size_t n){
	asm volatile("cld; rep stosb" :
				 "=D" (addr), "=c" (n) :
				 "0" (addr), "a" (data), "1" (n) :
				 "memory", "cc");
}

/*
static inline void stosb(void *addr, uint8_t data, size_t n){
	asm volatile("cld; rep stosb" ::
				 "D" (addr), "a" (data), "c" (n) :
				 "memory", "cc");
}
*/

static inline void stosw(void *addr, uint16_t data, size_t n){
	asm volatile("cld; rep stosw" :
				 "=D" (addr), "=c" (n) :
				 "0" (addr), "a" (data), "1" (n) :
				 "memory", "cc");
}

static inline void stosl(void *addr, uint32_t data, size_t n){
	asm volatile("cld; rep stosl" :
				 "=D" (addr), "=c" (n) :  
				 "0" (addr), "a" (data), "1" (n) :
				 "memory", "cc");
}

static inline void stosq(void *addr, uint64_t data, size_t n){
	asm volatile("cld; rep stosq" :
				 "=D" (addr), "=c" (n) :
				 "0" (addr), "a" (data), "1" (n) :
				 "memory", "cc");
}

static inline void lcr3(uintptr_t addr){
	asm volatile("mov %%rax,%%cr3" :: "a" (addr));
}

static inline uint64_t rcr2(void){
	uint64_t addr;
	asm volatile("mov %%cr2,%0" : "=r" (addr));
	return addr;
}

static inline void cli(void){
	asm volatile("cli");
}

static inline void sti(void){
	asm volatile("sti");
}

static inline void hlt(void){
	asm volatile("hlt");
}

static inline uint64_t rdmsr(uint32_t msr){
	uint32_t high, low;

	asm volatile("rdmsr" : "=a" (low), "=d" (high) : "c" (msr));

	return (uint64_t)high << 32 | low;
}

static inline void wrmsr(uint32_t msr, uint64_t data){

	uint32_t high, low;

	asm volatile("rdmsr" : "=a" (low), "=d" (high) : "c" (msr));

	low |= data;
	high |= data >> 32;

	asm volatile("wrmsr" :: "a" (low), "d" (high), "c" (msr));
}

static inline uint64_t readrflags(void){
	uint64_t flags;
	asm volatile("pushfq; pop %0" : "=r" (flags));
	return flags;
}
