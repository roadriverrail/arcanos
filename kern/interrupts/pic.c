#include <inc/types.h>
#include <inc/x86.h>
#include <inc/pic.h>
#include <inc/stringformat.h>

/**
 * Again, a debt of gratitude to osdev.org for this code.
 */

void PIC_sendEOI(unsigned char irq)
{
	if(irq >= 8)
		outb(PIC2_COMMAND,PIC_EOI);
	
	outb(PIC1_COMMAND,PIC_EOI);
}

/*
arguments:
	offset1 - vector offset for master PIC
		vectors on the master become offset1..offset1+7
	offset2 - same for slave PIC: offset2..offset2+7
*/
void PIC_remap(int offset1, int offset2)
{
	 uint8_t a1, a2;
	
	a1 = inb(PIC1_DATA);                        // save masks
	a2 = inb(PIC2_DATA);
	
	outb(PIC1_COMMAND, ICW1_INIT+ICW1_ICW4);  // starts the initialization sequence
	io_wait();
	outb(PIC2_COMMAND, ICW1_INIT+ICW1_ICW4);
	io_wait();
	outb(PIC1_DATA, offset1);                 // define the PIC vectors
	io_wait();
	outb(PIC2_DATA, offset2);
	io_wait();
	outb(PIC1_DATA, 4); //ICW3: Master takes slave on IRQ2 (4 = 100b -- bitmask for IRQ2)
	io_wait();
	outb(PIC2_DATA, 2); //ICW3: Slave sends to master on IRQ2 (2 expressed in binary here)
	io_wait();
	
	outb(PIC1_DATA, ICW4_8086);
	io_wait();
	outb(PIC2_DATA, ICW4_8086);
	io_wait();
	
	outb(PIC1_DATA, a1);   // restore saved masks.
	outb(PIC2_DATA, a2);
}

void PIC_clear_masks() {
	outb(PIC1_DATA, 0xFF);
	outb(PIC2_DATA, 0xFF);
}

/*
 * Set/clear interrupt mask bits.  Must specify by IRQ and value is
 * 0 to clear and all else to set.
 */
void PIC_set_mask(uint8_t irq, uint8_t value) {
	uint8_t mask_bit = 1;

	uint8_t pic_port;
		
	if(irq >= 8) {
		irq = irq-8;
		pic_port = PIC2_DATA;
	} else {
		pic_port = PIC1_DATA;
	}

		uint8_t mask;
		mask = inb(pic_port); //get current mask
		io_wait();
		mask_bit = mask_bit << irq;
		if (value) {
			mask = mask | mask_bit; //If value is 1, then set the mask bit
		} else {
			mask = mask & (~mask_bit); //If value is 0, then clear the mask bit
		}
		outb(pic_port, mask); //set the new mask
		io_wait();
}
