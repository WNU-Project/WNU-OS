ENTRY(_start)
SECTIONS {
    . = 1M;
    .multiboot_header : { *(.multiboot_header) }
    .text : { *(.text) }
}