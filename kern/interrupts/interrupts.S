/* filename : interrupts.S */

.globl   _load_idt
.extern  idtp
.extern _interrupt_handler

.align   4

//Divide-by-zero Error
.globl isr_0
isr_0:
    pushal
	push $0
    call    _interrupt_handler
    popal
    iret

//Debug
.globl isr_1
isr_1:
    pushal
	push $1
    call    _interrupt_handler
    popal
    iret

//Non-maskable Interrupt
.globl isr_2
isr_2:
    pushal
	push $2
    call    _interrupt_handler
    popal
    iret

//Breakpoint
.globl isr_3
isr_3:
    pushal
	push $3
    call    _interrupt_handler
    popal
    iret

//Overflow
.globl isr_4
isr_4:
    pushal
	push $4
    call    _interrupt_handler
    popal
    iret
	
//Bound Range Exceeded
.globl isr_5
isr_5:
    pushal
	push $5
    call    _interrupt_handler
    popal
    iret
	
//Invalid Opcode
.globl isr_6
isr_6:
    pushal
	push $6
    call    _interrupt_handler
    popal
    iret
	
//Device Not Available
.globl isr_7
isr_7:
    pushal
	push $7
    call    _interrupt_handler
    popal
    iret
	
//Double Fault
.globl isr_8
isr_8:
    pushal
	push $8
    call    _interrupt_handler
    popal
    iret
	
//Coprocessor Segment Overrun
.globl isr_9
isr_9:
    pushal
	push $9
    call    _interrupt_handler
    popal
    iret
	
//Invalid TSS
.globl isr_10
isr_10:
    pushal
	push $10
    call    _interrupt_handler
    popal
    iret

//Segment Not Present
.globl isr_11
isr_11:
    pushal
	push $11
    call    _interrupt_handler
    popal
    iret

//Stack-Segment Fault
.globl isr_12
isr_12:
    pushal
	push $12
    call    _interrupt_handler
    popal
    iret

//General Protection Fault
.globl isr_13
isr_13:
    pushal
	push $13
    call    _interrupt_handler
    popal
    iret

//Page Fault
.globl isr_14
isr_14:
    pushal
	push $14
    call    _interrupt_handler
    popal
    iret

//x87 Floating-Point Exception
.globl isr_16
isr_16:
    pushal
	push $16
    call    _interrupt_handler
    popal
    iret

//Alignment Check
.globl isr_17
isr_17:
    pushal
	push $17
    call    _interrupt_handler
    popal
    iret

//Machine Check
.globl isr_18
isr_18:
    pushal
	push $18
    call    _interrupt_handler
    popal
    iret

//SIMD Floating-Point Exception
.globl isr_19
isr_19:
    pushal
	push $19
    call    _interrupt_handler
    popal
    iret

//Security Exception
.globl isr_30
isr_30:
    pushal
	push $30
    call    _interrupt_handler
    popal
    iret

.globl keyboard_isr
keyboard_isr:
	pushal
	call _keyboard_interrupt
	popal
	iret
