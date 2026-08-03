
#define PHYSTOP 0x400000
#define DEVSPACE 0xfe000000

#define KERNBASE 0xffffffff80000000

#define KERNDEV (0xffffffff00000000+DEVSPACE)

#define KERN_P2V(x) (void*)((uintptr_t)(x) + KERNBASE)
#define KERN_V2P(x) ((uintptr_t)(x) - KERNBASE)
#define KERN_P2V_DEV(x) (void*)((uintptr_t)(x) + 0xffffffff00000000)
#define KERN_V2P_WO(x) ((x) - KERNBASE)

#define MMVIRTSTART 0xff88888810000000
#define MMPHYSSTART 0x10000000
#define MMPHYSSIZE 0x30000000
#define MMVIRTEND (MMVIRTSTART+MMPHYSSIZE)
#define MMPHYSEND ((uintptr_t)MMPHYSSTART+MMPHYSSIZE)

#define MM_P2V(x) (void*)((uintptr_t)(x) + 0xff88888800000000)
#define MM_V2P(x) ((uintptr_t)(x) - 0xff88888800000000)
