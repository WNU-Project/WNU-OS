# WSYS2 Package Manager

**WSYS2** is the official package manager for WNU OS, providing a simple and powerful way to install, manage, and update software packages.

## Features

- 📦 **`.wnupkg` Package Format** - Custom package format for WNU OS
- 🔧 **Simple Commands** - Easy-to-use command line interface  
- 🏠 **User & System Packages** - Support for both user and system-wide installations
- 🔍 **Package Search** - Find packages quickly
- 📋 **Package Database** - Track installed packages and dependencies
- 🎨 **Colored Output** - Beautiful terminal interface

## Installation

Build WSYS2 from source:

```bash
cd wsys2/
make
```

Install system-wide (requires admin):
```bash
make install
```

## Usage

### Install a Package
```bash
wsys2 install package.wnupkg
```

### Remove a Package
```bash
wsys2 remove package-name
```

### List Installed Packages
```bash
wsys2 list
```

### Search for Packages
```bash
wsys2 search editor
```

### Update All Packages
```bash
wsys2 update
```

### Show Package Information
```bash
wsys2 info package-name
```

### Show Version
```bash
wsys2 version
```

## Package Format (.wnupkg)

WSYS2 uses `.wnupkg` files, which are compressed archives containing:

- `manifest.json` - Package metadata
- `install.bat` - Installation script
- `files/` - Package files
- `uninstall.bat` - Removal script

### Example manifest.json
```json
{
  "name": "vim",
  "version": "9.0.0",
  "description": "Vi IMproved text editor",
  "author": "Bram Moolenaar",
  "dependencies": [],
  "files": [
    "bin/vim.exe",
    "share/vim/"
  ]
}
```

## Directory Structure

- **System packages**: `C:\WNU\packages\` (requires admin)
- **User packages**: `%USERPROFILE%\.local\packages\`
- **Database**: `installed.db` (tracks installed packages)
- **Temporary**: `%TEMP%\wsys2\` (extraction workspace)

## Integration with WNU OS

WSYS2 integrates seamlessly with WNU OS 1.0.1+:

```bash
# From WNU OS shell
wsys2 install git.wnupkg
wsys2 list
```

The `wsys2` command is available directly in the WNU OS shell, making package management effortless.

## Development

### Building
```bash
make all        # Build wsys2.exe
make clean      # Clean build files
make test       # Run basic tests
```

### Creating Test Packages
```bash
make test-package  # Create test package structure
```

## Version History

- **v1.0.0** - Initial release with WNU OS 1.0.1
  - Basic install/remove/list functionality
  - `.wnupkg` package format support
  - User and system package directories
  - Package database management

## Contributing

WSYS2 is part of the WNU OS project. Contributions welcome!

## License

Part of WNU OS - see main project license.