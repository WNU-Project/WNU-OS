# WSYS2 Package Manager Makefile
# Part of WNU OS 1.0.1

CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -O2
TARGET = wsys2.exe
SOURCES = main.c wsys2.c package.c
OBJECTS = $(SOURCES:.c=.o)
HEADERS = wsys2.h

# Default target
all: $(TARGET)

# Build wsys2.exe
$(TARGET): $(OBJECTS)
	@echo "Linking WSYS2..."
	$(CC) $(OBJECTS) -o $(TARGET) -lwininet
	@echo "✓ WSYS2 built successfully!"

# Build wnupkg-compiler.exe
$(COMPILER_TARGET): wnupkg-compiler.c
	@echo "Building WNUPKG Compiler..."
	$(CC) $(CFLAGS) wnupkg-compiler.c -o $(COMPILER_TARGET)
	@echo "✓ WNUPKG Compiler built successfully!"

# Build object files
%.o: %.c $(HEADERS)
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build files
clean:
	@echo "Cleaning build files..."
	del /f *.o $(TARGET) $(COMPILER_TARGET) 2>nul || true
	@echo "✓ Clean complete"
# Run tests
test: $(TARGET)
	@echo "Running WSYS2 tests..."
	.\$(TARGET) version
	.\$(TARGET) help
	.\$(TARGET) list
	@echo "✓ Basic tests completed"

# Compile example package
example-package: $(COMPILER_TARGET)
	@echo "Compiling example package..."
	.\$(COMPILER_TARGET) example-package example-app.wnupkg
	@echo "✓ Example package compiled!"

# Show help
help:
	@echo "WSYS2 Package Manager Build System"
	@echo ""
	@echo "Targets:"
	@echo "  all              Build wsys2.exe and wnupkg-compiler.exe (default)"
	@echo "  clean            Remove build files"
	@echo "  install          Install to C:\WNU\bin (requires admin)"
	@echo "  example-package  Compile example-package/ into example-app.wnupkg"
	@echo "  sample-packages  Create all sample .wnupkg packages"
	@echo "  hello-world-package Create Hello World demo package"
	@echo "  vim-package      Create Vim editor package"
	@echo "  git-package      Create Git VCS package"  
	@echo "  python-package   Create Python interpreter package"
	@echo "  test-package     Create basic test package"
	@echo "  test             Run basic tests"
	@echo "  help             Show this help"

.PHONY: all clean install example-package sample-packages hello-world-package vim-package git-package python-package test-package test help