# WNU OS SERVER 1.0.0-alpha Build Instructions

## Quick Build (Recommended)

### Using Docker/WSL
```bash
# Navigate to build directory
cd "WNU OS/SERVER/1.x.x/1.0.x/1.0.0"

# Build everything
make all

# Output: dist/x86_64/WNU-OS-SERVER-1.0.0.iso
```

## Individual Components

### Build Main OS
```bash
make build_x86_64
```
**Output:** `dist/x86_64/kernel.bin` and bootable ISO

### Build Minimal Kernel
```bash
make minimal_kernel  
```
**Output:** `dist/x86_64/minimal_kernel.bin`

### Build Live Server
```bash
make live_wnu_os_server
```
**Output:** `dist/x86_64/live_wnu_os_server.bin`

### Build WNUC Compiler
```bash
make wnuc_compiler
```
**Output:** Custom compiler tools

## Clean Build
```bash
make clean
```
Removes all build artifacts

## Testing

### QEMU Emulation
```bash
qemu-system-x86_64 -cdrom dist/x86_64/WNU-OS-SERVER-1.0.0.iso -m 512M
```

### VirtualBox
1. Create new VM (Other/Other 64-bit)
2. Attach ISO as CD/DVD
3. Boot VM

### Physical Hardware
1. Burn ISO to USB: `dd if=dist/x86_64/WNU-OS-SERVER-1.0.0.iso of=/dev/sdX bs=1M`
2. Boot from USB

## Troubleshooting

### Build Fails
- Ensure Linux environment (Docker/WSL)
- Install: `nasm`, `gcc`, `ld`, `grub-mkrescue`
- Check available disk space (>1GB)

### ISO Won't Boot  
- Verify GRUB installation
- Check BIOS/UEFI compatibility
- Try different virtual machine software

---
**Build Time:** ~2-5 minutes | **ISO Size:** ~6MB