#!/bin/bash
# WNU OS Binutils Cross-Compiler Build Script
# Step 1: Building Binutils for x86_64-wnu target

set -e  # Exit on any error

# Configuration
WNU_TARGET="x86_64-elf-wnu"
WNU_PREFIX="/opt/wnu-toolchain"
WNU_SYSROOT="/opt/wnu-sysroot"
BINUTILS_VERSION="2.41"
JOBS=$(nproc)

echo "========================================"
echo "WNU OS Binutils Cross-Compiler Builder"
echo "========================================"
echo "Target: $WNU_TARGET"
echo "Prefix: $WNU_PREFIX"
echo "Sysroot: $WNU_SYSROOT" 
echo "Version: $BINUTILS_VERSION"
echo "Jobs: $JOBS"
echo ""

# Create directories
echo "Creating toolchain directories..."
sudo mkdir -p $WNU_PREFIX
sudo mkdir -p $WNU_SYSROOT
sudo chown -R $(whoami):$(whoami) $WNU_PREFIX
sudo chown -R $(whoami):$(whoami) $WNU_SYSROOT

# Download Binutils if not present
if [ ! -f "binutils-$BINUTILS_VERSION.tar.xz" ]; then
    echo "Downloading Binutils $BINUTILS_VERSION..."
    wget https://ftp.gnu.org/gnu/binutils/binutils-$BINUTILS_VERSION.tar.xz
fi

# Extract Binutils
echo "Extracting Binutils..."
if [ -d "binutils-$BINUTILS_VERSION" ]; then
    rm -rf "binutils-$BINUTILS_VERSION"
fi
tar -xf binutils-$BINUTILS_VERSION.tar.xz

# Create build directory
echo "Setting up build environment..."
mkdir -p build-binutils
cd build-binutils

echo "Configuring Binutils for WNU OS..."
../binutils-$BINUTILS_VERSION/configure \
    --target=$WNU_TARGET \
    --prefix=$WNU_PREFIX \
    --with-sysroot=$WNU_SYSROOT \
    --disable-nls \
    --disable-werror \
    --enable-gold \
    --enable-ld=default \
    --enable-plugins

echo ""
echo "Building Binutils (this may take 5-15 minutes)..."
make -j$JOBS

echo ""
echo "Installing Binutils..."
make install

echo ""
echo "========================================"
echo "WNU OS Binutils Build Complete! ✅"
echo "========================================"
echo "Tools installed in: $WNU_PREFIX/bin/"
echo ""
echo "WNU-specific tools created:"
echo "  - $WNU_TARGET-as      (WNU Assembler)"
echo "  - $WNU_TARGET-ld      (WNU Linker)" 
echo "  - $WNU_TARGET-objdump (WNU Object Dump)"
echo "  - $WNU_TARGET-objcopy (WNU Object Copy)"
echo "  - $WNU_TARGET-nm      (WNU Symbol Table)"
echo "  - $WNU_TARGET-strip   (WNU Strip)"
echo "  - $WNU_TARGET-ranlib  (WNU Archive Index)"
echo ""
echo "Add to PATH: export PATH=$WNU_PREFIX/bin:\$PATH"
echo ""
echo "Next step: Build GCC cross-compiler!"
echo "========================================"

# Test the tools
echo ""
echo "Testing WNU Binutils..."
if [ -x "$WNU_PREFIX/bin/$WNU_TARGET-as" ]; then
    echo "✅ $WNU_TARGET-as - Available"
    $WNU_PREFIX/bin/$WNU_TARGET-as --version | head -1
else
    echo "❌ $WNU_TARGET-as - NOT FOUND"
fi

if [ -x "$WNU_PREFIX/bin/$WNU_TARGET-ld" ]; then
    echo "✅ $WNU_TARGET-ld - Available" 
    $WNU_PREFIX/bin/$WNU_TARGET-ld --version | head -1
else
    echo "❌ $WNU_TARGET-ld - NOT FOUND"
fi

echo ""
echo "WNU OS Binutils ready for GCC build! 🚀"