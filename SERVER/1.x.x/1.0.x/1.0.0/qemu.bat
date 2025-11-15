cd "%~dp0"
echo Serial pipe will be available at: \\.\pipe\wnucom
"C:\msys64\mingw64\bin\qemu-system-x86_64.exe" -cdrom "C:\Users\tadeo\OneDrive\Documents\GitHub\WNU OS\SERVER\1.x.x\1.0.x\1.0.0\dist\x86_64\WNU-OS-SERVER-1.0.0.iso" -m 512 -serial stdio -vga std -display sdl