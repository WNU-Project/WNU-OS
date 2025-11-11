#!/bin/bash
# WNUC Build and Test Utility Script
# This script helps build and test the WNU Compiler

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Script directory
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
WNUC_DIR="${SCRIPT_DIR}"

echo -e "${PURPLE}================================${NC}"
echo -e "${PURPLE}  WNUC Compiler Build Utility${NC}"
echo -e "${PURPLE}================================${NC}"
echo ""

# Function to print colored messages
print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check if we're in the right directory
if [ ! -f "wnuc.asm" ]; then
    print_error "wnuc.asm not found. Please run this script from the wnuc-compiler directory."
    exit 1
fi

# Function to show WNUC info
show_info() {
    print_status "WNUC Compiler Information:"
    echo -e "  ${CYAN}Name:${NC} WNU Compiler (WNUC)"
    echo -e "  ${CYAN}Version:${NC} 1.0"
    echo -e "  ${CYAN}Language:${NC} WNU-LANG"
    echo -e "  ${CYAN}Target:${NC} x86_64 Assembly"
    echo -e "  ${CYAN}Platform:${NC} WNU OS Server"
    echo -e "  ${CYAN}Source:${NC} wnuc.asm"
    echo -e "  ${CYAN}Sample:${NC} sample.wnulang"
    echo ""
}

# Function to build WNUC
build_wnuc() {
    print_status "Building WNUC Compiler..."
    
    if command -v make >/dev/null 2>&1; then
        make all
        if [ $? -eq 0 ]; then
            print_success "WNUC compilation completed successfully!"
        else
            print_error "WNUC compilation failed!"
            return 1
        fi
    else
        print_warning "Make not found, trying manual build..."
        
        # Create directories
        mkdir -p ../../../../../build/x86_64/wnuc
        mkdir -p ../../../../../dist/x86_64
        
        # Compile manually
        if command -v nasm >/dev/null 2>&1; then
            print_status "Assembling wnuc.asm..."
            nasm -f elf64 -g wnuc.asm -o ../../../../../build/x86_64/wnuc/wnuc.o
            
            if command -v ar >/dev/null 2>&1; then
                print_status "Creating library..."
                ar rcs ../../../../../dist/x86_64/libwnuc.a ../../../../../build/x86_64/wnuc/wnuc.o
                print_success "Manual build completed!"
            else
                print_error "ar (archiver) not found!"
                return 1
            fi
        else
            print_error "NASM assembler not found!"
            return 1
        fi
    fi
}

# Function to test WNUC
test_wnuc() {
    print_status "Testing WNUC Compiler..."
    
    if [ -f "sample.wnulang" ]; then
        print_status "Found sample WNU-LANG program:"
        echo -e "${CYAN}--- sample.wnulang ---${NC}"
        head -10 sample.wnulang
        echo -e "${CYAN}--- end sample ---${NC}"
        echo ""
        print_success "Sample program is ready for compilation!"
    else
        print_warning "sample.wnulang not found!"
    fi
    
    if [ -f "../../../../../dist/x86_64/libwnuc.a" ]; then
        print_success "WNUC library found: libwnuc.a"
    else
        print_warning "WNUC library not found!"
    fi
}

# Function to show help
show_help() {
    echo "WNUC Build Utility - Usage:"
    echo ""
    echo "  ./build.sh [command]"
    echo ""
    echo "Commands:"
    echo "  info      Show WNUC compiler information"
    echo "  build     Build the WNUC compiler"
    echo "  test      Test the WNUC compiler"
    echo "  clean     Clean build artifacts"
    echo "  all       Do everything (info + build + test)"
    echo "  help      Show this help message"
    echo ""
}

# Function to clean build
clean_build() {
    print_status "Cleaning WNUC build artifacts..."
    make clean 2>/dev/null || {
        print_warning "Make clean failed, trying manual cleanup..."
        rm -rf ../../../../../build/x86_64/wnuc
        rm -f ../../../../../dist/x86_64/libwnuc.a
        rm -f ../../../../../dist/x86_64/wnuc.bin
    }
    print_success "Cleanup completed!"
}

# Main script logic
case "${1:-all}" in
    "info")
        show_info
        ;;
    "build")
        build_wnuc
        ;;
    "test")
        test_wnuc
        ;;
    "clean")
        clean_build
        ;;
    "all")
        show_info
        build_wnuc
        test_wnuc
        ;;
    "help"|"-h"|"--help")
        show_help
        ;;
    *)
        print_error "Unknown command: $1"
        show_help
        exit 1
        ;;
esac

echo ""
print_success "WNUC utility script completed!"
echo -e "${PURPLE}================================${NC}"