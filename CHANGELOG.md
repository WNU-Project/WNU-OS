# Changelog

All notable changes to WNU OS will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.0.0] - 2025-10-11

### 🎉 Initial Release

The first stable release of WNU OS (Windows Not Unix) - a sophisticated Unix-like shell environment for Windows.

### ✨ Added

#### Core Shell Features

- **Complete Unix-style shell** with authentic command-line interface
- **Getty-style login screen** with `tty1` terminal identification
- **Colored prompts** - Green for normal users, plain text for root
- **Unix path display** - Forward slashes and `~` home directory support
- **Hidden password input** - True Unix-style authentication with no visual feedback

#### Authentication System

- **Multi-method authentication** supporting multiple credential types
- **Windows Hello PIN integration** with PowerShell verification
- **Smart credential detection** - Automatic PIN vs password recognition
- **Fallback authentication** - runas command and logon session validation
- **Enterprise security** - Multiple Windows authentication API support
- **User privilege management** - Root vs normal user modes with proper access control

#### Built-in Commands

- **Navigation**: `cd`, `pwd` with Unix-style path conversion
- **File operations**: Custom `ls` command with `-a` (show hidden) and `-l` (long format) flags
- **User management**: `whoami`, `su` for user switching between root and normal user
- **System information**: `uname -a` with proper architecture detection (x86_64, i686, arm, aarch64)
- **System control**: `shutdown`, `poweroff now`, `halt`, `reboot` with interrupt handling
- **Utilities**: `clear`/`cls` for screen clearing

#### System Management

- **Boot sequence simulation** with detailed service startup progress
- **Graceful shutdown procedures** with service stopping simulation
- **System halt with restart capability** - Ctrl+S+H to restart shell from halt state
- **Reboot with interrupt support** - Ctrl+C, ESC, or I key to cancel reboot
- **Real-time service status** with Unix epoch timestamps
- **ANSI color support** with loading bars and status indicators

#### Advanced Features

- **Interrupt handling** - Multiple methods (Ctrl+S+H, Ctrl+C, R key) for emergency controls
- **Home directory management** - Automatic path resolution and `~` substitution
- **Windows API integration** - Native system calls for authentication and file operations
- **Process simulation** - Realistic Unix-style system behavior on Windows
- **Error handling** - Comprehensive error checking with helpful user feedback

### 🏗️ Technical Implementation

#### Architecture

- **Native C implementation** using Windows APIs
- **Modular design** with separate components (boot, halt, poweroff, reboot, userlogin)
- **Memory safety** with proper buffer management
- **Cross-compilation support** for Windows environments

#### Build System

- **Makefile** for Unix-style building
- **build.bat** for Windows batch compilation
- **GitHub Actions CI/CD** with Windows runner and MinGW toolchain
- **Artifact generation** with automated build verification

#### Security Features

- **Multi-layer authentication** with enterprise-grade security
- **Privilege separation** between root and user contexts
- **Secure input handling** with hidden credential entry
- **Windows security integration** using native authentication APIs

### 🎨 User Experience

#### Interface Design

- **Authentic Unix feel** with proper command syntax and behavior
- **Visual feedback** with colored output and status indicators
- **Professional login experience** matching traditional Unix systems
- **Consistent prompt design** with hostname, username, and path display

#### Compatibility

- **Windows 10/11 support** with full API compatibility
- **MinGW/GCC compilation** for native Windows executables
- **Terminal compatibility** - Command Prompt, PowerShell, Windows Terminal
- **ANSI color support** via Virtual Terminal Processing

### 📚 Documentation

#### Project Documentation

- **Comprehensive README** files at root and version levels
- **Security policy** with vulnerability reporting procedures
- **Contributing guidelines** with development standards
- **Code of Conduct** for community participation
- **License** under GNU General Public License v3

#### Development Resources

- **Pull request templates** with structured review process
- **Issue templates** for bug reports and feature requests
- **GitHub workflows** for continuous integration
- **Build instructions** for multiple environments

### 🔧 Build and Installation

#### Compilation

```bash
# Using Make
make

# Using Windows batch script
build.bat

# Manual compilation
gcc -o wnuos.exe main.c boot.c halt.c poweroff.c reboot.c userlogin.c
```

#### System Requirements

- **Operating System**: Windows 10 or later
- **Compiler**: GCC (MinGW recommended)
- **Terminal**: ANSI color support recommended
- **Dependencies**: Windows SDK, standard C libraries

### 🐛 Known Issues

- **Quote handling**: Typing single quotes (`'`) may cause continuation prompt - type closing quote or Ctrl+C to exit
- **External commands**: Some Windows commands may behave differently than Unix equivalents
- **Path handling**: Mixed path separators in some edge cases

### 🎯 Usage Examples

#### Basic Login and Navigation

```bash
# Login as root (no password required)
WNU OS 1.0.0 COMPUTER tty1

COMPUTER login: root
Welcome, root!

# Navigate and list files
root@COMPUTER:~# pwd
/
root@COMPUTER:~# ls -la
# Shows detailed file listing with colors

# Switch to normal user
root@COMPUTER:~# su username
```

#### Authentication with PIN/Password

```bash
# Login with Windows username
COMPUTER login: tadeo
Password: [hidden input - no visual feedback]
✓ PIN authentication successful
Welcome, tadeo!

tadeo@COMPUTER:~$ whoami
FULL USERNAME: COMPUTER/tadeo
USER'S HOME DIRECTORY: C:/Users/tadeo
USERNAME: tadeo
```

### 🚀 Future Roadmap

#### Planned for 1.1.x

- Command history with arrow key navigation
- Tab completion for commands and file paths
- Enhanced file operations (mkdir, rmdir, cat, echo)
- Improved error messages and help system

#### Planned for 1.2.x

- Pipes and redirection (`|`, `>`, `>>`)
- Environment variables (`$PATH`, `$HOME`)
- Background processes (`&`)
- Process management commands (ps, kill)

#### Planned for 2.0.x

- Scripting support (.sh files)
- Network tools (ping, netstat, curl)
- Advanced text processing (grep, sed, awk)
- Package management system

---

### 🎉 Release Notes

**WNU OS 1.0.0** represents a complete Unix-like shell environment for Windows users. This initial release provides a solid foundation with enterprise-grade authentication, comprehensive system management, and an authentic Unix experience.

**Highlights:**

- ✅ **Production Ready** - Stable and tested core functionality
- 🔐 **Enterprise Security** - Advanced Windows Hello PIN integration
- 🎨 **Professional Interface** - Colored prompts and authentic Unix feel  
- 🛠️ **Complete Toolset** - All essential Unix commands implemented
- 📚 **Comprehensive Documentation** - Full project documentation and security policy

**Perfect for:**

- Windows users missing their Unix environment
- System administrators needing Unix tools on Windows
- Developers working in mixed Windows/Unix environments
- Anyone who prefers command-line interfaces

---

**Download WNU OS 1.0.0** and experience Unix on Windows! 🐧🪟

[1.0.0]: https://github.com/WNU-Project/WNU-OS/releases/tag/v1.0.0
