# WNUPKG Package Compiler

A tool for compiling directory structures into .wnupkg package files for WNU OS.

## Features

- 🔧 **Easy Package Creation** - Convert directories into packages
- 📦 **Standard Format** - Creates proper .wnupkg files
- 🎯 **Manifest Support** - Reads package metadata from manifest.txt
- 📁 **File Scanning** - Automatically includes all files in the files/ directory
- 🔄 **Script Integration** - Supports install.bat and uninstall.bat scripts

## Usage

```bash
# Build the compiler
make

# Basic usage
wnupkg-compiler example-package/

# Custom output name
wnupkg-compiler my-app/ my-custom-package.wnupkg

# Show help
wnupkg-compiler --help
```

## Directory Structure

Your source directory should follow this structure:

```
my-package/
├── manifest.txt      # Package metadata (required)
├── install.bat       # Installation script (optional)
├── uninstall.bat     # Uninstallation script (optional)
└── files/           # Files to be packaged
    ├── bin/         # Executable files
    ├── docs/        # Documentation
    └── ...          # Any other directories/files
```

## Manifest Format

The `manifest.txt` file contains package metadata:

```ini
name=my-package
version=1.0.0
description=My awesome package
author=Your Name
install_path=C:\\Program Files\\MyPackage
```

## Examples

See the `example-package/` directory for a complete example.

## Installation

1. Build the compiler: `make`
2. Copy to system PATH: `copy wnupkg-compiler.exe C:\WNU\bin\`
3. Use from anywhere: `wnupkg-compiler my-package/`