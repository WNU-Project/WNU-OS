#!/bin/bash
# WNU OS GCC Cross-Compiler Build Script
# Step 2: Building GCC for x86_64-wnu target (after Binutils)

set -e  # Exit on any error

# Configuration
WNU_TARGET="x86_64-elf-wnu"
WNU_PREFIX="/opt/wnu-toolchain"
WNU_SYSROOT="/opt/wnu-sysroot"
GCC_VERSION="13.2.0"
JOBS=$(nproc)

echo "========================================"
echo "WNU OS GCC Cross-Compiler Builder"
echo "========================================"
echo "Target: $WNU_TARGET"
echo "Prefix: $WNU_PREFIX"
echo "Sysroot: $WNU_SYSROOT" 
echo "Version: $GCC_VERSION"
echo "Jobs: $JOBS"
echo ""

# Check if Binutils is installed
if [ ! -x "$WNU_PREFIX/bin/$WNU_TARGET-as" ]; then
    echo "❌ ERROR: WNU Binutils not found!"
    echo "Please run build-wnu-binutils.sh first"
    exit 1
fi

echo "✅ WNU Binutils found - proceeding with GCC build"

# Add WNU toolchain to PATH
export PATH=$WNU_PREFIX/bin:$PATH

# Download GCC if not present
if [ ! -f "gcc-$GCC_VERSION.tar.xz" ]; then
    echo "Downloading GCC $GCC_VERSION..."
    wget https://ftp.gnu.org/gnu/gcc/gcc-$GCC_VERSION/gcc-$GCC_VERSION.tar.xz
fi

# Extract GCC
echo "Extracting GCC..."
if [ -d "gcc-$GCC_VERSION" ]; then
    rm -rf "gcc-$GCC_VERSION"
fi
tar -xf gcc-$GCC_VERSION.tar.xz

# Download GCC prerequisites
echo "Downloading GCC prerequisites..."
cd gcc-$GCC_VERSION
./contrib/download_prerequisites
cd ..

# Create build directory
echo "Setting up GCC build environment..."
mkdir -p build-gcc
cd build-gcc

echo "Configuring GCC for WNU OS..."
../gcc-$GCC_VERSION/configure \
    --target=$WNU_TARGET \
    --prefix=$WNU_PREFIX \
    --with-sysroot=$WNU_SYSROOT \
    --disable-nls \
    --disable-werror \
    --enable-languages=c,c++ \
    --without-headers \
    --disable-multilib \
    --disable-bootstrap \
    --disable-libssp \
    --disable-libgomp \
    --disable-libquadmath \
    --disable-libatomic \
    --enable-multiarch

echo ""
echo "Building GCC Stage 1 (this may take 20-60 minutes)..."
make -j$JOBS all-gcc

echo ""
echo "Building libgcc (basic runtime)..."
make -j$JOBS all-target-libgcc

echo ""
echo "Installing GCC..."
make install-gcc
make install-target-libgcc

echo ""
echo "========================================"
echo "WNU OS GCC Cross-Compiler Complete! ✅"
echo "========================================"
echo "Tools installed in: $WNU_PREFIX/bin/"
echo ""
echo "WNU GCC tools created:"
echo "  - $WNU_TARGET-gcc     (WNU C Compiler)"
echo "  - $WNU_TARGET-g++     (WNU C++ Compiler)" 
echo "  - $WNU_TARGET-cpp     (WNU Preprocessor)"
echo "  - $WNU_TARGET-gcov    (WNU Coverage Tool)"
echo ""
echo "Combined with Binutils:"
echo "  - $WNU_TARGET-as      (WNU Assembler)"
echo "  - $WNU_TARGET-ld      (WNU Linker)"
echo "  - $WNU_TARGET-objdump (WNU Object Tools)"
echo ""
echo "Add to PATH: export PATH=$WNU_PREFIX/bin:\$PATH"
echo ""
echo "Next step: Create WNU OS C Library!"
echo "========================================"

# Test the compiler
echo ""
echo "Testing WNU GCC..."
if [ -x "$WNU_PREFIX/bin/$WNU_TARGET-gcc" ]; then
    echo "✅ $WNU_TARGET-gcc - Available"
    $WNU_PREFIX/bin/$WNU_TARGET-gcc --version | head -1
    
    # Test compile
    echo ""
    echo "Testing WNU cross-compilation..."
    cat > test-wnu.c << 'EOF'
// WNU OS Test Program
void _start() {
    // Minimal WNU OS program
    while(1) {
        // WNU OS kernel loop
    }
}
EOF

    echo "Compiling test program for WNU OS..."
    if $WNU_PREFIX/bin/$WNU_TARGET-gcc -ffreestanding -nostdlib -c test-wnu.c -o test-wnu.o; then
        echo "✅ WNU cross-compilation successful!"
        $WNU_PREFIX/bin/$WNU_TARGET-objdump -h test-wnu.o
        rm -f test-wnu.c test-wnu.o
    else
        echo "❌ WNU cross-compilation failed"
    fi
else
    echo "❌ $WNU_TARGET-gcc - NOT FOUND"
fi

echo ""
echo "WNU OS Cross-Compiler Toolchain Ready! 🚀"