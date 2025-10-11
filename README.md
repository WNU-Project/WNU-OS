# WNU OS Project

## **Windows Not Unix Operating System**

A complete Unix-like operating system shell that brings authentic Unix experience to Windows environments.

## 🌟 Project Overview

WNU OS (Windows Not Unix) is a sophisticated shell environment that provides a true Unix-like experience on Windows systems. Built from the ground up in C, it features advanced authentication, system management, and a complete command-line interface that feels just like home for Unix users.

## 📁 Project Structure

```text
WNU OS/
├── README.md                    # This file - project overview
├── 1.x.x/                     # Version 1.x series
│   └── 1.0.x/                 # Version 1.0.x series  
│       └── 1.0.0/             # Current stable release
│           ├── main.c          # Core shell implementation
│           ├── boot.c/.h       # Boot sequence
│           ├── halt.c/.h       # System halt
│           ├── poweroff.c/.h   # Shutdown procedures  
│           ├── reboot.c/.h     # System restart
│           ├── userlogin.c/.h  # Authentication system
│           ├── wnuos.exe       # Compiled executable
│           ├── Makefile        # Build configuration
│           ├── build.bat       # Windows build script
│           ├── README.md       # Version-specific documentation
│           └── Windows Installer/ # Distribution package
└── [Future versions...]        # Planned releases
```

## 🚀 Current Release: Version 1.0.0

### Key Features

- 🔐 **Advanced Authentication** - Windows Hello PIN + Multi-method verification
- 🎨 **Unix-style Interface** - Colored prompts, forward slashes, `~` home directory
- 🛠️ **Built-in Commands** - `ls`, `cd`, `pwd`, `whoami`, `su`, system control
- ⚡ **System Management** - Boot sequences, graceful shutdown, interrupt handling
- 🛡️ **Enterprise Security** - Multi-layer authentication with fallback methods

### What's New in 1.0.0

- Complete custom `ls` implementation with `-a` and `-l` flags
- Advanced Windows Hello PIN integration
- Colored prompts (green for users, plain for root)
- Getty-style login screen with authentic Unix feel
- Comprehensive system control with interrupt handling

## 📋 Quick Start

### Installation

1. **Download** the latest release from `/1.x.x/1.0.x/1.0.0/`
2. **Run** `WNU OS-1.0.0-win64.exe` or build from source
3. **Login** as `root` (no password) or use your Windows credentials

### Building from Source

```bash
cd "1.x.x/1.0.x/1.0.0/"
make
# or
build.bat
# or  
gcc -o wnuos.exe main.c boot.c halt.c poweroff.c reboot.c userlogin.c
```

## 🎯 Version History

| Version | Status | Features |
|---------|--------|----------|
| **1.0.0** | ✅ **Current** | Full authentication, custom ls, colored prompts, system management |
| 1.1.x | 🔄 Planned | Enhanced file operations, command history, tab completion |
| 1.2.x | 📋 Roadmap | Pipes & redirection, environment variables, scripting |
| 2.0.x | 🌟 Future | Advanced process management, networking tools |

## 🛠️ Development

### Project Goals

- **Authentic Unix Experience** - Feel like native Unix/Linux on Windows
- **Enterprise Ready** - Advanced security and authentication  
- **User Friendly** - Modern colored interface with helpful feedback
- **Extensible** - Clean architecture for future enhancements

### Contributing

1. Fork the repository
2. Create a feature branch from the appropriate version directory
3. Implement changes in the relevant version folder
4. Test thoroughly on Windows environment
5. Submit pull request with version targeting

### Architecture Philosophy

- **Version Isolation** - Each version maintains its own complete codebase
- **Incremental Development** - Features build upon previous stable versions
- **Windows Native** - Uses Windows APIs for maximum compatibility
- **Unix Authentic** - Behavior matches traditional Unix systems

## 🔧 Technical Stack

- **Language**: C (Native Windows compilation)
- **APIs**: Windows Authentication, File System, Console
- **Compiler**: GCC (MinGW recommended)  
- **Target**: Windows 10/11 with ANSI color support
- **Dependencies**: Windows SDK, standard C libraries

## 🌍 Community

### WNU-Project Organization

This project is part of the larger WNU-Project initiative to bring Unix tools and philosophy to Windows environments.

### Getting Help

- 📖 **Documentation**: Check version-specific README files
- 🐛 **Issues**: Report bugs in the GitHub Issues
- 💬 **Discussions**: Join community discussions
- 🔧 **Development**: Contribute code via pull requests

## 📄 License

This project is licensed under [LICENSE] - see the LICENSE file for details.

## 🎉 Acknowledgments

- Inspired by traditional Unix shells (bash, zsh, csh)
- Built for Windows users who miss their Unix environment
- Community feedback and contributions
- The philosophy of "Windows Not Unix" - bringing the best of both worlds

---

**WNU OS Project** - *Making Windows feel like home for Unix users* 🐧🪟

> "Why choose between Windows and Unix when you can have both?"
