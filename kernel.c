// kernel.c - 内核主文件
#include "types.h"
#include "vga.h"
#include "shell.h"
#include "ata.h"
#include "fat32.h"
#include "keyboard.h"  /* 添加键盘头文件 */

/* 内核主函数 */
void kernel_main() {
    /* 清屏并初始化 */
    clear_screen();
    
    /* 启用光标 */
    enable_cursor(13, 14);
    
    /* 设置默认颜色 */
    set_color(VGA_LIGHT_GREY, VGA_BLACK);
    
    /* 显示欢迎信息 */
    show_logo();
    
    print_color("Welcome to VortexOS!\n", VGA_GREEN, VGA_BLACK);
    print_color("Initializing system...\n\n", VGA_CYAN, VGA_BLACK);
    
    /* 初始化ATA驱动 */
    if (!ata_init()) {
        print_color("WARNING: No ATA drive detected!\n", VGA_RED, VGA_BLACK);
        print("File system operations will not be available.\n\n");
    } else {
        /* 检查磁盘是否已经被VortexOS格式化过 */
        if (fat32_is_formatted()) {
            /* 磁盘已经格式化过，直接初始化 */
            print("Found existing VortexOS filesystem.\n");
            if (fat32_init()) {
                print_color("Filesystem mounted successfully!\n", VGA_GREEN, VGA_BLACK);
                print("Type 'help' for available commands.\n\n");
            } else {
                print_color("ERROR: Failed to mount filesystem!\n", VGA_RED, VGA_BLACK);
                print("The disk may be corrupted. Use 'format' to reformat.\n\n");
            }
        } else {
            /* 磁盘未格式化，提示用户 */
            if (!fat32_init()) {
                print_color("Disk is not formatted.\n", VGA_YELLOW, VGA_BLACK);
                print("Do you want to format the disk as FAT32? (y/n): ");
                
                char response = getchar();
                putchar(response);
                putchar('\n');
                
                if (response == 'y' || response == 'Y') {
                    print("\nFormatting disk...\n");
                    if (fat32_format()) {
                        print_color("Format successful!\n", VGA_GREEN, VGA_BLACK);
                        /* 重新初始化文件系统 */
                        if (fat32_init()) {
                            print_color("Filesystem mounted successfully!\n", VGA_GREEN, VGA_BLACK);
                            print("Type 'help' for available commands.\n\n");
                        } else {
                            print_color("ERROR: Filesystem mount failed after format!\n", VGA_RED, VGA_BLACK);
                        }
                    } else {
                        print_color("Format failed!\n", VGA_RED, VGA_BLACK);
                        print("You can try the 'format' command later.\n\n");
                    }
                } else {
                    print("\nYou can use the 'format' command to format the disk later.\n");
                    print("Note: Some features require a formatted disk.\n\n");
                }
            } else {
                /* 磁盘可能由其他工具格式化，但未被VortexOS格式化 */
                print_color("Found a FAT32 filesystem (not created by VortexOS).\n", VGA_YELLOW, VGA_BLACK);
                print("Mounting anyway...\n");
                
                /* 初始化文件系统 */
                if (fat32_init()) {
                    print_color("Filesystem mounted successfully!\n", VGA_GREEN, VGA_BLACK);
                    print("Type 'help' for available commands.\n\n");
                }
            }
        }
    }
    
    /* 进入Shell */
    shell();
    
    /* Shell不会返回 */
    while(1);
}