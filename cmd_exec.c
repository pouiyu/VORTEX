// cmd_exec.c - 统一命令执行实现
#include "cmd_exec.h"
#include "fat32.h"
#include "vga.h"
#include "string.h"
#include "shell.h"
#include "system.h"
#include "brc.h"

// 添加 NULL 定义
#ifndef NULL
#define NULL ((void *)0)
#endif

/**
 * 统一命令执行函数
 * show_output: 1=显示详细输出（交互模式），0=静默模式（脚本NSO模式）
 */
int cmd_execute(const char *cmd, const char *args, int show_output) {
    
    /* ============ 系统信息命令 ============ */
    if (strcmp(cmd, "info") == 0) {
        show_system_info();
        return 1;
    }
    
    /* ============ 计算器 ============ */
    if (strcmp(cmd, "calc") == 0) {
        if (show_output) {
            calculator();
        } else {
            print("Calculator not available in script mode.\n");
        }
        return 1;
    }
    
    /* ============ 清屏 ============ */
    if (strcmp(cmd, "clear") == 0) {
        clear_screen();
        return 1;
    }
    
    /* ============ Logo ============ */
    if (strcmp(cmd, "logo") == 0) {
        show_logo();
        return 1;
    }
    
    /* ============ 系统控制 ============ */
    if (strcmp(cmd, "shutdown") == 0 || strcmp(cmd, "poweroff") == 0) {
        shutdown_system();
        return 1;
    }
    
    if (strcmp(cmd, "reboot") == 0 || strcmp(cmd, "restart") == 0) {
        reboot_system();
        return 1;
    }
    
    /* ============ 时间日期 ============ */
    if (strcmp(cmd, "time") == 0) {
        if (show_output) {
            unsigned char hour, minute, second;
            unsigned int year;
            unsigned char month, day;
            read_rtc_datetime(&hour, &minute, &second, &year, &month, &day);
            hour = (hour + 8) % 24;
            
            set_color(VGA_CYAN, VGA_BLACK);
            print("Current Time: ");
            set_color(VGA_GREEN, VGA_BLACK);
            print_two_digits(hour);
            putchar(':');
            print_two_digits(minute);
            putchar(':');
            print_two_digits(second);
            print("\n\n");
            set_color(VGA_LIGHT_GREY, VGA_BLACK);
        } else {
            print("Current Time: --:--:--\n\n");
        }
        return 1;
    }
    
    if (strcmp(cmd, "date") == 0) {
        if (show_output) {
            unsigned char hour, minute, second;
            unsigned int year;
            unsigned char month, day;
            read_rtc_datetime(&hour, &minute, &second, &year, &month, &day);
            
            set_color(VGA_CYAN, VGA_BLACK);
            print("Current Date: ");
            set_color(VGA_GREEN, VGA_BLACK);
            print_four_digits(year);
            putchar('/');
            print_two_digits(month);
            putchar('/');
            print_two_digits(day);
            print("\n\n");
            set_color(VGA_LIGHT_GREY, VGA_BLACK);
        } else {
            print("Current Date: ----/--/--\n\n");
        }
        return 1;
    }
    
    if (strcmp(cmd, "timecode") == 0) {
        if (show_output) {
            unsigned char hour, minute, second;
            unsigned int year;
            unsigned char month, day;
            read_rtc_datetime(&hour, &minute, &second, &year, &month, &day);
            hour = (hour + 8) % 24;
            
            unsigned short count = read_pit_count();
            unsigned short milliseconds = (count / 1193) % 1000;
            
            set_color(VGA_CYAN, VGA_BLACK);
            print("Timecode: ");
            set_color(VGA_GREEN, VGA_BLACK);
            print_four_digits(year);
            putchar('/');
            print_two_digits(month);
            putchar('/');
            print_two_digits(day);
            putchar('/');
            print_two_digits(hour);
            putchar(':');
            print_two_digits(minute);
            putchar(':');
            print_two_digits(second);
            putchar(':');
            
            if (milliseconds < 100) {
                putchar('0');
                if (milliseconds < 10) putchar('0');
            }
            print_int(milliseconds);
            print("\n");
            
            set_color(VGA_YELLOW, VGA_BLACK);
            print_four_digits(year);
            print_two_digits(month);
            print_two_digits(day);
            print_two_digits(hour);
            print_two_digits(minute);
            print_two_digits(second);
            if (milliseconds < 100) {
                putchar('0');
                if (milliseconds < 10) putchar('0');
            }
            print_int(milliseconds);
            print("\n\n");
            set_color(VGA_LIGHT_GREY, VGA_BLACK);
        } else {
            print("Timecode: ----/--/--/--:--:--:---\n");
            print("------------------\n\n");
        }
        return 1;
    }
    
    /* ============ 文件系统命令 ============ */
    
    /* mk - 创建文件 */
    if (strcmp(cmd, "mk") == 0) {
        if (args[0] == '\0') {
            if (show_output) print("Usage: mk <filename>\n");
            return 0;
        }
        if (fat32_create_file(args)) {
            return 1;
        } else {
            if (show_output) {
                print_color("ERROR: Failed to create file: ", VGA_RED, VGA_BLACK);
                print(args);
                putchar('\n');
            }
            return 0;
        }
    }
    
    /* md - 创建目录 */
    if (strcmp(cmd, "md") == 0) {
        if (args[0] == '\0') {
            if (show_output) print("Usage: md <dirname>\n");
            return 0;
        }
        if (fat32_create_directory(args)) {
            return 1;
        } else {
            if (show_output) {
                print_color("ERROR: Failed to create directory: ", VGA_RED, VGA_BLACK);
                print(args);
                putchar('\n');
            }
            return 0;
        }
    }
    
    /* rm - 删除文件/目录 */
    if (strcmp(cmd, "rm") == 0) {
        if (args[0] == '\0') {
            if (show_output) print("Usage: rm <path>\n");
            return 0;
        }
        if (fat32_delete(args)) {
            return 1;
        } else {
            if (show_output) {
                print_color("ERROR: Not found: ", VGA_RED, VGA_BLACK);
                print(args);
                putchar('\n');
            }
            return 0;
        }
    }
    
    /* ls - 列出目录 */
    if (strcmp(cmd, "ls") == 0) {
        fat32_list_directory("/");
        return 1;
    }
    
    /* cat - 显示文件内容 */
    if (strcmp(cmd, "cat") == 0) {
        if (args[0] == '\0') {
            if (show_output) print("Usage: cat <filename>\n");
            return 0;
        }
        unsigned char buffer[512];
        int bytes_read = fat32_read_file(args, buffer, 512);
        if (bytes_read > 0) {
            for (int i = 0; i < bytes_read; i++) {
                putchar(buffer[i]);
            }
            putchar('\n');
            return 1;
        } else {
            if (show_output) {
                print_color("ERROR: Cannot read file: ", VGA_RED, VGA_BLACK);
                print(args);
                putchar('\n');
            }
            return 0;
        }
    }
    
    /* df - 显示磁盘信息 */
    if (strcmp(cmd, "df") == 0) {
        print("\nDisk Partition: hd0\n");
        print("Filesystem: FAT32\n");
        print("Total space: 512 MB\n");
        print("Available: 400 MB\n\n");
        return 1;
    }
    
    /* wr - 写入文件（简化版，用于脚本） */
    if (strcmp(cmd, "wr") == 0) {
        if (args[0] == '\0') {
            if (show_output) print("Usage: wr <filename> <content>\n");
            return 0;
        }
        char *space_ptr = (char *)args;
        while (*space_ptr && *space_ptr != ' ') space_ptr++;
        if (*space_ptr == ' ') {
            *space_ptr = '\0';
            space_ptr++;
            while (*space_ptr == ' ') space_ptr++;
            
            if (!fat32_file_exists(args)) {
                fat32_create_file(args);
            }
            
            int content_len = strlen(space_ptr);
            if (fat32_write_file(args, space_ptr, content_len)) {
                if (show_output) {
                    print("File written: ");
                    print(args);
                    print(" (");
                    print_int(content_len);
                    print(" bytes)\n");
                }
                return 1;
            } else {
                if (show_output) {
                    print_color("ERROR: Failed to write file: ", VGA_RED, VGA_BLACK);
                    print(args);
                    putchar('\n');
                }
                return 0;
            }
        }
        return 0;
    }
    
    /* ============ 输出命令 ============ */
    
    /* echo - 回显文本 */
    if (strcmp(cmd, "echo") == 0) {
        if (args[0] != '\0') {
            print(args);
        }
        putchar('\n');
        return 1;
    }
    
    /* ============ 脚本控制命令 ============ */
    
    /* run - 执行脚本 */
    if (strcmp(cmd, "run") == 0) {
        if (args[0] == '\0') {
            if (show_output) print("Usage: run <filename.brc>\n");
            return 0;
        }
        return brc_execute(args);
    }
    
    /* ============ 格式化命令 ============ */
    
    /* format - 格式化磁盘 */
    if (strcmp(cmd, "format") == 0) {
        if (show_output) {
            print_color("WARNING: This will erase ALL data on the disk!\n", VGA_RED, VGA_BLACK);
            print_color("This action cannot be undone!\n", VGA_RED, VGA_BLACK);
            print("\nAre you sure you want to format? (yes/no): ");
            
            char confirm[8];
            read_line_with_history(confirm, 8);
            
            if (strcmp(confirm, "yes") == 0) {
                print("\nFormatting disk as FAT32...\n");
                if (fat32_format()) {
                    print_color("\nFormat complete!\n", VGA_GREEN, VGA_BLACK);
                    print("Re-initializing filesystem...\n");
                    if (fat32_init()) {
                        print_color("Filesystem mounted successfully!\n", VGA_GREEN, VGA_BLACK);
                    } else {
                        print_color("WARNING: Filesystem mount failed after format!\n", VGA_RED, VGA_BLACK);
                    }
                    return 1;
                } else {
                    print_color("\nFormat failed!\n", VGA_RED, VGA_BLACK);
                    return 0;
                }
            } else {
                print("\nFormat cancelled.\n");
                return 0;
            }
        } else {
            /* 脚本模式下直接格式化 */
            if (fat32_format()) {
                fat32_init();
                return 1;
            }
            return 0;
        }
    }
    
    /* ============ 脚本专用命令 ============ */
    
    /* pause - 暂停等待按键 */
    if (strcmp(cmd, "pause") == 0) {
        print("Press any key to continue...\n");
        getchar();
        return 1;
    }
    
    /* sleep - 延迟 */
    if (strcmp(cmd, "sleep") == 0) {
        int delay = args[0] != '\0' ? atoi(args) : 1;
        if (show_output) {
            print("Waiting ");
            print_int(delay);
            print(" second(s)...\n");
        }
        for (volatile int k = 0; k < delay * 10000000; k++);
        return 1;
    }
    
    /* ============ 未知命令 ============ */
    if (show_output) {
        set_color(VGA_RED, VGA_BLACK);
        print("Unknown command: ");
        print(cmd);
        print("\nType 'help' for available commands.\n");
        set_color(VGA_LIGHT_GREY, VGA_BLACK);
    }
    return 0;
}