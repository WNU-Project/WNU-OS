@echo off
REM Build WNU OS Binutils using patch file

echo Building WNU OS Binutils...

REM Navigate to toolchain directory
cd /d "C:\Users\tadeo\OneDrive\Documents\GitHub\WNU OS\SERVER\1.x.x\1.0.x\1.0.0\src\impl\x86_64\boot\live_server_os\toolchain"

REM Create temporary build script for Docker
echo set -e > build-script.sh
echo apt-get update >> build-script.sh
echo apt-get install -y build-essential wget git texinfo autotools-dev automake autoconf bison flex >> build-script.sh
echo cd /tmp >> build-script.sh
echo echo "Downloading binutils 2.43.1..." >> build-script.sh
echo wget -q https://ftp.gnu.org/gnu/binutils/binutils-2.43.1.tar.xz >> build-script.sh
echo tar -xf binutils-2.43.1.tar.xz >> build-script.sh
echo cd binutils-2.43.1 >> build-script.sh
echo echo "Applying WNU OS patch..." >> build-script.sh
echo git init >> build-script.sh
echo git add . >> build-script.sh
echo git commit -m "Initial binutils source" >> build-script.sh
echo git apply /workspace/patches/wnuos-binutils.patch >> build-script.sh
echo echo "Creating emulation parameter files..." >> build-script.sh
echo mkdir -p ld/emulparams >> build-script.sh
echo echo "source_sh \${srcdir}/emulparams/elf_i386.sh" ^> ld/emulparams/elf_i386_wnuos.sh >> build-script.sh
echo echo "TEXT_START_ADDR=0x08000000" ^>^> ld/emulparams/elf_i386_wnuos.sh >> build-script.sh
echo echo "source_sh \${srcdir}/emulparams/elf_x86_64.sh" ^> ld/emulparams/elf_x86_64_wnuos.sh >> build-script.sh
echo mkdir -p /tmp/build-binutils >> build-script.sh
echo cd /tmp/build-binutils >> build-script.sh
echo echo "Configuring binutils..." >> build-script.sh
echo ../binutils-2.43.1/configure --target=x86_64-wnuos-elf --prefix=/workspace/cross --with-sysroot --disable-nls --disable-werror --disable-multilib >> build-script.sh
echo echo "Building binutils..." >> build-script.sh
echo make -j$(nproc) >> build-script.sh
echo echo "Installing binutils..." >> build-script.sh
echo make install >> build-script.sh
echo echo "WNU OS Binutils build complete!" >> build-script.sh
echo echo "Binaries installed in: /workspace/cross/bin" >> build-script.sh
echo ls -la /workspace/cross/bin/x86_64-wnuos-elf-* >> build-script.sh

REM Run Docker with the build script
docker run -it --rm --privileged -v "%cd%:/workspace" ubuntu:latest

echo.
echo Build completed! Check the 'cross' directory for your WNU OS binutils.
pause