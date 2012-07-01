#ifndef ARCANOS_INC_MEMLAYOUT_H
#define ARCANOS_INC_MEMLAYOUT_H

#ifndef __ASSEMBLER__
#include <inc/types.h>
//#include <inc/queue.h>
#include <inc/mmu.h>
#endif /* not __ASSEMBLER__ */

/*
 * This file contains definitions for memory management in our OS,
 * which are relevant to both the kernel and user-mode software.
 */

// Global descriptor numbers
#define GD_KT     0x08     // kernel text
#define GD_KD     0x10     // kernel data
#define GD_UT     0x18     // user text
#define GD_UD     0x20     // user data
#define GD_TSS    0x28     // Task segment selector

// All physical memory mapped at this address
#define	KERNBASE	0xC0000000

#define INITIAL_PDE 0x09D000

// At IOPHYSMEM (640K) there is a 384K hole for I/O.  From the kernel,
// IOPHYSMEM can be addressed at KERNBASE + IOPHYSMEM.  The hole ends
// at physical address EXTPHYSMEM.
#define IOPHYSMEM	0x0A0000
#define EXTPHYSMEM	0x100000

#define KSTACKTOP	VPT
#define KSTKSIZE	(8*PGSIZE)   		// size of a kernel stack

#endif /* !ARCANOS_INC_MEMLAYOUT_H */
