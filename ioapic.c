
#include "types.h"
#include "memlayout.h"
#include "idt.h"

#define IOAPIC 0xfec00000
#define REG_TABLE 0x10

struct ioapic {
	uint32_t reg;
	uint32_t pad[3];
	uint32_t data;	
};

volatile struct ioapic *ioapic;

uint32_t ioapicread(uint32_t reg){
	ioapic->reg = reg;
	return ioapic->data;
}

void ioapicwrite(uint32_t reg, uint32_t data){
	ioapic->reg = reg;
	ioapic->data = data;
}

void ioapicinit(void){
	ioapic = KERN_P2V_DEV(IOAPIC);
}

void ioapicenable(uint32_t irq, uint32_t cpunum){
	ioapicwrite(REG_TABLE+2*irq, T_IRQ0+irq);
	ioapicwrite(REG_TABLE+2*irq+1, cpunum << 24);
}
