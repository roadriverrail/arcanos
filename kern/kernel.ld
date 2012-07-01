/* Simple linker script for the Arcanos kernel.
   See the GNU ld 'info' manual ("info ld") to learn the syntax. */

OUTPUT_FORMAT("elf32-i386", "elf32-i386", "elf32-i386")
OUTPUT_ARCH(i386)
ENTRY(_start)

virt = 0xC0100000;
phys = 0x00100000;
offset = 0xC0000000;

SECTIONS
{
	.text virt : AT(phys) {
	    PROVIDE(load_start = phys);
		PROVIDE(kern_textbase = .);
		*(.text .stub .text.* .gnu.linkonce.t.*)
		PROVIDE(kern_textend = .);
	}

	PROVIDE(etext = .);
	phys = ALIGN(4) - offset;

	.rodata : AT(phys) {
		robase = .;
		PROVIDE(kern_robase = .);
		*(.rodata .rodata.* .gnu.linkonce.r.*)
		PROVIDE(kern_roend = .);

	}

	phys = ALIGN(4) - offset;

	/* Include debugging information in kernel memory */
	.stab : AT(phys) {
		stabbase = .;
		PROVIDE(kern_stabbase = .);
		*(.stab);
		PROVIDE(kern_stabend = .);
		BYTE(0)		/* Force the linker to allocate space
				   for this section */
	}

	phys = ALIGN(1) - offset;

	.stabstr : AT(phys) {
		stabstrbase = .;
		PROVIDE(kern_stabstrbase = .);
		*(.stabstr);
		PROVIDE(kern_stabstrend = .);
		BYTE(0)		/* Force the linker to allocate space
				   for this section */
	}

	phys = . - offset;
	phys = ALIGN(0x1000) - offset;

	/* The data segment */
	.data ALIGN(0x1000) : AT (phys) {
		database = .;
		PROVIDE(kern_database = .);
		*(.data)
		PROVIDE(kern_dataend = .);
	}

	phys = ALIGN(32) - offset;
	
	PROVIDE(edata = .);

	.bss : AT (phys) {
		bssbase = .;
		PROVIDE(kern_bssbase = .);
		*(.bss)
		PROVIDE(kern_bssend = .);
	}

	PROVIDE(end = .);
	phys = . - offset;
	PROVIDE(load_end = phys);

	/DISCARD/ : {
		*(.eh_frame .note.GNU-stack)
	}
}
