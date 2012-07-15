/* See COPYRIGHT for copyright information. */

#include <inc/string.h>
#include <inc/info.h>
#include <inc/stringformat.h>
#include <inc/memmgr.h>
#include <inc/idt.h>
#include <inc/pic.h>
#include <inc/console.h>
#include <inc/multiboot.h>

/* Check if the bit BIT in FLAGS is set. */
#define CHECK_FLAG(flags,bit)   ((flags) & (1 << (bit)))

extern void keyboard_isr();
extern void load_partitions();
void i386_init(multiboot_info_t* mbi);

void kernel_main( void* mbd, unsigned int magic )
{
   if ( magic != 0x2BADB002 )
   {
      /* Something went not according to specs. Print an error */
      /* message and halt, but do *not* rely on the multiboot */
      /* data structure. */
      _kern_print("BROKEN MULTIBOOT HEADER!  Bailing out!");
      while(1);
   }
	mbd = KERNBASE + mbd;
	i386_init((multiboot_info_t*)mbd);
}

void
i386_init(multiboot_info_t* mbi)
{
	extern char etext[], edata[], end[], kern_textbase[], kern_textend[];
	extern char kern_robase[], kern_roend[];
	extern char kern_stabbase[], kern_stabend[];
	extern char kern_stabstrbase[], kern_stabstrend[];
	extern char kern_database[], kern_dataend[];
	extern char kern_bssbase[], kern_bssend[];
	extern char end[];
	extern char load_start[], load_end[];
	
	int i;
	int j;

	console_init();
	_kern_print("Arcanos version %s\n", ARCANOS_VERSION);
	_kern_print("Kernel mapped to address 0x%x\n", KERNBASE);

	/* Print out the flags. */
	_kern_print("multiboot info address = 0x%x\n", mbi);
	_kern_print("flags = 0x%x\n", (unsigned) mbi->flags);
	
	/* Are mem_* valid? */
	if (CHECK_FLAG (mbi->flags, 0))
	 _kern_print("mem_lower = %uKB, mem_upper = %uKB\n",
			 (unsigned) mbi->mem_lower, (unsigned) mbi->mem_upper);
	
	/* Is boot_device valid? */
	if (CHECK_FLAG (mbi->flags, 1))
	 _kern_print("boot_device = 0x%x\n", (unsigned) mbi->boot_device);
	
	/* Is the command line passed? */
	if (CHECK_FLAG (mbi->flags, 2)) {
	 _kern_print("cmdline = %s\n", (char *) (mbi->cmdline));
	}


	/* Are mods_* valid? */
	if (CHECK_FLAG (mbi->flags, 3))
	 {
	   multiboot_module_t *mod;
	   int i;
	
	   _kern_print("mods_count = %d, mods_addr = 0x%x\n",
			   (int) mbi->mods_count, (int) (mbi->mods_addr));
	   for (i = 0, mod = (multiboot_module_t *) (mbi->mods_addr);
			i < mbi->mods_count;
			i++, mod++)
		 _kern_print(" mod_start = 0x%x, mod_end = 0x%x, cmdline = %s\n",
				 (unsigned) (mod->mod_start),
				 (unsigned) (mod->mod_end),
				 (char *) (mod->cmdline));
	 }

	
	/* Bits 4 and 5 are mutually exclusive! */
	if (CHECK_FLAG (mbi->flags, 4) && CHECK_FLAG (mbi->flags, 5))
	 {
	   _kern_print("Both bits 4 and 5 are set.\n");
	   return;
	 }
	
	/* Is the symbol table of a.out valid? */
	if (CHECK_FLAG (mbi->flags, 4))
	 {
	   multiboot_aout_symbol_table_t *multiboot_aout_sym = &(mbi->u.aout_sym);
	
	   _kern_print("multiboot_aout_symbol_table: tabsize = 0x%0x, "
			   "strsize = 0x%x, addr = 0x%x\n",
			   (unsigned) multiboot_aout_sym->tabsize,
			   (unsigned) multiboot_aout_sym->strsize,
			   (unsigned) multiboot_aout_sym->addr);
	 }
	
	/* Is the section header table of ELF valid? */
	if (CHECK_FLAG (mbi->flags, 5))
	 {
	   multiboot_elf_section_header_table_t *multiboot_elf_sec = &(mbi->u.elf_sec);
	
	   _kern_print("multiboot_elf_sec: num = %u, size = 0x%x,"
			   " addr = 0x%x, shndx = 0x%x\n",
			   (unsigned) multiboot_elf_sec->num, (unsigned) multiboot_elf_sec->size,
			   (unsigned) (multiboot_elf_sec->addr), (unsigned) multiboot_elf_sec->shndx);
	 }
	
	/* Are mmap_* valid? */
	if (CHECK_FLAG (mbi->flags, 6))
	 {
	   multiboot_memory_map_t *mmap;
	
	   frame_allocator_init(mbi, (uint32_t)load_start, (uint32_t)load_end);
	 }

	//void* init_addr = memmgr_init(end);
	//_kern_print("Mem mgr initialized and using address %x\n", init_addr);
	asm("cli");
	reset_idt();
	load_idt();
	PIC_clear_masks();
	PIC_remap(32, 40); //PIC1 - 32-39, PIC2 - 40-47;
	set_idt_entry(33, keyboard_isr, INTERRUPT_GATE_TYPE_ATTR);
	PIC_set_mask(1, 0);
	load_idt();
	asm("sti"); //Interrupts are on now, bitches.
	_kern_print("Interrupts enabled.\n");
	//I think that it might actually be necessary to forcibly run the
	//keyboard ISR to perhaps...process any lingering key presses?
	asm("int $33");
	memmgr_init();

	_kern_print("Initializing ATAPIO driver\n");
	load_partitions();
//stop here for now
	_kern_print("Entering kernel idle loop.\n");
	asm("hlt");
	while (1);
}
