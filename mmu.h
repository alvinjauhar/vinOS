
#define FL_IF 0x00000200

#define DPL_USER 0x3

#define PGSIZE 0x1000
#define PGSIZE_LARGE 0x200000
#define PGSIZE_HUGE 0x40000000

#define PTE_P 	0x001
#define PTE_W	0x002
#define PTE_U	0x004
#define PTE_PS	0x080

#define PGUP(x) (((uintptr_t)(x) + 0xfff) & ~0xfff)
#define PGDN(x) ((uintptr_t)(x) & ~0xfff)

#define PML5SHIFT 48
#define PML4SHIFT 39
#define PDPTSHIFT 30
#define PDSHIFT 21
#define PTSHIFT 12

#define PML5X(x) (((uintptr_t)(x) >> PML5SHIFT) & 0x1ff)
#define PML4X(x) (((uintptr_t)(x) >> PML4SHIFT) & 0x1ff)
#define PDPTX(x) (((uintptr_t)(x) >> PDPTSHIFT) & 0x1ff)
#define PDX(x) (((uintptr_t)(x) >> PDSHIFT) & 0x1ff)
#define PTX(x) (((uintptr_t)(x) >> PTSHIFT) & 0x1ff)

#define PG_ADDR(x) ((uintptr_t)(x) & ~0xfff)
#define PG_FLAG(x) ((uintptr_t)(x) &  0xfff)
