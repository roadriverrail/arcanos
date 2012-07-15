#include <kern/devices/atapio.h>
#include <inc/stringformat.h>
#include <inc/memmgr.h>

void load_partitions()
{
  _kern_print("Loading partitions...\n");
  atapio_init();
  
  uint16_t *buffer = memmgr_allocate(512);
  
  atapio_read(0, 1, buffer);
  
  uint8_t *partition_1 = ((uint8_t *)buffer) + 0x1BE;
  uint8_t *partition_2 = ((uint8_t *)buffer) + 0x1CE;
  uint8_t *partition_3 = ((uint8_t *)buffer) + 0x1DE;
  uint8_t *partition_4 = ((uint8_t *)buffer) + 0x1EE;
  
  uint8_t fstype_1 = *(partition_1 + 4);
  uint8_t fstype_2 = *(partition_2 + 4);
  uint8_t fstype_3 = *(partition_3 + 4);
  uint8_t fstype_4 = *(partition_4 + 4);
  
  _kern_print("Partition 1 type: %x\n", fstype_1);
  _kern_print("Partition 2 type: %x\n", fstype_2);
  _kern_print("Partition 3 type: %x\n", fstype_3);
  _kern_print("Partition 4 type: %x\n", fstype_4);
}