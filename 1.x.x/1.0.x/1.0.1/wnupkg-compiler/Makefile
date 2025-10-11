# WNUPKG Compiler Makefile
# Part of WNU OS 1.0.1

CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -O2

# Output executable name
TARGET = wnupkg-compiler.exe

# Source files
SRCS = main.c
OBJS = $(SRCS:.c=.o)

# Default build
all: $(TARGET)

# Build wnupkg-compiler.exe
$(TARGET): $(OBJS)
	@echo "Building WNUPKG Compiler..."
	$(CC) $(CFLAGS) -o $@ $^
	@echo "✓ WNUPKG Compiler built successfully!"

# Compile .c to .o
%.o: %.c
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	@echo "Cleaning build files..."
	del /Q $(OBJS) $(TARGET) 2>nul || rm -f $(OBJS) $(TARGET)
	@echo "✓ Clean complete"

# Test compiler
test: $(TARGET)
	@echo "Testing WNUPKG Compiler..."
	.\$(TARGET) --help
	@echo "✓ Test completed"

# Show help
help:
	@echo "WNUPKG Compiler Build System"
	@echo ""
	@echo "Targets:"
	@echo "  all      Build wnupkg-compiler.exe (default)"
	@echo "  clean    Remove build files"
	@echo "  test     Test the compiler"
	@echo "  help     Show this help"

.PHONY: all clean test help
