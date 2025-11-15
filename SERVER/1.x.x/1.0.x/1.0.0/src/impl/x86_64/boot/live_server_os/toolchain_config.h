/*
 * WNU OS Toolchain Configuration
 * OS-specific toolchain definitions following OSDev standards
 */

#ifndef WNU_TOOLCHAIN_CONFIG_H
#define WNU_TOOLCHAIN_CONFIG_H

/* WNU OS Target Identification */
#define TARGET_WNU_OS 1
#define __wnu__ 1
#define __unix__ 1

/* Toolchain Architecture Support */
#define WNU_ARCH_X86_64 1
#define WNU_ARCH_I386 1

/* Cross-Platform Compatibility Layers */
#define WNU_LINUX_COMPAT 1
#define WNU_WINDOWS_COMPAT 1
#define WNU_MACOS_COMPAT 1

/* Target Triplets */
#define WNU_TARGET_X86_64 "x86_64-wnu"
#define WNU_TARGET_I386 "i386-wnu"

/* Standard Library Configuration */
#define WNU_LIBC_PATH "/lib/libc.a"
#define WNU_LIBK_PATH "/lib/libk.a"

/* Linker Configuration */
#define WNU_TEXT_START_ADDR 0x08000000
#define WNU_LINK_SPEC "%{shared:-shared} %{static:-static}"

/* Compiler Definitions */
#define WNU_CPP_BUILTINS() \
    do { \
        builtin_define("__wnu__"); \
        builtin_define("__unix__"); \
        builtin_assert("system=wnu"); \
        builtin_assert("system=unix"); \
        builtin_assert("system=posix"); \
    } while(0)

/* Cross-Compilation Targets */
typedef enum {
    WNU_TARGET_NATIVE = 0,
    WNU_TARGET_LINUX,
    WNU_TARGET_WINDOWS,
    WNU_TARGET_MACOS,
    WNU_TARGET_MAX
} wnu_target_os_t;

/* Toolchain Commands */
#define WNU_GCC_NATIVE "x86_64-wnu-gcc"
#define WNU_GCC_LINUX "x86_64-linux-gnu-gcc"
#define WNU_GCC_WINDOWS "x86_64-w64-mingw32-gcc"
#define WNU_GCC_MACOS "x86_64-apple-darwin-gcc"

#endif /* WNU_TOOLCHAIN_CONFIG_H */