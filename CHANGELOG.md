# Changelog

All notable changes to WNU OS will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [1.0.1.update.1] - 2025-10-15

### Added

- WSYS2: `wsys2 upgrade all` — interactive command to upgrade all installed packages (iterates installed packages and attempts online install for each).

### Improved

- Hardened upgrade confirmation input parsing (skips whitespace and requires explicit `y`/`Y` to continue).
- Upgrade flow now attempts a best-effort `wsys2 remove` before installing the new package to mirror single-package behavior.
- Minor robustness fixes: safe package-name handling, small delay between package installs to avoid repository hammering, and added required includes.

### Fixed

- Resolved implicit-declaration warnings by adding missing prototypes and headers.
- Makefile adjustments to avoid stale object reuse that caused cross-architecture link failures in CI.

### Notes

- This update improves UX and robustness but the upgrade flow is not yet atomic — a failed install after removal may leave a package uninstalled. Implementing atomic upgrade (download → verify → install-to-temp → atomic swap with backup + rollback) is recommended for the next patch.

## [1.0.1] - 2025-10-11

### Added

**Major Package Management Release** - Complete package ecosystem with online repository integration and binary-safe compilation.

#### WSYS2 Package Manager

- **Complete package management system** with local and online capabilities
- **Real HTTP connectivity** to GitHub Pages repository ([https://wnu-project.github.io/wnuos.packages.com/](https://wnu-project.github.io/wnuos.packages.com/))
- **Download-first workflow** with packages stored in `C:\WNU\packages\online\`
- **Binary-safe file extraction** with proper PE executable handling
- **Package database tracking** with installation status management
- **Online repository integration** with search, list, and install commands
- **Package verification** with size validation and integrity checking

#### WNUPKG-COMPILER Package Compiler

- **Custom .wnupkg format creation** with binary-safe compilation
- **Recursive directory processing** with proper manifest generation
- **Binary mode file handling** preventing executable corruption
- **Manifest-based package structure** with install/uninstall scripts
- **Cross-platform package distribution** for WNU OS ecosystem

#### Enhanced WNU OS Shell (1.0.1)

- **Integrated WSYS2 package manager** accessible via shell commands
- **Multi-word command parsing** for complex package operations
- **Online command support** (`online list`, `online install`, `online search`)
- **Enhanced help system** with package management documentation
- **Seamless package installation** with automatic directory creation

#### Package Repository System

- **GitHub Pages hosting** for package distribution
- **JSON-based package metadata** with version tracking
- **RESTful API endpoints** for package discovery and download
- **Automatic package indexing** with real-time updates
- **Secure HTTPS downloads** with Windows wininet integration

#### Binary File Handling

- **PE executable extraction** with proper Windows compatibility
- **64-bit binary support** with architecture preservation
- **Corruption-free compilation** using consistent binary modes
- **Executable installation** with proper file permissions
- **Binary verification** ensuring executable integrity

#### Advanced Features

- **HTTP connectivity** using Windows wininet library
- **Recursive directory creation** for package installation
- **Package dependency tracking** with proper installation order
- **Command-line interface** with comprehensive help system
- **Error handling** with detailed user feedback

#### Technical Implementation

- **Native Windows API integration** for HTTP operations
- **Binary-safe file I/O** throughout entire pipeline
- **Memory management** with proper buffer handling
- **Cross-compilation support** for package distribution
- **Enterprise-grade reliability** with comprehensive error checking

### Technical Details

#### Package Format (.wnupkg)

```text
[MANIFEST_SIZE]\r\n
[MANIFEST_CONTENT]\r\n
[FILE_COUNT]\r\n
[FILE_PATH]\r\n[FILE_SIZE]\r\n[BINARY_DATA]
...
```

#### Installation Workflow

1. **Search online repository** via HTTP API
2. **Download package** to `C:\WNU\packages\online\`
3. **Verify package integrity** with size validation
4. **Extract files** using binary-safe parsing
5. **Execute installation** with proper directory creation

#### Build System

- **Enhanced Makefile** with wininet linking (`-lwininet`)
- **Windows installer scripts** (NSIS) for distribution
- **Automated compilation** with proper binary handling
- **Cross-platform compatibility** for Windows environments

### Usage Examples

#### Package Management

```bash
# Search for packages
wsys2 online search dev-tools
wsys2 online list

# Install packages from online repository  
wsys2 online install wnu-dev-tools

# Local package operations
wsys2 install local-package.wnupkg
wsys2 list
```

#### Package Compilation

```bash
# Create package from directory
wnupkg-compiler create my-package/ output.wnupkg

# Install compiled package
wsys2 install output.wnupkg
```

### 🎉 Major Breakthrough

This release represents a **complete package management ecosystem** built in a single day, including:

- ✅ **Real repository connectivity** with HTTP downloads
- ✅ **Binary-safe executable handling** with PE file support  
- ✅ **Complete compilation pipeline** from source to distribution
- ✅ **Professional package manager** with online/offline capabilities
- ✅ **Integrated shell experience** with seamless package operations

**Perfect for:**

- Software distribution in WNU OS ecosystem
- Binary package management on Windows
- Enterprise software deployment
- Developer tool distribution

---

## [1.0.0] - 2025-10-11

### Added

**Initial Release** - The first stable release of WNU OS (Windows Not Unix) - a sophisticated Unix-like shell environment for Windows.

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

#### Technical Implementation - Architecture

- **Native C implementation** using Windows APIs
- **Modular design** with separate components (boot, halt, poweroff, reboot, userlogin)
- **Memory safety** with proper buffer management
- **Cross-compilation support** for Windows environments

#### Technical Implementation - Build System

- **Makefile** for Unix-style building
- **build.bat** for Windows batch compilation
- **GitHub Actions CI/CD** with Windows runner and MinGW toolchain
- **Artifact generation** with automated build verification

#### Technical Implementation - Security Features

- **Multi-layer authentication** with enterprise-grade security
- **Privilege separation** between root and user contexts
- **Secure input handling** with hidden credential entry
- **Windows security integration** using native authentication APIs

#### User Experience - Interface Design

- **Authentic Unix feel** with proper command syntax and behavior
- **Visual feedback** with colored output and status indicators
- **Professional login experience** matching traditional Unix systems
- **Consistent prompt design** with hostname, username, and path display

#### User Experience - Compatibility

- **Windows 10/11 support** with full API compatibility
- **MinGW/GCC compilation** for native Windows executables
- **Terminal compatibility** - Command Prompt, PowerShell, Windows Terminal
- **ANSI color support** via Virtual Terminal Processing

#### Documentation - Project Files

- **Comprehensive README** files at root and version levels
- **Security policy** with vulnerability reporting procedures
- **Contributing guidelines** with development standards
- **Code of Conduct** for community participation
- **License** under GNU General Public License v3

#### Documentation - Development Resources

- **Pull request templates** with structured review process
- **Issue templates** for bug reports and feature requests
- **GitHub workflows** for continuous integration
- **Build instructions** for multiple environments

#### Build and Installation - Compilation

```bash
# Using Make
make

# Using Windows batch script
build.bat

# Manual compilation
gcc -o wnuos.exe main.c boot.c halt.c poweroff.c reboot.c userlogin.c
```

#### Build and Installation - System Requirements

- **Operating System**: Windows 10 or later
- **Compiler**: GCC (MinGW recommended)
- **Terminal**: ANSI color support recommended
- **Dependencies**: Windows SDK, standard C libraries

### Known Issues

- **Quote handling**: Typing single quotes (`'`) may cause continuation prompt - type closing quote or Ctrl+C to exit
- **External commands**: Some Windows commands may behave differently than Unix equivalents
- **Path handling**: Mixed path separators in some edge cases

### Usage Examples

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

**Download WNU OS** and experience Unix on Windows with complete package management! 🐧🪟📦

[Unreleased]: https://github.com/WNU-Project/WNU-OS/compare/v1.0.1...HEAD
[1.0.1]: https://github.com/WNU-Project/WNU-OS/releases/tag/v1.0.1
[1.0.0]: https://github.com/WNU-Project/WNU-OS/releases/tag/v1.0.0
