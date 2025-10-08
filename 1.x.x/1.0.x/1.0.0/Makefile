# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c11

# Output executable name
TARGET = wnuos.exe

# Source files
SRCS = main.c boot.c userlogin.c
OBJS = $(SRCS:.c=.o)

# Default build
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ -lgdi32 -luser32 -ladvapi32 -lnetapi32 -Wl,-subsystem,console

# Compile .c to .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	del /Q $(OBJS) $(TARGET) 2>nul || rm -f $(OBJS) $(TARGET)

.PHONY: all clean
