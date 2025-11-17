#!/bin/bash
set -e

echo "=== WNU OS Binutils Build Script ==="
echo "Building binutils with WNU OS target support..."

# Step 1: Download and extract binutils
cd /tmp
echo "Downloading binutils 2.43.1..."
wget -q https://ftp.gnu.org/gnu/binutils/binutils-2.43.1.tar.xz
tar -xf binutils-2.43.1.tar.xz
cd binutils-2.43.1

# Step 2: Create the emulation parameter files (new files not in patch)
echo "Creating emulation parameter files..."
mkdir -p ld/emulparams

cat > ld/emulparams/elf_i386_wnuos.sh << 'EOF'
source_sh ${srcdir}/emulparams/elf_i386.sh
TEXT_START_ADDR=0x08000000
EOF

cat > ld/emulparams/elf_x86_64_wnuos.sh << 'EOF'
source_sh ${srcdir}/emulparams/elf_x86_64.sh
EOF

# Step 3: Configure for WNU OS target
echo "Configuring binutils for x86_64-wnuos-elf..."
mkdir -p /tmp/build-binutils
cd /tmp/build-binutils

../binutils-2.43.1/configure \
    --target=x86_64-wnuos-elf \
    --prefix=/workspace/cross \
    --with-sysroot \
    --disable-nls \
    --disable-werror \
    --disable-multilib

# Step 4: Build and install
echo "Building binutils (this may take a while)..."
make -j$(nproc)

echo "Installing binutils..."
make install

echo "=== Build Complete! ==="
echo "WNU OS binutils installed to: /workspace/cross/bin/"
echo "Available tools:"
ls -1 /workspace/cross/bin/x86_64-wnuos-elf-*

echo ""
echo "Test your new tools:"
echo "/workspace/cross/bin/x86_64-wnuos-elf-ld --help"
echo "/workspace/cross/bin/x86_64-wnuos-elf-as --help"