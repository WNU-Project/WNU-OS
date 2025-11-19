#include "vga.h"
// CD-ROM Device Detection for WNU OS
// Detects ATAPI CD-ROM drives for ISO 9660 mounting

#include <stdint.h>

// Define NULL for freestanding environment
#ifndef NULL
#define NULL ((void*)0)
#endif

// VGA output functions
extern void vga_puts(const char* str);

// Define NULL for freestanding environment
#ifndef NULL
#define NULL ((void*)0)
#endif

// VGA output functions
extern void vga_puts(const char* str);

// ATA/ATAPI I/O ports
#define ATA_PRIMARY_DATA     0x1F0
#define ATA_PRIMARY_ERROR    0x1F1
#define ATA_PRIMARY_FEATURES 0x1F1
#define ATA_PRIMARY_SECTOR   0x1F2
#define ATA_PRIMARY_CYL_LOW  0x1F3
#define ATA_PRIMARY_CYL_HIGH 0x1F4
#define ATA_PRIMARY_DRIVE    0x1F5
#define ATA_PRIMARY_STATUS   0x1F7
#define ATA_PRIMARY_COMMAND  0x1F7

#define ATA_SECONDARY_DATA     0x170
#define ATA_SECONDARY_ERROR    0x171
#define ATA_SECONDARY_FEATURES 0x171
#define ATA_SECONDARY_SECTOR   0x172
#define ATA_SECONDARY_CYL_LOW  0x173
#define ATA_SECONDARY_CYL_HIGH 0x174
#define ATA_SECONDARY_DRIVE    0x175
#define ATA_SECONDARY_STATUS   0x177
#define ATA_SECONDARY_COMMAND  0x177

// ATA Commands
#define ATA_CMD_IDENTIFY_PACKET 0xA1
#define ATA_CMD_PACKET          0xA0

// Device types
#define DEVICE_TYPE_UNKNOWN 0
#define DEVICE_TYPE_ATA     1
#define DEVICE_TYPE_ATAPI   2

// CD-ROM device structure
typedef struct {
    uint16_t base_port;      // Base I/O port (0x1F0 or 0x170)
    uint8_t drive;           // Drive number (0 = master, 1 = slave)
    uint8_t device_type;     // Device type (ATA/ATAPI)
    char model[41];          // Device model string
    uint8_t is_cdrom;        // 1 if this is a CD-ROM device
} cdrom_device_t;

// Global CD-ROM device storage
static cdrom_device_t g_cdrom_devices[4];  // Max 4 devices (2 channels * 2 drives)
static int g_cdrom_count = 0;

// Helper functions
static uint8_t inb(uint16_t port);
static void outb(uint16_t port, uint8_t data);
static uint16_t inw(uint16_t port);
static void io_delay(void);

// Port I/O functions (inline assembly)
static uint8_t inb(uint16_t port) {
    uint8_t data;
    asm volatile("inb %1, %0" : "=a"(data) : "dN"(port));
    return data;
}

static void outb(uint16_t port, uint8_t data) {
    asm volatile("outb %0, %1" : : "a"(data), "dN"(port));
}

static uint16_t inw(uint16_t port) {
    uint16_t data;
    asm volatile("inw %1, %0" : "=a"(data) : "dN"(port));
    return data;
}

static void io_delay(void) {
    // Small delay for hardware
    for (int i = 0; i < 1000; i++) {
        asm volatile("nop");
    }
}

// Check if a drive exists on the given channel/drive
static int check_drive_exists(uint16_t base_port, uint8_t drive) {
    // Select drive
    outb(base_port + 6, 0xA0 | (drive << 4));
    io_delay();
    
    // Send IDENTIFY PACKET command
    outb(base_port + 7, ATA_CMD_IDENTIFY_PACKET);
    io_delay();
    
    // Check status
    uint8_t status = inb(base_port + 7);
    if (status == 0) {
        return 0; // No drive
    }
    
    // Wait for BSY to clear
    int timeout = 1000;
    while ((status & 0x80) && timeout--) {
        io_delay();
        status = inb(base_port + 7);
    }
    
    if (timeout <= 0) {
        return 0; // Timeout
    }
    
    // Check for ATAPI signature
    uint8_t cl = inb(base_port + 4);  // Cylinder low
    uint8_t ch = inb(base_port + 5);  // Cylinder high
    
    if (cl == 0x14 && ch == 0xEB) {
        return DEVICE_TYPE_ATAPI; // ATAPI device (potentially CD-ROM)
    }
    
    return DEVICE_TYPE_ATA; // ATA device (hard disk)
}

// Read device identification data
static int read_device_info(uint16_t base_port, uint8_t drive, cdrom_device_t* device) {
    device->base_port = base_port;
    device->drive = drive;
    device->device_type = check_drive_exists(base_port, drive);
    device->is_cdrom = 0;
    
    if (device->device_type != DEVICE_TYPE_ATAPI) {
        return 0; // Not an ATAPI device
    }
    
    // Select drive
    outb(base_port + 6, 0xA0 | (drive << 4));
    io_delay();
    
    // Send IDENTIFY PACKET command
    outb(base_port + 7, ATA_CMD_IDENTIFY_PACKET);
    io_delay();
    
    // Wait for data ready
    uint8_t status;
    int timeout = 1000;
    do {
        status = inb(base_port + 7);
        io_delay();
    } while ((status & 0x88) != 0x08 && timeout--);
    
    if (timeout <= 0) {
        return 0; // Timeout
    }
    
    // Read 256 words of identification data
    uint16_t id_data[256];
    for (int i = 0; i < 256; i++) {
        id_data[i] = inw(base_port);
    }
    
    // Extract model name (words 27-46, byte-swapped)
    for (int i = 0; i < 20; i++) {
        uint16_t word = id_data[27 + i];
        device->model[i * 2] = (word >> 8) & 0xFF;
        device->model[i * 2 + 1] = word & 0xFF;
    }
    device->model[40] = '\0';
    
    // Trim trailing spaces
    for (int i = 39; i >= 0 && device->model[i] == ' '; i--) {
        device->model[i] = '\0';
    }
    
    // Check if it's a CD-ROM by looking at device type in word 0
    uint16_t device_config = id_data[0];
    if ((device_config & 0x1F00) == 0x0500) { // Device type 5 = CD-ROM
        device->is_cdrom = 1;
        return 1; // Found CD-ROM
    }
    
    return 0; // ATAPI device but not CD-ROM
}

// Detect all CD-ROM devices in the system
void* detect_cdrom_device(void) {
    vga_puts("Detecting CD-ROM drives...\n");
    
    g_cdrom_count = 0;
    
    // Check primary channel (0x1F0)
    if (read_device_info(ATA_PRIMARY_DATA, 0, &g_cdrom_devices[g_cdrom_count])) {
        vga_puts("Found CD-ROM on primary master: ");
        vga_puts(g_cdrom_devices[g_cdrom_count].model);
        vga_puts("\n");
        g_cdrom_count++;
    }
    
    if (read_device_info(ATA_PRIMARY_DATA, 1, &g_cdrom_devices[g_cdrom_count])) {
        vga_puts("Found CD-ROM on primary slave: ");
        vga_puts(g_cdrom_devices[g_cdrom_count].model);
        vga_puts("\n");
        g_cdrom_count++;
    }
    
    // Check secondary channel (0x170)
    if (read_device_info(ATA_SECONDARY_DATA, 0, &g_cdrom_devices[g_cdrom_count])) {
        vga_puts("Found CD-ROM on secondary master: ");
        vga_puts(g_cdrom_devices[g_cdrom_count].model);
        vga_puts("\n");
        g_cdrom_count++;
    }
    
    if (read_device_info(ATA_SECONDARY_DATA, 1, &g_cdrom_devices[g_cdrom_count])) {
        vga_puts("Found CD-ROM on secondary slave: ");
        vga_puts(g_cdrom_devices[g_cdrom_count].model);
        vga_puts("\n");
        g_cdrom_count++;
    }
    
    if (g_cdrom_count == 0) {
        vga_puts("No CD-ROM drives detected\n");
        return NULL;
    }
    
    vga_puts("CD-ROM detection complete. Found ");
    // Simple number to string conversion for device count
    char count_str[2];
    count_str[0] = '0' + g_cdrom_count;
    count_str[1] = '\0';
    vga_puts(count_str);
    vga_puts(" device(s)\n");
    
    // Return pointer to first CD-ROM device
    return &g_cdrom_devices[0];
}

// Get CD-ROM device by index
cdrom_device_t* get_cdrom_device(int index) {
    if (index < 0 || index >= g_cdrom_count) {
        return NULL;
    }
    return &g_cdrom_devices[index];
}

// Get number of detected CD-ROM devices
int get_cdrom_count(void) {
    return g_cdrom_count;
}

// Read a sector from CD-ROM (implementation for iso9660_read_sector)
int cdrom_read_sector(void* device, uint32_t lba, void* buffer) {
    cdrom_device_t* cdrom = (cdrom_device_t*)device;
    
    if (!cdrom || !cdrom->is_cdrom) {
        return -1;
    }
    
    // TODO: Implement actual ATAPI packet command to read sector
    // This would involve:
    // 1. Send PACKET command
    // 2. Send READ(10) SCSI command packet
    // 3. Read data from device
    
    vga_puts("Reading sector ");
    // Simple LBA to string (for debugging)
    vga_puts(" from CD-ROM\n");
    
    // For now, return success but don't actually read
    // In a complete implementation, this would send ATAPI commands
    return 0;
}