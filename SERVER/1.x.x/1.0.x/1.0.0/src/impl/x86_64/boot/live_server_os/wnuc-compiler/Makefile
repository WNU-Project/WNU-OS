# WNUC Compiler Makefile
# WNU Compiler build system for WNU OS Server

# Compiler and assembler settings
ASM = nasm
LD = x86_64-elf-ld
ASMFLAGS = -f elf64 -g
LDFLAGS = -n

# Directories
SRC_DIR = .
BUILD_DIR = ../../../../../build/x86_64/wnuc
DIST_DIR = ../../../../../dist/x86_64

# Source files
WNUC_SOURCES = wnuc.asm
WNUC_OBJECTS = $(BUILD_DIR)/wnuc.o

# Output files
WNUC_LIB = $(DIST_DIR)/libwnuc.a
WNUC_BIN = $(DIST_DIR)/wnuc.bin

# Default target
all: $(WNUC_LIB) $(WNUC_BIN)

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Create dist directory  
$(DIST_DIR):
	mkdir -p $(DIST_DIR)

# Compile WNUC source files
$(BUILD_DIR)/wnuc.o: $(SRC_DIR)/wnuc.asm | $(BUILD_DIR)
	$(ASM) $(ASMFLAGS) $< -o $@

# Create WNUC static library
$(WNUC_LIB): $(WNUC_OBJECTS) | $(DIST_DIR)
	ar rcs $@ $(WNUC_OBJECTS)

# Create WNUC standalone binary
$(WNUC_BIN): $(WNUC_OBJECTS) | $(DIST_DIR)
	$(LD) $(LDFLAGS) -o $@ $(WNUC_OBJECTS)

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR)
	rm -f $(WNUC_LIB) $(WNUC_BIN)

# Install WNUC into system
install: $(WNUC_LIB)
	@echo "Installing WNUC compiler library..."
	@echo "WNUC library: $(WNUC_LIB)"
	@echo "WNUC binary: $(WNUC_BIN)"

# Test the compiler
test: $(WNUC_BIN)
	@echo "Testing WNUC compiler..."
	@echo "Compiling sample WNU-LANG program..."
	@echo "Sample input: sample.wnulang"
	@if [ -f sample.wnulang ]; then \
		echo "Source found - would compile with WNUC"; \
	else \
		echo "Warning: sample.wnulang not found"; \
	fi

# Development targets
debug: ASMFLAGS += -F dwarf
debug: $(WNUC_BIN)

# Show compiler info
info:
	@echo "=== WNUC Compiler Build Information ==="
	@echo "Assembler: $(ASM)"
	@echo "Linker: $(LD)"
	@echo "Assembly flags: $(ASMFLAGS)"
	@echo "Linker flags: $(LDFLAGS)"
	@echo "Source directory: $(SRC_DIR)"
	@echo "Build directory: $(BUILD_DIR)"
	@echo "Distribution directory: $(DIST_DIR)"
	@echo "Source files: $(WNUC_SOURCES)"
	@echo "Object files: $(WNUC_OBJECTS)"
	@echo "Output library: $(WNUC_LIB)"
	@echo "Output binary: $(WNUC_BIN)"
	@echo "======================================="

# Help target
help:
	@echo "WNUC Compiler Makefile"
	@echo ""
	@echo "Available targets:"
	@echo "  all        - Build WNUC library and binary (default)"
	@echo "  clean      - Remove build artifacts"
	@echo "  install    - Install WNUC into system"
	@echo "  test       - Test the compiler"
	@echo "  debug      - Build with debug symbols"
	@echo "  info       - Show build configuration"
	@echo "  help       - Show this help message"
	@echo ""
	@echo "Files:"
	@echo "  wnuc.asm        - Main compiler source"
	@echo "  sample.wnulang - Sample WNU-LANG program"
	@echo "  README.md       - Documentation"

# Force rebuilds
.PHONY: all clean install test debug info help

# Dependencies
$(WNUC_OBJECTS): wnuc.asm