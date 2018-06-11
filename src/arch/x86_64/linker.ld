ENTRY(start)

SECTIONS {
    . = 1M;

    .boot :
    {
        *(.multiboot_header)
    }

    .text :
    {
        *(.text, .comment)
    }

    .debug :
    {
        *(.debug*)
    }

    .bss : ALIGN(4K)
    {
        *(.bss)
    }
}
