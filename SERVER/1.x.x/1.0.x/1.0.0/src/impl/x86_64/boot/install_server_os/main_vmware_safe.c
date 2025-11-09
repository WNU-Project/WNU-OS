/* WNU OS SERVER 1.0.0 Interactive Installer - VMware Safe Version
 * Based on Arch Linux manual installation style
 * No direct hardware I/O - compatible with all virtualization platforms
 */

typedef unsigned long size_t;
typedef unsigned long uintptr_t;

/* VGA Text Mode Functions - Safe for VMware */
static void vga_putc_at(unsigned int idx, char c, unsigned char attr) {
    volatile unsigned short *vga = (volatile unsigned short *)0xB8000;
    if (idx < 80 * 25) { // Bounds check
        vga[idx] = (unsigned short)((attr << 8) | (unsigned char)c);
    }
}

static void vga_clear_screen(void) {
    volatile unsigned short *vga = (volatile unsigned short *)0xB8000;
    for (int i = 0; i < 80 * 25; i++) {
        vga[i] = 0x0720; // Space with white on black
    }
}

static void vga_print_at(unsigned int *idxp, const char *s, unsigned char attr) {
    unsigned int idx = *idxp;
    while (*s) {
        if (*s == '\n') {
            // Move to next line
            idx = ((idx / 80) + 1) * 80;
        } else {
            if (idx < 80 * 25) { // Bounds check
                vga_putc_at(idx, *s, attr);
                idx++;
            }
        }
        s++;
    }
    *idxp = idx;
}

/* String functions */
static int strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(unsigned char *)s1 - *(unsigned char *)s2;
}

static void strcpy(char *dest, const char *src) {
    while ((*dest++ = *src++));
}

static int strlen(const char *s) {
    int len = 0;
    while (s[len]) len++;
    return len;
}

/* VMware-Safe Input Simulation
 * Uses pre-programmed commands to demonstrate full installation
 * This avoids VMware CPU faults from keyboard controller access
 */
static char vmware_safe_input(void) {
    static const char *demo_script[] = {
        "help",
        "fdisk -l", 
        "fdisk /dev/main",
        "mkfs.unfs /dev/main1",
        "mount /dev/main1 /mnt",
        "cp -r ~/filesystem/* /mnt",
        "cp ~/pacman/* /mnt/usr/bin", 
        "grub-install --root-directory=/mnt /dev/main",
        "reboot",
        NULL
    };
    
    static int command_index = 0;
    static int char_index = 0;
    static int waiting_for_enter = 0;
    
    /* Simulate typing delay */
    for (volatile int i = 0; i < 50000; i++);
    
    if (demo_script[command_index] == NULL) {
        /* All demo commands done, wait */
        return 0;
    }
    
    if (waiting_for_enter) {
        waiting_for_enter = 0;
        char_index = 0;
        command_index++;
        
        /* Pause between commands */
        for (volatile int i = 0; i < 200000; i++);
        
        return '\r';
    }
    
    const char *current_cmd = demo_script[command_index];
    if (char_index < strlen(current_cmd)) {
        return current_cmd[char_index++];
    } else {
        waiting_for_enter = 1;
        return '\r';
    }
}

/* Command processing functions */
static void cmd_fdisk_l(unsigned int *cursor) {
    vga_print_at(cursor, "Disk /dev/main: 10 GiB, 10737418240 bytes, 20971520 sectors\n", 0x07);
    vga_print_at(cursor, "Units: sectors of 1 * 512 = 512 bytes\n", 0x07);
    vga_print_at(cursor, "Sector size (logical/physical): 512 bytes / 512 bytes\n", 0x07);
    vga_print_at(cursor, "I/O size (minimum/optimal): 512 bytes / 512 bytes\n", 0x07);
    vga_print_at(cursor, "Disklabel type: dos\n", 0x07);
    vga_print_at(cursor, "Disk identifier: 0x12345678\n\n", 0x07);
    
    vga_print_at(cursor, "Device      Boot Start      End  Sectors Size Type\n", 0x0F);
    vga_print_at(cursor, "/dev/main1       2048 20971519 20969472  10G Linux\n", 0x07);
}

static void cmd_fdisk_main(unsigned int *cursor) {
    vga_print_at(cursor, "Welcome to fdisk (util-linux 2.37.2).\n", 0x07);
    vga_print_at(cursor, "Changes will remain in memory only, until you decide to write them.\n", 0x0E);
    vga_print_at(cursor, "Be careful before using the write command.\n\n", 0x0C);
    
    vga_print_at(cursor, "Command (m for help): d\n", 0x0D);
    vga_print_at(cursor, "Selected partition 1\n", 0x07);
    vga_print_at(cursor, "Partition 1 has been deleted.\n\n", 0x0A);
    
    vga_print_at(cursor, "Command (m for help): n\n", 0x0D);
    vga_print_at(cursor, "Partition type\n", 0x07);
    vga_print_at(cursor, "   p   primary (0 primary, 0 extended, 4 free)\n", 0x07);
    vga_print_at(cursor, "   e   extended (container for logical partitions)\n", 0x07);
    vga_print_at(cursor, "Select (default p): p\n", 0x0A);
    vga_print_at(cursor, "Partition number (1-4, default 1): 1\n", 0x0A);
    vga_print_at(cursor, "First sector (2048-20971519, default 2048): 2048\n", 0x0A);
    vga_print_at(cursor, "Last sector (+/-sectors/+size{K,M,G,T,P}): +10G\n", 0x0A);
    vga_print_at(cursor, "Created a new partition 1 of type 'Linux' and of size 10 GiB.\n\n", 0x0A);
    
    vga_print_at(cursor, "Command (m for help): a\n", 0x0D);
    vga_print_at(cursor, "Selected partition 1\n", 0x07);
    vga_print_at(cursor, "The bootable flag on partition 1 is enabled now.\n\n", 0x0A);
    
    vga_print_at(cursor, "Command (m for help): w\n", 0x0D);
    vga_print_at(cursor, "The partition table has been altered.\n", 0x0A);
    vga_print_at(cursor, "Syncing disks.\n", 0x0A);
}

static void cmd_mkfs_unfs(unsigned int *cursor) {
    vga_print_at(cursor, "mkfs.unfs 1.0.0 (WNU OS Server filesystem)\n", 0x0F);
    vga_print_at(cursor, "Creating filesystem on /dev/main1...\n", 0x07);
    vga_print_at(cursor, "Superblock backups stored on blocks: 8193, 24577, 40961\n", 0x07);
    vga_print_at(cursor, "Allocating group tables... done\n", 0x0A);
    vga_print_at(cursor, "Writing inode tables... done\n", 0x0A);
    vga_print_at(cursor, "Creating journal (4096 blocks)... done\n", 0x0A);
    vga_print_at(cursor, "Writing superblocks and filesystem accounting information... done\n", 0x0A);
}

static void cmd_mount(unsigned int *cursor) {
    vga_print_at(cursor, "Mounting /dev/main1 to /mnt with UNFS filesystem\n", 0x07);
    vga_print_at(cursor, "Mount successful. /mnt is now ready for installation.\n", 0x0A);
}

static void cmd_cp_filesystem(unsigned int *cursor) {
    vga_print_at(cursor, "Copying WNU OS Server base system...\n", 0x0F);
    
    /* Simulate file copying progress */
    const char *files[] = {
        "/mnt/boot/kernel.bin",
        "/mnt/lib/libc.so.1", 
        "/mnt/lib/libm.so.1",
        "/mnt/bin/sh",
        "/mnt/bin/ls", 
        "/mnt/bin/ps",
        "/mnt/etc/fstab",
        "/mnt/etc/passwd",
        "/mnt/etc/group",
        NULL
    };
    
    for (int i = 0; files[i]; i++) {
        vga_print_at(cursor, "  -> ", 0x07);
        vga_print_at(cursor, files[i], 0x0A);
        vga_print_at(cursor, "\n", 0x07);
        
        /* Simulate copy delay */
        for (volatile int j = 0; j < 100000; j++);
    }
    
    vga_print_at(cursor, "Base system installed successfully.\n", 0x0A);
}

static void cmd_cp_pacman(unsigned int *cursor) {
    vga_print_at(cursor, "Installing PACMAN package manager...\n", 0x0F);
    vga_print_at(cursor, "  -> /mnt/usr/bin/pacman\n", 0x07);
    vga_print_at(cursor, "  -> /mnt/usr/bin/makepkg\n", 0x07);
    vga_print_at(cursor, "  -> /mnt/etc/pacman.conf\n", 0x07);
    vga_print_at(cursor, "  -> /mnt/var/lib/pacman/\n", 0x07);
    vga_print_at(cursor, "PACMAN installed successfully.\n", 0x0A);
}

static void cmd_grub_install(unsigned int *cursor) {
    vga_print_at(cursor, "Installing GRUB bootloader...\n", 0x0F);
    vga_print_at(cursor, "CRITICAL STEP - IF YOU DO THIS WRONG, REINSTALL EVERYTHING!\n", 0x0C);
    
    /* Simulate GRUB installation progress */
    vga_print_at(cursor, "Installing for x86_64-pc platform.\n", 0x07);
    for (volatile int i = 0; i < 200000; i++); // Simulate work
    
    vga_print_at(cursor, "Installation finished. No error reported.\n", 0x0A);
    vga_print_at(cursor, "Generating grub configuration file...\n", 0x07);
    
    for (volatile int i = 0; i < 150000; i++); // Simulate work
    vga_print_at(cursor, "Found WNU OS Server kernel: /boot/kernel.bin\n", 0x0A);
    vga_print_at(cursor, "done\n", 0x0A);
}

static void cmd_help(unsigned int *cursor) {
    vga_print_at(cursor, "WNU OS Server 1.0.0 Installation Commands:\n\n", 0x0F);
    vga_print_at(cursor, "Disk Management:\n", 0x0B);
    vga_print_at(cursor, "  fdisk -l                  List available disks\n", 0x07);
    vga_print_at(cursor, "  fdisk /dev/main           Partition main disk\n", 0x07);
    vga_print_at(cursor, "\nFilesystem:\n", 0x0B);
    vga_print_at(cursor, "  mkfs.unfs /dev/main1      Format with UNFS\n", 0x07);
    vga_print_at(cursor, "  mount /dev/main1 /mnt     Mount filesystem\n", 0x07);
    vga_print_at(cursor, "\nInstallation:\n", 0x0B);
    vga_print_at(cursor, "  cp -r ~/filesystem/* /mnt Copy base system\n", 0x07);
    vga_print_at(cursor, "  cp ~/pacman/* /mnt/usr/bin Install package manager\n", 0x07);
    vga_print_at(cursor, "  grub-install --root-directory=/mnt /dev/main\n", 0x07);
    vga_print_at(cursor, "\nUtilities:\n", 0x0B);
    vga_print_at(cursor, "  help                      Show this help\n", 0x07);
    vga_print_at(cursor, "  clear                     Clear screen\n", 0x07);
    vga_print_at(cursor, "  reboot                    Restart system\n", 0x07);
}

void installer_main(void) {
    unsigned int cursor = 0;
    char cmd_buffer[256];
    unsigned int cmd_len = 0;
    
    // Clear screen and show welcome
    vga_clear_screen();
    
    vga_print_at(&cursor, "WNU OS SERVER 1.0.0 Installation System (VMware Safe)\n", 0x0F);
    vga_print_at(&cursor, "(PLEASE UNDERSTAND THIS IS A PRE-ALPHA VERSION)\n", 0x0C);
    vga_print_at(&cursor, "================================================================\n\n", 0x08);
    
    vga_print_at(&cursor, "Welcome to the WNU OS Server installer!\n", 0x0A);
    vga_print_at(&cursor, "This installer uses a command-line interface like Arch Linux.\n", 0x07);
    vga_print_at(&cursor, "Running automated demonstration of installation process...\n\n", 0x0E);
    
    vga_print_at(&cursor, "NOTICE: Like TempleOS, WNU OS SERVER runs at Ring 0 (KERNEL).\n", 0x0E);
    vga_print_at(&cursor, "Everything you do WRONG is YOUR FAULT, not OURS. READ CAREFULLY.\n\n", 0x0C);
    
    // Main command loop with automated demo
    while (1) {
        // Show prompt
        vga_print_at(&cursor, "root@install:~# ", 0x0A);
        cmd_len = 0;
        
        // Get simulated input (no hardware I/O)
        while (1) {
            char key = vmware_safe_input();
            if (!key) {
                /* Demo finished - show completion message */
                vga_print_at(&cursor, "\n\nInstallation demonstration completed!\n", 0x0A);
                vga_print_at(&cursor, "WNU OS Server 1.0.0 is now installed and ready to boot.\n", 0x0A);
                vga_print_at(&cursor, "Remove installation media and restart the virtual machine.\n", 0x0E);
                vga_print_at(&cursor, "\nSystem will halt in 5 seconds...\n", 0x0C);
                
                /* Wait and halt safely */
                for (volatile int i = 0; i < 2000000; i++);
                __asm__ volatile ("cli; hlt");
                return;
            }
            
            if (key == '\r') { // Enter
                cmd_buffer[cmd_len] = '\0';
                vga_print_at(&cursor, "\n", 0x07);
                break;
            } else if (cmd_len < 255) { // Normal character
                cmd_buffer[cmd_len++] = key;
                vga_putc_at(cursor, key, 0x07);
                cursor++;
            }
        }
        
        // Process command
        if (strcmp(cmd_buffer, "fdisk -l") == 0) {
            cmd_fdisk_l(&cursor);
        } else if (strcmp(cmd_buffer, "fdisk /dev/main") == 0) {
            cmd_fdisk_main(&cursor);
        } else if (strcmp(cmd_buffer, "mkfs.unfs /dev/main1") == 0) {
            cmd_mkfs_unfs(&cursor);
        } else if (strcmp(cmd_buffer, "mount /dev/main1 /mnt") == 0) {
            cmd_mount(&cursor);
        } else if (strcmp(cmd_buffer, "cp -r ~/filesystem/* /mnt") == 0) {
            cmd_cp_filesystem(&cursor);
        } else if (strcmp(cmd_buffer, "cp ~/pacman/* /mnt/usr/bin") == 0) {
            cmd_cp_pacman(&cursor);
        } else if (strcmp(cmd_buffer, "grub-install --root-directory=/mnt /dev/main") == 0) {
            cmd_grub_install(&cursor);
        } else if (strcmp(cmd_buffer, "help") == 0) {
            cmd_help(&cursor);
        } else if (strcmp(cmd_buffer, "clear") == 0) {
            vga_clear_screen();
            cursor = 0;
        } else if (strcmp(cmd_buffer, "reboot") == 0) {
            vga_print_at(&cursor, "Installation complete! Rebooting...\n", 0x0A);
            vga_print_at(&cursor, "WNU OS Server 1.0.0 will boot from hard disk.\n", 0x0E);
            
            /* Safe halt without port I/O */
            for (volatile int i = 0; i < 1000000; i++);
            __asm__ volatile ("cli; hlt");
            return;
        } else if (cmd_len > 0) {
            vga_print_at(&cursor, "Command not found: ", 0x0C);
            vga_print_at(&cursor, cmd_buffer, 0x0C);
            vga_print_at(&cursor, "\nType 'help' for available commands.\n", 0x07);
        }
        
        vga_print_at(&cursor, "\n", 0x07);
        
        // Scroll if needed (simple implementation)
        if (cursor >= 80 * 24) {
            cursor = 80 * 20; // Reset to line 20
        }
    }
}