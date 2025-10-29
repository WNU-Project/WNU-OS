# Docker Linux Is Used to Run this Makefile.
.PHONY: all
all: build_x86_64 minimal_kernel install_wnu_os_server

$(x86_64_asm_object_files): build/x86_64/%.o: src/impl/x86_64/%.asm
	@mkdir -p $(dir $@)
	nasm -f elf64 $< -o $@
$(x86_64_c_object_files): build/x86_64/%.o: src/impl/x86_64/%.c
	@mkdir -p $(dir $@)
	x86_64-elf-gcc -c -I src/intf -ffreestanding $(patsubst build/x86_64/%.o, src/impl/x86_64/%.c, $@) -o $@ -O2 -Wall -Wextra

build/kernel/%.o: src/impl/kernel/%.c
	@mkdir -p $(dir $@)
	x86_64-elf-gcc -c -I src/intf -ffreestanding $< -o $@ -O2 -Wall -Wextra

.PHONY: build_x86_64
build_x86_64: build/x86_64/boot/header.o build/x86_64/boot/main.o $(kernel_object_files)
	@echo "Building Main OS Components"
	mkdir -p dist/x86_64
	# Ensure all object files are up to date
	$(MAKE) build/x86_64/boot/header.o
	$(MAKE) build/x86_64/boot/main.o
	# Ensure C kernel object files are built
	$(MAKE) $(kernel_object_files)
	# Link header.o first and only once
	x86_64-elf-ld -n -o dist/x86_64/kernel.bin -T src/impl/x86_64/linker.ld build/x86_64/boot/header.o build/x86_64/boot/main.o $(kernel_object_files)
	cp dist/x86_64/kernel.bin targets/x86_64/iso/boot/kernel.bin
	grub-mkrescue /usr/lib/grub/i386-pc -o dist/x86_64/WNU-OS-SERVER-1.0.0.iso targets/x86_64/iso

build/x86_64/boot/%.o: src/impl/x86_64/boot/%.asm
	@mkdir -p $(dir $@)
	nasm -f elf64 $< -o $@
.PHONY:  minimal_kernel
minimal_kernel: src/impl/x86_64/minimal_kernel.asm src/impl/x86_64/minimal_linker.ld
	@echo "Building Minimal Kernel"
	mkdir -p build/x86_64 dist/x86_64
	nasm -f elf64 src/impl/x86_64/minimal_kernel.asm -o build/x86_64/minimal_kernel.o
	x86_64-elf-ld -n -o dist/x86_64/minimal_kernel.bin -T src/impl/x86_64/minimal_linker.ld build/x86_64/minimal_kernel.o
	cp dist/x86_64/minimal_kernel.bin targets/x86_64/iso/boot/minimal_kernel.bin
	cp dist/x86_64/minimal_kernel.bin targets/x86_64/iso/boot/kernel_minimal.bin
	grub-mkrescue /usr/lib/grub/i386-pc -o dist/x86_64/WNU-OS-SERVER-1.0.0.iso targets/x86_64/iso/
install_wnu_os_server: src/impl/x86_64/boot/install_server_os/main.asm src/impl/x86_64/linker.ld
	@echo "Building Install Components"
	mkdir -p build/x86_64 dist/x86_64
	nasm -f elf64 src/impl/x86_64/boot/install_server_os/header.asm -o build/x86_64/install_header.o
	nasm -f elf64 src/impl/x86_64/boot/install_server_os/main.asm -o build/x86_64/install_wnu_os_server.o
	x86_64-elf-ld -n -o dist/x86_64/install_wnu_os_server.bin -T src/impl/x86_64/linker.ld build/x86_64/install_header.o build/x86_64/install_wnu_os_server.o
	cp dist/x86_64/install_wnu_os_server.bin targets/x86_64/iso/boot/install_wnu_os_server.bin
	grub-mkrescue /usr/lib/grub/i386-pc -o dist/x86_64/WNU-OS-SERVER-1.0.0.iso targets/x86_64/iso/
.PHONY: clean
clean:
	rm -rf build dist
kernel_source_files := $(shell find src/impl/kernel -name *.c)
kernel_object_files := $(patsubst src/impl/kernel/%.c, build/kernel/%.o, $(kernel_source_files))

x86_64_asm_source_files := $(shell find src/impl/x86_64 -name *.asm)
x86_64_asm_object_files := $(patsubst src/impl/x86_64/%.asm, build/x86_64/%.o, $(x86_64_asm_source_files))

x86_64_object_files := $(x86_64_asm_object_files)