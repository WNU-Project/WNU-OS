# WNU OS SERVER 1.0.0-alpha Release

## 🎉 First Alpha Release!

Welcome to the first alpha release of **WNU OS SERVER** - a custom operating system with server capabilities, GUI support, and custom tooling.

## ✨ Features

### Core Components
- **Boot System** - Custom assembly bootloader with GRUB support
- **Minimal Kernel** - Lightweight kernel for basic operations
- **Live Server OS** - Full server environment with system services

### System Services
- **systemd** - System and service manager
- **networkd** - Network management
- **Filesystem Support** - ISO9660 and disk I/O operations
- **Driver Framework** - Hardware abstraction layer

### Development Tools
- **WNUC Compiler** - Custom compiler for WNU OS applications
- **VBE GUI** - VESA BIOS Extensions graphics support
- **Custom Toolchain** - OS-specific development environment

## 🛠️ Build Requirements

### Prerequisites
- **Docker** with Linux container support
- **Git** for repository management
- **4GB+ RAM** recommended for building

### Build Environment
```bash
# Clone the repository
git clone https://github.com/WNU-Project/WNU-OS.git
cd "WNU OS/SERVER/1.x.x/1.0.x/1.0.0"

# Build in Docker/WSL Linux environment
make all
```

### Build Products
- `dist/x86_64/WNU-OS-SERVER-1.0.0.iso` - Bootable ISO image
- `dist/x86_64/kernel.bin` - Main kernel binary
- `dist/x86_64/minimal_kernel.bin` - Minimal kernel
- `dist/x86_64/live_wnu_os_server.bin` - Live server binary

## 🚨 Alpha Release Notice

**This is an ALPHA release** - experimental and for testing purposes only.

### Known Limitations
- Requires specific Linux build environment
- Limited hardware compatibility testing
- Documentation is minimal
- Some features may be incomplete

### Not Recommended For
- Production use
- Critical systems
- Inexperienced users without Linux knowledge

## 📖 Quick Start

1. **Build the OS:**
   ```bash
   make all
   ```

2. **Test with QEMU:**
   ```bash
   qemu-system-x86_64 -cdrom dist/x86_64/WNU-OS-SERVER-1.0.0.iso
   ```

3. **Boot on hardware:**
   - Burn ISO to USB/DVD
   - Boot from USB/DVD drive

## 🐛 Bug Reports

Found issues? Please report them on our GitHub repository with:
- System specifications
- Build environment details
- Error messages/logs
- Steps to reproduce

## 📅 Roadmap

### Beta Release Goals
- Improved hardware compatibility
- Enhanced documentation
- Stability improvements
- Additional system services

---

**WNU OS PROJECT** | Version 1.0.0-alpha | November 14, 2025