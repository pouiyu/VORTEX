/* link.ld - 链接器脚本 */
ENTRY(start)

SECTIONS {
    . = 1M;                /* 内核加载到1MB位置 */
    
    .text BLOCK(4K) : ALIGN(4K) {
        *(.multiboot)
        *(.text)
    }
    
    .rodata BLOCK(4K) : ALIGN(4K) {
        *(.rodata)
    }
    
    .data BLOCK(4K) : ALIGN(4K) {
        *(.data)
    }
    
    .bss BLOCK(4K) : ALIGN(4K) {
        *(COMMON)
        *(.bss)
    }
}