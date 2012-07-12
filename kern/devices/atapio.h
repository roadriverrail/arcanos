#include <inc/types.h>

/**
Port Offset	 Function	 Description
0	 Data Port	 Read/Write PIO data bytes on this port.
1	 Features / Error Information	 Usually used for ATAPI devices.
2	 Sector Count	 Number of sectors to read/write (0 is a special value).
3	 Sector Number / LBAlo	 This is CHS / LBA28 / LBA48 specific.
4	 Cylinder Low / LBAmid	 Partial Disk Sector address.
5	 Cylinder High / LBAhi	 Partial Disk Sector address.
6	 Drive / Head Port	 Used to select a drive and/or head. May supports extra address/flag bits.
7	 Command port / Regular Status port	 Used to send commands or read the current status.
*/


#define ATA_BUS1_BASE 0x1F0
#define ATA_BUS2_BASE 0x170

uint16_t base_port;



#define ATA_DATA_PORT (base_port + 0)
#define ATA_FEATURES_ERR_INFO_PORT (base_port + 1)
#define ATA_SECTOR_COUNT_PORT (base_port + 2)
#define ATA_LBALOW_PORT (base_port + 3)
#define ATA_LBAMID_PORT (base_port + 4)
#define ATA_LBAHI_PORT (base_port + 5)
#define ATA_DRIVE_HEAD_PORT (base_port + 6)
#define ATA_COMMAND_STATUS_PORT (base_port + 7)
 
#define ATA_DEVCONTROL ((base_port == ATA_BUS1_BASE) ? 0x3F6 : 0x376)
#define ATA_ALT_STATUS_PORT ATA_DEVCONTROL

/*
 * Bit	 Abbreviation	 Function
0	 ERR	 Indicates an error occurred. Send a new command to clear it (or nuke it with a Software Reset).
3	 DRQ	 Set when the drive has PIO data to transfer, or is ready to accept PIO data.
4	 SRV	 Overlapped Mode Service Request.
5	 DF	 Drive Fault Error (does not set ERR).
6	 RDY	 Bit is clear when drive is spun down, or after an error. Set otherwise.
7	 BSY	 Indicates the drive is preparing to send/receive data (wait for it to clear). In case of 'hang' (it never clears), do a software reset.
*/

#define STATUS_ERR (1 << 0)
#define STATUS_DRQ (1 << 3)
#define STATUS_SRV (1 << 4)
#define STATUS_DF (1 << 5)
#define STATUS_RDY (1 << 6)
#define STATUS_BSY (1 << 7)

#define SELECT_MASTER_DRIVE 0xA0
#define SELECT_SLAVE_DRIVE 0xB0

#define RW_MASTER_DRIVE 0xE0
#define RW_SLAVE_DRIVE 0xF0

#define COMMAND_READ_SECTORS 0x20
#define COMMAND_IDENTIFY 0xEC