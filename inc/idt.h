#include <inc/types.h>
#include <inc/memlayout.h>

typedef struct _IDTDescr{
   uint16_t offset_1; // offset bits 0..15
   uint16_t selector; // a code segment selector in GDT or LDT
   uint8_t zero;      // unused, set to 0
   uint8_t type_attr; // type and attributes, see below
   uint16_t offset_2; // offset bits 16..31
}__attribute__((__packed__)) IDTDescr;

typedef struct _IDT{
	uint16_t limit;
	uint32_t base;
}__attribute__((__packed__)) IDT;

IDTDescr idt[256];
IDT idt_struct;
IDT* idtp;
IDT* phys_idtp;

#define UNUSED_TYPE_ATTR 0x00
#define INTERRUPT_GATE_TYPE_ATTR 0x8E
#define TRAP_GATE_TYPE_ATTR 0x8F

#define HIGH_OFFSET(x) ((x >> 16))
#define LOW_OFFSET(x) ((x & 0x0000FFFF)) 

#define SYSCALL_IRQ 0x80

void reset_idt();
void set_idt_entry(uint8_t number, void (*handler)(), uint16_t type);
void load_idt();
