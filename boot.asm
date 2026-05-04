; boot.asm - 系统启动文件
; 编译: nasm -f elf32 boot.asm -o boot.o

section .multiboot
align 4
    dd 0x1BADB002          ; multiboot魔数
    dd 0x03                ; flags
    dd -(0x1BADB002 + 0x03) ; 校验和

section .text
global start
extern kernel_main

start:
    mov esp, stack_top     ; 设置栈指针
    push eax               ; 传递multiboot magic number
    push ebx               ; 传递multiboot info structure
    call kernel_main       ; 调用C内核函数
    
    cli                    ; 禁用中断
.hang:
    hlt
    jmp .hang              ; 无限循环

section .bss
align 16
stack_bottom:
    resb 16384             ; 16KB栈空间
stack_top: