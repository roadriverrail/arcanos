#ifndef __ARCANOS_KERN_MEMMGR_H__
#define __ARCANOS_KERN_MEMMGR_H__

#include <inc/multiboot.h>

#define PAGE_PRESENT 0x1
#define PAGE_RW 0x2
#define PAGE_USER 0x4

#define PAGE_SIZE 4096

//4GB / 4kb page size / 1 bit per page
#define PHYS_MEM_MAP_SIZE 131072

//Bitmap of memory.
uint8_t phys_mem_map [PHYS_MEM_MAP_SIZE];

typedef struct _kmemdesc {
	void* data_ptr;
	unsigned int size;
	struct _kmemdesc* next;
} kmemdesc;

void init_paging();

void frame_allocator_init(multiboot_info_t* mbi, uint32_t kernel_base, uint32_t kernel_end);
void mark_frame(uint32_t base, uint8_t status);
void* get_frame();
void* get_page();
void memmgr_init();
void* memmgr_allocate(unsigned int size);
void memmgr_free(void* ptr);

void map_page(void* physaddr, void* virtualaddr, unsigned int flags);
	
#endif
