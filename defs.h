
// console.c
void cls(void);
int consolewrite1(char *, size_t );
void consputc(int);
void cprintf(const char*, ...);
void panic(const char*) __attribute__((noreturn));

// gdt.c
void gdt_init(void);
void tss_init(void);

// idt.c
struct registers;

void idt_init(void);
void isr_install(size_t, void (*)(struct registers*));
void picinit(void);

// ioapic.c
void ioapicenable(int , int );
void ioapicinit(void);

// kbd.c
void kbd_init(void);

// lapic.c
void lapic_init(void);
void lapiceoi(void);
void timer_init(void);

// mm.c
void kfree(void*, size_t );
void *kmalloc(size_t);
void mm_init(void*, void*);

// sched.c
struct task;

void kill(uint32_t, uint32_t);
void schedule(void);
void send_signal(struct task *, uint32_t);
void show_all_task(void);
void switch_to_user(void);
int sys_exit(void);
int sys_pause(void);
void task_init(void);

// string.c
void *memmove(void *, const void *, size_t);
void *memset(void *, int , size_t );

// syscall.c
void syscall_init(void);

// uart.c
void uartinit(void);
void uartintr(void);
void uartputc(int);

// vm.c

struct page_list;

void *alloc_page(void);
int copy_mem(uintptr_t*, uintptr_t*, size_t);
void free_page(void *);
void free_page_table(uintptr_t*, size_t);
void kvm_init(void);
int page_fault_no_page(uintptr_t *, uintptr_t);
int page_fault_wp_page(uintptr_t *, uintptr_t);
void remove_page_list(struct page_list*);
struct page_list *search_page(void *);
void *setup_kernel_page_table(void);
void uvm_init(uintptr_t *, void *, size_t);

#define NELEM(x) (sizeof(x)/sizeof(x[0]))
