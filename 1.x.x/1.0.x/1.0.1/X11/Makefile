# Compiler and flags
CC = C:/msys64/mingw64/bin/gcc.exe
CFLAGS = -Wall -Wextra -std=c11 -Iwsys2

# Output executable name
TARGET = x11.exe

# Source files
SRCS = main.c x11.c xcalc.c shell_win.c
OBJS = $(SRCS:.c=.o)

# Default build
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ -L. -lraylib -lopengl32 -lwinmm -lgdi32 -luser32 -ladvapi32 -lnetapi32 -lkernel32 -lwininet -lws2_32

# Compile .c to .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
# Clean build artifacts (works in cmd or msys)
.PHONY: clean
clean:
	del .\\*.o
.PHONY: all
