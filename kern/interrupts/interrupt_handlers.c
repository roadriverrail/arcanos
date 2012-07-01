#include <inc/types.h>
#include <inc/x86.h>
#include <inc/pic.h>
#include <inc/stringformat.h>

char* exception_messages[32] = {
"Divide-by-zero Error",
"Debug",
"Non-maskable Interrupt",
"Breakpoint",
"Overflow",
"Bound Range Exceeded",
"Invalid Opcode",
"Device Not Available",
"Double Fault",
"Coprocessor Segment Overrun",
"Invalid TSS",
"Segment Not Present",
"Stack-Segment Fault",
"General Protection Fault",
"Page Fault",
"Reserved",
"x87 Floating-Point Exception",
"Alignment Check",
"Machine Check",
"SIMD Floating-Point Exception",
"Reserved",
"Reserved",
"Reserved",
"Reserved",
"Reserved",
"Reserved",
"Reserved",
"Reserved",
"Reserved",
"Reserved",
"Security Exception",
"Reserved"
};

/**
 * Exceptions to handle:
 * 
Divide-by-zero Error	 0	 Fault	 #DE	 No
Debug	 1	 Fault/Trap	 #DB	 No
Non-maskable Interrupt	 2	 Interrupt	 -	 No
Breakpoint	 3	 Trap	 #BP	 No
Overflow	 4	 Trap	 #OF	 No
Bound Range Exceeded	 5	 Fault	 #BR	 No
Invalid Opcode	 6	 Fault	 #UD	 No
Device Not Available	 7	 Fault	 #NM	 No
Double Fault	 8	 Abort	 #DF	 Yes
Coprocessor Segment Overrun	 9	 Fault	 -	 No
Invalid TSS	 10	 Fault	 #TS	 Yes
Segment Not Present	 11	 Fault	 #NP	 Yes
Stack-Segment Fault	 12	 Fault	 #SS	 Yes
General Protection Fault	 13	 Fault	 #GP	 Yes
Page Fault	 14	 Fault	 #PF	 Yes
Reserved	 15	 -	 -	 No
x87 Floating-Point Exception	 16	 Fault	 #MF	 No
Alignment Check	 17	 Fault	 #AC	 Yes
Machine Check	 18	 Abort	 #MC	 No
SIMD Floating-Point Exception	 19	 Fault	 #XM/#XF	 No
Reserved	 20-29	 -	 -	 No
Security Exception	 30	 -	 #SX	 No
Reserved	 31	 -	 -	 No
Triple Fault	 -	 -	 -	 No
*/

/* filename : interrupt_handlers.c */
void _interrupt_handler(uint8_t number)
{
	_kern_print("%s exception.  Halting.\n", exception_messages[number]);
	asm("hlt");
	while(1);
}

void _keyboard_interrupt()
{
    uint8_t new_scan_code=0;
	new_scan_code = inb(0x60);
 
    /* Do something with the scancode.
     * Remember you only get '''one''' byte of the scancode each time the ISR is invoked.
     * (Though most of the times the scancode is only one byte.) 
     */
	_kern_print("Keyboard scancode 0x%x\n", new_scan_code);
    /* Acknowledge the IRQ, pretty much tells the PIC that we can accept >=priority IRQs now. */
	PIC_sendEOI(1); //Keyboard is IRQ 1.
}
