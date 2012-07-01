#include <inc/memlayout.h>
#include <inc/memmgr.h>
#include <inc/stringformat.h>
#include <inc/console.h>

#define CELL_SIZE (PAGE_SIZE*sizeof(uint8_t)*8)
#define FRAME_STATUS_FREE 1
#define FRAME_STATUS_USED 0
#define CELL_STATUS_USED 0
#define PT_ENTRIES 1024
#define HIGHER_HALF_BASE 768
#define INVALID_FRAME ((void*)0xFFFFFFFF)

static kmemdesc* free_list_head;
static kmemdesc* alloc_list_head;
static kmemdesc* free_list_tail;
static kmemdesc* alloc_list_tail;

static uint32_t* pde = (uint32_t*)INITIAL_PDE;

int mem_available;

void flush_tlb();

/* 
 * I can't honestly say if this is the dumbest thing I've done or one
 * of my more clever tricks.  This function is written to sling together
 * some page tables for the kernel.  The kernel is higher-half (actually
 * it's linked at 0xc0100000) but I just didn't want to write this in
 * assembly.  Well...nothing in here involves a long jump or a far call
 * or refers to global variables ("end" is linker-provided), so the
 * code should be location-independent!  So, we call this from within
 * entry.S when we have nothing set up, and it still works!
 * 
 * Probably shouldn't call this from within the kernel proper, though.
 */

void init_paging() {
	extern char end[];
	
	int i;
	uint32_t* initial_pde = (uint32_t*)INITIAL_PDE;
	uint32_t* firstmeg_table = (uint32_t*)(INITIAL_PDE-PAGE_SIZE);
	uint32_t* kernel_table = (uint32_t*)(INITIAL_PDE-(PAGE_SIZE*2));
	
	//First, clear the tables
	for (i=0; i<1024; ++i) {
		*initial_pde=0;
		*firstmeg_table = 0;
		*kernel_table = 0;
	}
	
	//Identity-page the first 4MB
	for (i=0; i<1024; i++) {
		uint32_t page_base = i*PAGE_SIZE;
		page_base = page_base | PAGE_RW | PAGE_PRESENT;
		firstmeg_table[i] = page_base;
	}
	
	initial_pde[0] = ((uint32_t)firstmeg_table) | PAGE_RW | PAGE_PRESENT;
	
	//Now page in the kernel's space.
	uint32_t kernel_page_count = (((uint32_t)end) - KERNBASE) / PAGE_SIZE;
	if ((((uint32_t)end) - KERNBASE) % PAGE_SIZE > 0) ++kernel_page_count;

    /* NOTE: this should really map in only enough pages to fit the kernel
     * but for now, this is mapping the entire lower 4MB to 0xc0000000
     * in addition to the identity paging above.  Why?  Because GRUB
     * is putting the multiboot header struct in low memory but passing
     * it a value that appears to be link-address relative (i.e. it's at
     * 0xc00300000 or so) and so the area below the kernel must be mapped
     * in, too.  We're going to change link addresses before MAGUS is
     * over, anyway, so for now we're mapping the same 4MB as both identity
     * and into the link-address space of the kernel. */
    
	for (i=0; i<1024/*kernel_page_count*/; ++i) {
		//Load base is 1MB
		uint32_t page_base = (i*PAGE_SIZE)/*+(1024*1024)*/;
		page_base = page_base | PAGE_RW | PAGE_PRESENT;
		//the link address base is not flat higher half but higher half
		//plus 1MB
		kernel_table[i] = page_base;
	}
	
	initial_pde[768] = ((uint32_t)kernel_table) | PAGE_RW | PAGE_PRESENT;
	
	return;

}

void frame_allocator_init(multiboot_info_t* mbi, uint32_t kernel_base, uint32_t kernel_end) {
	
	int i;
	int frames_allocated = 0;
	for(i=0; i<(PHYS_MEM_MAP_SIZE); i++) {
		phys_mem_map[i] = 0;
	}

   //_kern_print("mmap_addr = 0x%x, mmap_length = 0x%x\n",
	//	   (unsigned) (mbi->mmap_addr), (unsigned) mbi->mmap_length);

   multiboot_memory_map_t* mmap;
   for (mmap = (multiboot_memory_map_t *) (mbi->mmap_addr);
		(unsigned long) mmap < (mbi->mmap_addr) + mbi->mmap_length;
		mmap = (multiboot_memory_map_t *) ((unsigned long) mmap
								 + mmap->size + sizeof (mmap->size)))
	{
	 //_kern_print(" size = 0x%x, base_addr = 0x%x%x,"
		//	 " length = 0x%x%x, type = 0x%x\n",
			// (unsigned) mmap->size,
		//	 mmap->addr_high,
	//		 mmap->addr_low,
		//	 mmap->len_high,
	//		 mmap->len_low,
		//	 (unsigned) mmap->type);
			 
	if (mmap->type == MULTIBOOT_MEMORY_AVAILABLE) {
		 //Kindly note here that we will *NOT* be making use of the "high"
		 //fields here.  The memory manager is current capped at 4GB, so
		 //we will deal only with the lower 32-bits of addresses.
		 
		 //The goal here is to flip on the bits for available RAM.  After
		 //that, we will flip OFF the bits for the areas where the kernel
		 //was loaded and where the initial page tables sit.
		 
		 //The frame allocator is very happy to fail to mark partial
		 //frames as "not available."  This is a good, safe starting point
		 //as it means that the only physical memory allocated out will
		 //"really be there."
		 
		 uint32_t base = mmap->addr_low;
		 uint32_t len = mmap->len_low;
	
	     //First, advance up to the next page frame boundary.
		 if ((base % PAGE_SIZE) != 0) {
			 base = base + (PAGE_SIZE - (base % PAGE_SIZE));
			 len = len - (PAGE_SIZE - (base % PAGE_SIZE));
		 }
	
	     //As long as there is another frame to allocate, allocate it.
		 while (!(len<PAGE_SIZE)) {
			 mark_frame(base, FRAME_STATUS_FREE);
			 ++frames_allocated;
			 base += PAGE_SIZE;
			 len -= PAGE_SIZE;
		 }
	}
   }
   _kern_print("frame allocator identified %d frames in available memory\n", frames_allocated);
   //Now, time to make sure the page directory and any existing page
   //tables are never again re-allocated.
   mark_frame(INITIAL_PDE, FRAME_STATUS_USED);
   
   uint32_t* page_entry = (uint32_t*)INITIAL_PDE;
   for (i=0; i<1024; i++) {
		uint32_t pde_entry = page_entry[i];
		pde_entry &= 0xFFFFF000; //Page table address is in the upper 20 bits
		if (pde_entry != 0) {
			_kern_print("Located existing page at 0x%x and will mark it as used\n", pde_entry);
			mark_frame(pde_entry, FRAME_STATUS_USED);
		}
   }
   
   //Finally, mark off the area where the kernel is loaded
   kernel_base &= 0xFFFFF000; //Find the nearest page (rounding down)
   while(kernel_base < kernel_end) {
	   mark_frame(kernel_base, FRAME_STATUS_USED);
	   kernel_base += PAGE_SIZE;
   }
   
}

void mark_frame(uint32_t base, uint8_t status) {

	uint8_t mask = 1 << ((base % CELL_SIZE)/PAGE_SIZE);
	
	if (status == FRAME_STATUS_FREE) {
		phys_mem_map[base/CELL_SIZE] |= mask;
	} else if (status == FRAME_STATUS_USED) {
		mask = ~mask;
		phys_mem_map[base/CELL_SIZE] &= mask;
	}
}

void* get_frame() {
	int i;
	for (i=0; i<PHYS_MEM_MAP_SIZE; i++) {
		if (phys_mem_map[i] != CELL_STATUS_USED) {
			//This cell has a free frame.  Find the lowest order bit
			//which is 0.
			int bit;
			for (bit=0; bit<(sizeof(uint8_t)*8); bit++) {
				//Rotate over to test another bit.  If it can &
				//against 0x0001 then the 1's bit is 1, so keep trying
				//else, the blank frame is at the i'th cell on bit
				if ((phys_mem_map[i] >> bit) & FRAME_STATUS_FREE) {
					break;
				} else {
					continue;
				}
			}
			
			//Once we get here, we know we must have an available frame
			//As the mem_map entry was not full and so an empty bit was
			//found somewhere.
			
			//Mark it as taken
			mark_frame(((i*CELL_SIZE)+(bit*PAGE_SIZE)), FRAME_STATUS_USED);
			
			//return the address
			uint32_t addr = (i*CELL_SIZE)+(bit*PAGE_SIZE);
			return (void*)addr;
		}
	}
	//Looks like everything was taken already
	_kern_print("ERROR: Could not locate a frame to allocate\n");
	return (void*)INVALID_FRAME;
}

void* get_page() {
	
	void* frame = get_frame();
	if (frame == INVALID_FRAME) { 
		_kern_print("Did not get a frame for the requested page!\n");
		return 0; 
	}
	
	//All page addresses returned from here should be higher-half
	int pd_entry;
	for (pd_entry = HIGHER_HALF_BASE; pd_entry < PT_ENTRIES; pd_entry++) {
		if (pde[pd_entry] == 0) {
			//Found a blank spot in the page directory, so we will
			//create a table and use it.
			uint32_t* new_table = (uint32_t*)get_frame();
			if (new_table == INVALID_FRAME) {
				_kern_print("Could not allocate a new page table.\n");
				//TODO: RETURN INITIAL FRAME TO POOL TO PREVENT A LEAK
				return 0;
			}
			//Blank the table
			int i;
			for (i=0; i<PT_ENTRIES; i++) { new_table[i] = 0; }
			new_table[0] = ((uint32_t)frame)| PAGE_RW | PAGE_PRESENT;
			pde[pd_entry] = ((uint32_t)new_table)|PAGE_RW|PAGE_PRESENT;
			flush_tlb();
			return (void*)(pd_entry*PAGE_SIZE*PT_ENTRIES);
		} else {
			//Scan this page table to see if it has any holes
			uint32_t* pt = ((uint32_t*)(pde[pd_entry] & 0xFFFFF000));
			int pt_entry;
			for (pt_entry=0; pt_entry<PT_ENTRIES; pt_entry++) {
				if (pt[pt_entry] == 0) {
					pt[pt_entry] = ((uint32_t)frame)| PAGE_RW | PAGE_PRESENT;
					flush_tlb();
					return(void*)((pd_entry*PAGE_SIZE*PT_ENTRIES)+(pt_entry*PAGE_SIZE));
				}
			}
			//Didn't find any holes in this table...
		}
	}
	
	//If we get here, it means that all the tables and the directory are full
	_kern_print("ERROR: Page directory is FULL?!\n");
	return 0;
}

void memmgr_init() {
	alloc_list_head = 0;
	alloc_list_tail = 0;
	free_list_head = (kmemdesc*)get_page();
	if (!free_list_head) {
		_kern_print("Could not init memmgr: no pages available!\n");
		return;
	}

	int mem_available = PAGE_SIZE;
	mem_available -= sizeof(kmemdesc);	

	free_list_head->next = 0;
	free_list_head->size = mem_available;
	free_list_head->data_ptr = ((char*)free_list_head)+sizeof(kmemdesc);
	free_list_tail = free_list_head;
	return;
}

void* memmgr_allocate(unsigned int size) {
	
	if (size > (PAGE_SIZE - sizeof(kmemdesc))) {
		_kern_print("Alloc of sizes greater than %d not supported!\n",
		               (PAGE_SIZE - sizeof(kmemdesc)));
		return 0;
	} 
	//Sweep the free list to find a fit.
	kmemdesc* sweep = 0;
	kmemdesc* prev = 0;
	
	if (free_list_head == 0) return 0;
	
	sweep = free_list_head;
	while (sweep != free_list_tail) {

		//_kern_print("Comparing size %d against %d\n",sweep->size, size);
		if (sweep->size == size) {
			//_kern_print("Comparing size matched object at %x\n",sweep->data_ptr);
			
			//Found it.  If it's the head, then the free list needs
			//a new head.  Otherwise, suture it out.
			//a head == tail case cannot occur inside this loop
			if (sweep == free_list_head) {
				if (sweep == free_list_tail) {
					free_list_head = 0;
					free_list_tail = 0;
				} else {
					free_list_head = sweep->next;
				}
			} else {
				prev->next = sweep->next;
			}
			
			if (alloc_list_tail == 0) {
				alloc_list_head = sweep;
				alloc_list_tail = sweep;
			} else {
				alloc_list_tail->next = sweep;
				alloc_list_tail = sweep;
			}
			return sweep->data_ptr;
		}
		
		prev = sweep;
		sweep = sweep->next;
	}
		
	//We are here under the following conditions:
	//head == tail == sweep , previous == 0
	//sweep == tail, previous != 0
	
	//Before proceeding, see if we need to allocate more memory on
	//the end.
	if (free_list_tail->size < (size+sizeof(kmemdesc))) {

		kmemdesc* next_page = (kmemdesc*)get_page();
		if (!next_page) {
			_kern_print("Could not satisfy: no pages available!\n");
			return 0;
		}

		int mem_available = PAGE_SIZE;
		mem_available -= sizeof(kmemdesc);	
		next_page->size = mem_available;
		next_page->data_ptr = ((char*)next_page)+sizeof(kmemdesc);

		free_list_tail->next = next_page;
		free_list_tail = next_page;
		free_list_tail = free_list_head;
	}

	sweep = (kmemdesc*)(((char*)free_list_tail)+sizeof(kmemdesc)+size);
	sweep->data_ptr = ((char*)sweep) + sizeof(kmemdesc);
	mem_available = mem_available - size - sizeof(kmemdesc);
	sweep->size = mem_available;
	sweep->next = 0;

	//free_list_tail is now a kmemdesc describing the allocated space
	free_list_tail->size = size;

	if (alloc_list_tail != 0) {
		alloc_list_tail->next = free_list_tail;
		alloc_list_tail = free_list_tail;
	} else {
		alloc_list_head = free_list_tail;
		alloc_list_tail = free_list_tail;
	}
	
	//sweep is now set to be the new tail
	if (free_list_head == free_list_tail) {
		free_list_head = sweep;
		free_list_tail = sweep;
	} else {
		prev->next = sweep;
		free_list_tail = sweep;
	}
	
	//having moved the former free list tail to the tail of the
	//alloc list, return the alloc list tail data pointer
	return alloc_list_tail->data_ptr;
}

void memmgr_free(void* ptr) {
	kmemdesc* sweep = 0;
	kmemdesc* prev = 0;
	

	if (alloc_list_head == 0) return;
	
	//First, locate the correct kmemdesc in the free list.  Keep
	//prev updated in case we need to suture the list.
	
	sweep = alloc_list_head;
	while (sweep != alloc_list_tail) {
		if (sweep->data_ptr == ptr) {
			//Found it.  Break the loop.
			break;
		}
		prev = sweep;
		sweep = sweep->next;
	}
	//Conditions that get us to this point:
	//-- head == tail == sweep, prev == NULL, sweep data_ptr not checked
	if (alloc_list_head == alloc_list_tail) {
		if (sweep->data_ptr == ptr) {
			alloc_list_head = 0;
			alloc_list_tail = 0;
		} else {
			//ERROR! The only entry on the free list didn't
			//have a data pointer that matched the given pointer
			//_kern_print("Couldn't match the one item in the free list\n");
			return;
		}
	}
	//-- head == sweep, prev == null, sweep data_ptr matched
	else if (sweep == alloc_list_head) {
		alloc_list_head = sweep->next;
	}
	//-- sweep == tail, prev != null, sweep data ptr not checked
	else if (sweep == alloc_list_tail) {
		if (sweep->data_ptr == ptr) {
			//Matched at the tail of the list
			alloc_list_tail = prev;
			alloc_list_tail->next = 0;
		} else {
			//ERROR!  Traversed the list without a match!
			//_kern_print("Traversed the free list in vain\n");
			return;
		}
	}
	//-- prev != null, sweep data_ptr matched
	else {
		sweep->next = prev->next;
	}
	
	//At this point, sweep points to a kmemdesc that has been removed
	//from the alloc list.  We will place it at the head of the free
	//list.
	if (free_list_head == 0) {
		free_list_head = sweep;
		free_list_tail = sweep;		
	} else {
		sweep->next = free_list_head;
		free_list_head = sweep;
	}
	//_kern_print("Object at address %x freed\n", sweep->data_ptr);
	//_kern_print("Free list head now size %d address %x\n", free_list_head->size, free_list_head->data_ptr);
}

void flush_tlb() {
	//Do the super-heavy full TLB flush.  We should use invlpg in the
	//future.
	asm("movl %cr3, %eax; movl %eax, %cr3;");
}
