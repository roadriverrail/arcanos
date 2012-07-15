#include <kern/devices/atapio.h>
#include <inc/x86.h>
#include <inc/stringformat.h>

#define RW_DRIVE (0xE0 | slave_bit << 4)

static uint8_t drive_attached = 0;
static uint8_t slave_bit = 0;

void atapio_init()
{
  base_port = ATA_BUS1_BASE;
  
  // Confirm if a disk drive even exists!
  uint8_t result = inb(ATA_COMMAND_STATUS_PORT);
  if (result == 0xFF) {
    drive_attached = 0;
    return;
  }
  
  // Reset the bus
  _kern_print("Resetting ATA bus\n");
  outb(ATA_DEVCONTROL, 0x04);
  // Stall for 2ms
  int i;
  for (i=0; i<20; i++)
  {
    inb(ATA_COMMAND_STATUS_PORT);
  }
  outb(ATA_COMMAND_STATUS_PORT, 0x00);
  
  // Really, we should issue an IDENTIFY command here
  // but we'll just presume for now.  Select the master
  // disc and let it go

  outb(ATA_DEVCONTROL, SELECT_MASTER_DRIVE);
  slave_bit = 0;
  // Drives may need up to 500ns for drive select to
  // complete, and an IO read is about 100ns, so burn
  // 400ns.
  
  inb(ATA_COMMAND_STATUS_PORT);
  inb(ATA_COMMAND_STATUS_PORT);
  inb(ATA_COMMAND_STATUS_PORT);
  inb(ATA_COMMAND_STATUS_PORT);

  uint8_t cl=inb(ATA_LBAMID_PORT);	/* get the "signature bytes" */
  uint8_t ch=inb(ATA_LBAHI_PORT);

  /* differentiate ATA, ATAPI, SATA and SATAPI */
  if (cl==0x14 && ch==0xEB) 
  {
    _kern_print("Detected PATAPI device on default bus and drive\n");
  }
  else if (cl==0x69 && ch==0x96)
  {
    _kern_print("Detected SATAPI device on default bus and drive\n");
  }
  else if (cl==0 && ch == 0)
  {
    _kern_print("Detected PATA device on default bus and drive\n");
  }
  else if (cl==0x3c && ch==0xc3)
  {
    _kern_print("Detected SATA device on default bus and drive\n");
  }
  else
  {
    _kern_print("UNKNOWN device on default bus and drive\n");
  }
  
}

void atapio_read(uint32_t sector, uint32_t sector_count, uint16_t *buffer)
{
/* Comment of how to read from http://wiki.osdev.org/ATA_PIO_Mode
An example of a 28 bit LBA PIO mode read on the Primary bus:
Send 0xE0 for the "master" or 0xF0 for the "slave", ORed with the highest 4 bits of the LBA to port 0x1F6: outb(0x1F6, 0xE0 | (slavebit << 4) | ((LBA >> 24) & 0x0F))
Send a NULL byte to port 0x1F1, if you like (it is ignored and wastes lots of CPU time): outb(0x1F1, 0x00)
Send the sectorcount to port 0x1F2: outb(0x1F2, (unsigned char) count)
Send the low 8 bits of the LBA to port 0x1F3: outb(0x1F3, (unsigned char) LBA))
Send the next 8 bits of the LBA to port 0x1F4: outb(0x1F4, (unsigned char)(LBA >> 8))
Send the next 8 bits of the LBA to port 0x1F5: outb(0x1F5, (unsigned char)(LBA >> 16))
Send the "READ SECTORS" command (0x20) to port 0x1F7: outb(0x1F7, 0x20)
Wait for an IRQ or poll.
Transfer 256 words, a word at a time, into your buffer from I/O port 0x1F0. (In assembler, REP INSW works well for this.)
Then loop back to waiting for the next IRQ (or poll again -- see next note) for each successive sector.
*/

  outb(ATA_DRIVE_HEAD_PORT, RW_DRIVE | ((sector >> 24) & 0x0F));
  outb(ATA_FEATURES_ERR_INFO_PORT, 0x00);
  outb(ATA_SECTOR_COUNT_PORT, sector_count);
  outb(ATA_LBALOW_PORT, (uint8_t) sector);
  outb(ATA_LBAMID_PORT, (uint8_t)(sector >> 8));
  outb(ATA_LBAHI_PORT, (uint8_t)(sector >> 16));
  outb(ATA_COMMAND_STATUS_PORT, COMMAND_READ_SECTORS);

  while (1)
  {
    if ((inb(ATA_COMMAND_STATUS_PORT) & STATUS_BSY) == 0)
    {
      break;
    }
  }
  
  if ((inb(ATA_COMMAND_STATUS_PORT) & STATUS_DRQ) != 0)
  {
    for (int i=0; i < 255; i++)
    {
      buffer[i] = inw(ATA_DATA_PORT);
    }
  } else if ((inb(ATA_COMMAND_STATUS_PORT) & STATUS_ERR) != 0)
  {
    _kern_print("Failed to read disk base port: %x slave %x sector %d", base_port, slave_bit, sector);
  }

  return;
}