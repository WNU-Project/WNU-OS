#!/bin/bash
set -e

apt-get update
apt-get install -y build-essential wget git texinfo autotools-dev automake autoconf bison flex

cd /tmp
echo "Downloading binutils 2.43.1..."
wget -q https://ftp.gnu.org/gnu/binutils/binutils-2.43.1.tar.xz
tar -xf binutils-2.43.1.tar.xz
cd binutils-2.43.1

echo "Applying WNU OS patch..."
git init
git add .
git commit -m "Initial binutils source"
git apply /workspace/patches/wnuos-binutils.patch

echo "Creating emulation parameter files..."
mkdir -p ld/emulparams
cat > ld/emulparams/elf_i386_wnuos.sh << 'EOFPARAM'
source_sh ${srcdir}/emulparams/elf_i386.sh
TEXT_START_ADDR=0x08000000
EOFPARAM

cat > ld/emulparams/elf_x86_64_wnuos.sh << 'EOFPARAM'
source_sh ${srcdir}/emulparams/elf_x86_64.sh
EOFPARAM

mkdir -p /tmp/build-binutils
cd /tmp/build-binutils

echo "Configuring binutils..."
../binutils-2.43.1/configure \
    --target=x86_64-wnuos-elf \
    --prefix=/workspace/cross \
    --with-sysroot \
    --disable-nls \
    --disable-werror \
    --disable-multilib

echo "Building binutils..."
make -j$(nproc)

echo "Installing binutils..."
make install

echo "WNU OS Binutils build complete!"
echo "Binaries installed in: /workspace/cross/bin"
ls -la /workspace/cross/bin/x86_64-wnuos-elf-*