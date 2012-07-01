#include <inc/idt.h>
#include <inc/string.h>
#include <inc/stringformat.h>

extern void isr_0();
extern void isr_1();
extern void isr_2();
extern void isr_3();
extern void isr_4();
extern void isr_5();
extern void isr_6();
extern void isr_7();
extern void isr_8();
extern void isr_9();
extern void isr_10();
extern void isr_11();
extern void isr_12();
extern void isr_13();
extern void isr_14();
extern void isr_16();
extern void isr_17();
extern void isr_18();
extern void isr_19();
extern void isr_30();

void reset_idt() {
		memset(idt, 0, sizeof(IDTDescr)*256);
		//The base of the table must be a linear address (which is 
		//currently the same as the physical address)
		idt_struct.base = (uint32_t)idt-KERNBASE;
		idt_struct.limit = 256*(sizeof(IDTDescr)-1);
		idtp = &idt_struct;		
		phys_idtp = (IDT*)(((uint32_t)idtp) - KERNBASE);
		_kern_print("IDTP: %x PHYS_IDTP %x\n", ((uint32_t)idtp), ((uint32_t)phys_idtp));
		//Set default exception handlers.  There's no compelling reason
		//to change them, but kernel code can by making its own
		//set_idt_entry calls.
		set_idt_entry(0, isr_0, TRAP_GATE_TYPE_ATTR);
		set_idt_entry(1,isr_1, TRAP_GATE_TYPE_ATTR);
		set_idt_entry(2,isr_2, INTERRUPT_GATE_TYPE_ATTR);
		set_idt_entry(3,isr_3, TRAP_GATE_TYPE_ATTR);
		set_idt_entry(4,isr_4, TRAP_GATE_TYPE_ATTR);
		set_idt_entry(5,isr_5, TRAP_GATE_TYPE_ATTR);
		set_idt_entry(6,isr_6, TRAP_GATE_TYPE_ATTR);
		set_idt_entry(7,isr_7, TRAP_GATE_TYPE_ATTR);
		set_idt_entry(8,isr_8, TRAP_GATE_TYPE_ATTR);
		set_idt_entry(9,isr_9, TRAP_GATE_TYPE_ATTR);
		set_idt_entry(10,isr_10, TRAP_GATE_TYPE_ATTR);
		set_idt_entry(11,isr_11, TRAP_GATE_TYPE_ATTR);
		set_idt_entry(12,isr_12, TRAP_GATE_TYPE_ATTR);
		set_idt_entry(13,isr_13, TRAP_GATE_TYPE_ATTR);
		set_idt_entry(14,isr_14, TRAP_GATE_TYPE_ATTR);
		set_idt_entry(16,isr_16, TRAP_GATE_TYPE_ATTR);
		set_idt_entry(17,isr_17, TRAP_GATE_TYPE_ATTR);
		set_idt_entry(18,isr_18, TRAP_GATE_TYPE_ATTR);
		set_idt_entry(19,isr_19, TRAP_GATE_TYPE_ATTR);
		set_idt_entry(30,isr_30, TRAP_GATE_TYPE_ATTR);

}

void set_idt_entry(uint8_t number, void (*handler)(), uint16_t type) {
	uint32_t handler_addr = (uint32_t)handler - KERNBASE;
	handler_addr = handler_addr;
	idt[number].offset_1 = LOW_OFFSET(handler_addr);
	idt[number].offset_2 = HIGH_OFFSET(handler_addr);
	idt[number].selector = GD_KT;
	idt[number].type_attr = type;
	idt[number].zero = 0;
}

void load_idt() {
    asm volatile("LIDT (%0) ": :"p" (phys_idtp));
}
