
#define EXTMEM 0x100000
#define PHYSTOP 0x400000
#define DEVSPACE 0xfe000000

#define KERNBASE 0xffffffff80000000

#define KERNDEV (0xffffffff00000000+DEVSPACE)

#define KERN_P2V(x) (void*)((uint8_t*)(x) + KERNBASE)
#define KERN_V2P(x) ((uintptr_t)(x) - KERNBASE)
#define KERN_P2V_DEV(x) (void*)((uint8_t*)(x) + 0xffffffff00000000)
#define KERN_V2P_WO(x) ((x) - KERNBASE)

#define MMBASE 0xff88888840000000
#define MMPHYSSTART 0x40000000
#define MMPHYSHALF 0x20000000
#define MMPHYSSIZE 0x40000000
#define MMPHYSEND ((uintptr_t)MMPHYSSTART+MMPHYSSIZE)

#define MM_P2V(x) (void*)((uint8_t*)(x) + 0xff88888800000000)
#define MM_V2P(x) ((uintptr_t)(x) - 0xff88888800000000)
