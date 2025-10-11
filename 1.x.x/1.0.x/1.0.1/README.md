# WNU OS 1.0.0

## **Windows Not Unix Operating System Shell**

A sophisticated Unix-like shell environment that runs natively on Windows, providing an authentic Unix experience with advanced authentication and system management capabilities.

## 🌟 Features

### 🔐 Advanced Authentication

- **Windows Hello PIN Integration** - Native Windows authentication support
- **Multi-method Verification** - PowerShell, runas, and logon session validation
- **Smart Credential Detection** - Automatic PIN vs password recognition
- **Enterprise Security** - Multiple fallback authentication mechanisms

### 🎨 Modern Shell Experience  

- **Getty-style Login Screen** - Authentic Unix login experience
- **Colored Prompts** - Green for users, plain for root
- **Unix-style Path Display** - Forward slashes and `~` home directory
- **Hidden Password Input** - True Unix-style authentication (no visual feedback)

### 🛠️ Built-in Commands

- **System Control**: `shutdown`, `poweroff`, `halt`, `reboot` with interrupt handling
- **Navigation**: `cd`, `pwd`, `ls` (with `-a`, `-l` flags)
- **User Management**: `whoami`, `su` (switch user)
- **System Info**: `uname -a` with architecture detection  
- **Utilities**: `clear`/`cls`

### ⚡ System Management

- **Boot Sequence** - Detailed service startup simulation
- **Graceful Shutdown** - Proper service stopping with progress indicators
- **Interrupt Handling** - Ctrl+S+H, Ctrl+C emergency controls
- **Service Status** - Real-time system state monitoring

## 🚀 Quick Start

### Building

```bash
# Using Make
make

# Or using build script
build.bat

# Or manual compilation
gcc -o wnuos.exe main.c boot.c halt.c poweroff.c reboot.c userlogin.c
```

### Running

```bash
./wnuos.exe
```

## 📖 Usage

### Login

- **Root access**: Login as `root` (no password required)
- **User access**: Use your Windows username with Windows password or PIN
- **PIN Support**: 4-6 digit PINs or alphanumeric like "1234"

### Commands

#### File Operations

```bash
ls              # List files and directories
ls -l           # Long format with details  
ls -a           # Show hidden files
ls -la          # Long format + hidden files
cd <directory>  # Change directory
pwd             # Print working directory
```

#### System Information

```bash
whoami          # Current user information
uname -a        # System information with architecture
```

#### User Management  

```bash
su root         # Switch to root user
su <username>   # Switch to normal user
```

#### System Control

```bash
poweroff        # Shutdown in 10 seconds
poweroff now    # Immediate shutdown
halt            # System halt (Ctrl+S+H to restart)
reboot          # System reboot (Ctrl+C/ESC/I to interrupt)
```

## 🎯 Architecture

### Core Components

- **main.c** - Primary shell with authentication and command processing
- **boot.c** - System boot sequence simulation - systemd-like
- **halt.c** - System halt with interrupt handling - systemd-like
- **poweroff.c** - Graceful shutdown procedures - systemd-like
- **reboot.c** - System restart with interruption support - systemd-like
- **userlogin.c** - User authentication utilities

### Authentication Flow

1. **Credential Detection** - Analyze input format (PIN vs password)
2. **Windows Hello** - PowerShell-based PIN verification  
3. **System Integration** - Multiple Windows authentication APIs
4. **Fallback Methods** - Runas and logon session validation
5. **Access Control** - Root vs user privilege management

## 🔧 Technical Details

### Windows Integration

- Native Windows API usage (`LogonUserA`, `GetComputerNameA`)
- ANSI color code support via Virtual Terminal Processing
- Windows Hello PIN compatibility
- File system operations using Windows APIs

### Unix Compatibility

- Forward slash path separators
- Home directory shortcuts (`~`)  
- Hidden password input (no asterisks)
- Unix-style command syntax and behavior
- POSIX-compliant command responses

## 🎨 Customization

### Prompt Colors

- **Normal Users**: Green username@hostname, blue path, green `$`
- **Root User**: Plain text for serious system administration

### Interrupt Controls

- **Ctrl+S+H**: Restart from halt state
- **Ctrl+C**: Cancel operations  
- **ESC/I**: Interrupt reboot sequence
- **R**: Quick restart (backup method)

## 🛡️ Security Features

- **Multi-layer Authentication** - PIN, password, and Windows integration
- **Privilege Separation** - Distinct root and user modes
- **Secure Input** - Hidden password entry without visual feedback
- **Session Management** - Proper user context switching

## 📋 Requirements

- **OS**: Windows 10/11 (tested)
- **Compiler**: GCC (MinGW recommended)
- **APIs**: Windows Authentication APIs
- **Terminal**: ANSI color support (Windows Terminal recommended)

## 🏗️ Development

### Project Structure

```text
WNU-OS/
├── 1.x.x/1.0.x/1.0.0/
│   ├── main.c              # Core shell implementation
│   ├── boot.c/.h           # Boot sequence  
│   ├── halt.c/.h           # System halt
│   ├── poweroff.c/.h       # Shutdown procedures
│   ├── reboot.c/.h         # System restart
│   ├── userlogin.c/.h      # Authentication
│   ├── Makefile            # Build configuration
│   ├── build.bat           # Windows build script
│   └── Windows Installer/   # Distribution package
```

### Contributing

1. Fork the repository
2. Create a feature branch
3. Implement your changes
4. Test on Windows environment  
5. Submit a pull request

## 📄 License

This project is part of the WNU-Project organization. See LICENSE file for details.

## 🎉 Acknowledgments

Created as part of the "Windows Not Unix" project - bringing authentic Unix experience to Windows environments.

---

**WNU OS** - *Where Windows meets Unix philosophy* 🐧🪟
