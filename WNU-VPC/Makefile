# Compiler and flags
CC = C:\msys64\mingw64\bin\gcc.exe
CFLAGS = -Wall -Wextra -std=c11 -Iwsys2 -D_CRT_SECURE_NO_WARNINGS

# Output executable names
SHELL_TARGET = wnu-vpc.exe

# Source files
SHELL_SRCS = main.c
SHELL_OBJS = $(SHELL_SRCS:.c=.o)

# Libraries
SHELL_LIBS = -lopengl32 -lgdi32 -luser32 -ladvapi32 -lnetapi32 -lkernel32 -lwininet -lws2_32 -lwinmm

# Default build
all: clean $(SHELL_TARGET) $(GUI_TARGET)

# Build shell (console app)
$(SHELL_TARGET): $(SHELL_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ \
	-Wl,--start-group $(SHELL_LIBS) -Wl,--end-group \
	-Wl,-subsystem,console
# Compile .c to .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	@echo Cleaning build artifacts...
	-@del "C:\Users\tadeo\OneDrive\Documents\GitHub\WNU OS\WNU-VPC\*.o"

.PHONY: all clean
