# WNU OS SERVER - OSDev Wiki Reference Guide

## Development Philosophy
All WNU OS SERVER development will now follow OSDev Wiki best practices and standards.

## Primary Resources
- **Main Reference**: https://wiki.osdev.org/Expanded_Main_Page
- **Getting Started**: https://wiki.osdev.org/Getting_Started
- **Tutorials**: https://wiki.osdev.org/Tutorials

## Current WNU OS SERVER Status (1.0.1.BETA)
- ✅ Basic bootloader with GRUB
- ✅ x86_64 assembly kernel
- ✅ **ISO 9660 filesystem** (replacing VFL)
- ✅ SystemD service management
- ✅ Basic keyboard input with shift support
- ✅ Command system (ls, cat commands)
- ✅ Docker build environment
- ✅ QEMU testing setup
- ✅ **Disk I/O module** for reading sectors

## Next Development Areas (Based on OSDev Wiki)

### 1. File Systems (Current Implementation: ISO 9660)
- **Wiki Reference**: https://wiki.osdev.org/ISO_9660
- **Current State**: ISO 9660 filesystem implementation
- **Implementation Details**: 
  - Primary Volume Descriptor parsing
  - Directory record reading 
  - File location and size extraction
  - Case-insensitive filename matching
  - 2KB sector support
  - Both-endian format handling
- **OSDev Resources**:
  - ISO 9660: https://wiki.osdev.org/ISO_9660 ✅ IMPLEMENTED
  - File Systems: https://wiki.osdev.org/File_Systems
  - VFS: https://wiki.osdev.org/VFS (future enhancement)

### 2. Memory Management
- **Wiki Reference**: https://wiki.osdev.org/Memory_Management
- **Current State**: Basic memory access
- **Next Steps**: Implement paging and heap management
- **OSDev Resources**:
  - Paging: https://wiki.osdev.org/Paging
  - Higher Half Kernel: https://wiki.osdev.org/Higher_Half_x86_Bare_Bones
  - Memory Map: https://wiki.osdev.org/Detecting_Memory_(x86)

### 3. Multitasking
- **Wiki Reference**: https://wiki.osdev.org/Multitasking
- **Current State**: Single-threaded
- **Next Steps**: Implement process scheduling
- **OSDev Resources**:
  - Scheduling: https://wiki.osdev.org/Scheduling_Algorithms
  - Context Switching: https://wiki.osdev.org/Context_Switching
  - Processes: https://wiki.osdev.org/Process

### 4. Device Drivers
- **Wiki Reference**: https://wiki.osdev.org/Category:Device_Drivers
- **Current State**: Basic keyboard driver
- **Next Steps**: Expand driver system
- **OSDev Resources**:
  - PCI: https://wiki.osdev.org/PCI
  - USB: https://wiki.osdev.org/Universal_Serial_Bus
  - Network: https://wiki.osdev.org/Network_Stack

### 5. System Calls
- **Wiki Reference**: https://wiki.osdev.org/System_Calls
- **Current State**: Direct function calls
- **Next Steps**: Implement syscall interface
- **OSDev Resources**:
  - x86_64 Syscalls: https://wiki.osdev.org/SYSENTER
  - ABI: https://wiki.osdev.org/System_V_ABI

## Development Workflow
1. **Research Phase**: Always fetch relevant OSDev Wiki pages first
2. **Planning Phase**: Follow OSDev standards and best practices
3. **Implementation Phase**: Reference specific OSDev tutorials and examples
4. **Testing Phase**: Use OSDev testing methodologies
5. **Documentation Phase**: Document following OSDev conventions

## Reference Template for Future Features
When implementing any new OS feature:

```markdown
## Feature: [Feature Name]
- **OSDev Wiki Page**: [URL]
- **Current WNU OS State**: [Description]
- **Implementation Plan**: [Based on OSDev guidance]
- **Testing Strategy**: [OSDev testing approach]
- **Dependencies**: [Other OS components needed]
```

## Quality Standards
- Follow OSDev coding conventions
- Implement proper error handling as per OSDev guidelines
- Use standard OS data structures and algorithms
- Maintain compatibility with x86_64 architecture standards
- Document all implementations with OSDev references

---
*This document ensures WNU OS SERVER development follows industry-standard OS development practices via OSDev Wiki guidance.*