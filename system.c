// system.c - 系统控制实现（关机、重启）
#include "types.h"
#include "io.h"
#include "vga.h"
#include "keyboard.h"  /* 添加键盘头文件引用 */
#include "system.h"

/**
 * ACPI关机方式
 */
static void acpi_shutdown(void) {
    /* 尝试标准ACPI关机端口 */
    outw(0x604, 0x2000);  /* QEMU默认ACPI关机端口 */
    
    /* 如果上面不工作，尝试其他常见端口 */
    for(volatile int i = 0; i < 1000; i++);  /* 短暂延迟 */
    outw(0xB004, 0x2000);  /* 备选端口1 */
    
    for(volatile int i = 0; i < 1000; i++);
    outw(0x4004, 0x3400);  /* 备选端口2 */
}

/**
 * APM关机方式
 */
static void apm_shutdown(void) {
    /* 连接到APM BIOS */
    outb(0xB2, 0x00);  /* APM Command Port */
    outb(0xB3, 0x01);  /* 连接APM */
    
    /* 设置电源状态为关机 */
    outb(0xB2, 0x00);  
    outb(0xB3, 0x07);  /* Set Power State */
    
    outb(0xB2, 0x00);
    outb(0xB3, 0x03);  /* Power off */
}

/**
 * QEMU特定关机方式
 */
static void qemu_shutdown(void) {
    /* QEMU debug exit端口 */
    outb(0x501, 0x01);  /* QEMU debug exit - CPU0 halt */
    
    /* 其他QEMU可能的关机方式 */
    for(volatile int i = 0; i < 1000; i++);
    outw(0x604, 0x2000);  /* QEMU ACPI shutdown */
    
    for(volatile int i = 0; i < 1000; i++);
    outw(0x4004, 0x3400);  /* QEMU alternative */
}

/**
 * 键盘控制器重启
 */
static void kbd_reboot(void) {
    unsigned char temp;
    
    /* 等待键盘控制器准备好 */
    do {
        temp = inb(0x64);
    } while ((temp & 0x02) != 0);
    
    /* 发送重启命令 */
    outb(0x64, 0xFE);  /* Pulse CPU reset line */
    
    /* 如果重启失败，无限循环 */
    while(1) {
        __asm__ __volatile__("hlt");
    }
}

/**
 * 三重故障重启
 */
static void triple_fault_reboot(void) {
    /* 加载空IDT并触发中断导致三重故障 */
    struct {
        unsigned short limit;
        unsigned int base;
    } __attribute__((packed)) idt_ptr = {0, 0};
    
    /* 加载空IDT */
    __asm__ __volatile__("lidt %0" : : "m"(idt_ptr));
    
    /* 触发中断，由于IDT为空，会导致三重故障 */
    __asm__ __volatile__("int $0x00");
}

/**
 * PCI重启
 */
static void pci_reboot(void) {
    /* 配置PCI重启寄存器 */
    outb(0x0CF9, 0x06);  /* 写入重启命令到复位控制寄存器 */
    
    /* 短暂延迟 */
    for(volatile int i = 0; i < 1000; i++);
    
    /* 备用端口 */
    outb(0x0CF9, 0x0E);  /* 温暖重启 */
}

/**
 * 系统关机主函数
 */
void shutdown_system(void) {
    print_color("\nShutting down VortexOS...\n", VGA_YELLOW, VGA_BLACK);
    print("Attempting system shutdown...\n");
    
    /* 尝试多种关机方式 */
    print("Trying ACPI shutdown...\n");
    acpi_shutdown();
    
    /* 短暂延迟 */
    for(volatile int i = 0; i < 100000; i++);
    
    print("Trying APM shutdown...\n");
    apm_shutdown();
    
    /* 短暂延迟 */
    for(volatile int i = 0; i < 100000; i++);
    
    print("Trying QEMU shutdown...\n");
    qemu_shutdown();
    
    /* 如果所有方法都失败 */
    print_color("\nShutdown failed. You may power off the system manually.\n", VGA_RED, VGA_BLACK);
    print("Press any key to continue...\n");
    
    /* 等待按键 */
    while(!keyboard_has_data()) {
        __asm__ __volatile__("hlt");  /* 使用hlt降低功耗 */
    }
    inb(KEYBOARD_DATA_PORT);
}

/**
 * 系统重启主函数
 */
void reboot_system(void) {
    print_color("\nRebooting VortexOS...\n", VGA_YELLOW, VGA_BLACK);
    print("Attempting system reboot...\n");
    
    /* 短暂延迟，让消息显示 */
    for(volatile int i = 0; i < 50000; i++);
    
    /* 尝试多种重启方式 */
    print("Trying keyboard controller reboot...\n");
    kbd_reboot();
    
    /* 短暂延迟 */
    for(volatile int i = 0; i < 100000; i++);
    
    print("Trying PCI reboot...\n");
    pci_reboot();
    
    /* 短暂延迟 */
    for(volatile int i = 0; i < 100000; i++);
    
    print("Trying triple fault reboot...\n");
    triple_fault_reboot();
    
    /* 如果所有方法都失败 */
    print_color("\nReboot failed. Please restart the system manually.\n", VGA_RED, VGA_BLACK);
    while(1) {
        __asm__ __volatile__("hlt");
    }
}