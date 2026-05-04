// shell.c - Shell命令处理实现（简化版）
#include "types.h"
#include "vga.h"
#include "keyboard.h"
#include "rtc.h"
#include "pit.h"
#include "string.h"
#include "system.h"
#include "fs.h"
#include "fat32.h"
#include "shell.h"
#include "brc.h"
#include "cmd_exec.h"  // 添加统一命令执行头文件

// 添加 NULL 定义
#ifndef NULL
#define NULL ((void *)0)
#endif

/* ASCII艺术 - VortexOS Logo */
void show_logo(void) {
    print("\n");
    set_color(VGA_CYAN, VGA_BLACK);
    print("                  _            \n");
    print(" __   _____  _ __| |_ _____  __\n");
    print(" \\ \\ / / _ \\| '__| __/ _ \\ \\/ /\n");
    print("  \\ V / (_) | |  | ||  __/>  < \n");
    print("   \\_/ \\___/|_|   \\__\\___/_/\\_\\\n\n");
}

/* 系统信息 */
void show_system_info(void) {
    print_color("=== VortexOS System Info ===\n", VGA_CYAN, VGA_BLACK);
    print("OS Name: VortexOS v1.2\n");
    print("Developer: Vortex Team\n");
    print("Kernel Type: Monolithic\n");
    print("Architecture: x86 (32-bit)\n");
    print_color("Features:\n", VGA_GREEN, VGA_BLACK);
    print("  * Text Mode VGA Driver\n");
    print("  * Keyboard Input Handler\n");
    print("  * Command Shell\n");
    print("  * System Control (shutdown/reboot)\n");
    print("  * FAT32 Filesystem Support\n");
    print("  * BRC Script Engine\n");
    print("  * Basic Utilities\n");
}

/* 简易计算器 */
void calculator(void) {
    char input[32];
    int num1, num2;
    char op;
    
    print("=== Simple Calculator ===\n");
    print("Enter first number: ");
    read_line_with_history(input, 32);
    num1 = atoi(input);
    
    print("Enter operator (+, -, *, /): ");
    op = getchar();
    putchar(op);
    putchar('\n');
    
    print("Enter second number: ");
    read_line_with_history(input, 32);
    num2 = atoi(input);
    
    print("Result: ");
    switch (op) {
        case '+': print_int(num1 + num2); break;
        case '-': print_int(num1 - num2); break;
        case '*': print_int(num1 * num2); break;
        case '/':
            if (num2 != 0) print_int(num1 / num2);
            else print("Error: Division by zero!");
            break;
        default: print("Unknown operator!");
    }
    print("\n");
}

/* 分割命令行为命令和参数 */
static void parse_command(char *command, char *cmd, char *args) {
    int i = 0, j = 0;
    
    while (command[i] == ' ') i++;
    
    while (command[i] != ' ' && command[i] != '\0') {
        cmd[j++] = command[i++];
    }
    cmd[j] = '\0';
    
    while (command[i] == ' ') i++;
    
    j = 0;
    while (command[i] != '\0') {
        args[j++] = command[i++];
    }
    args[j] = '\0';
}

/* Shell主循环 */
void shell(void) {
    char command[64];
    char cmd[8];
    char args[56];
    
    while (1) {
        /* 显示提示符 */
        set_color(VGA_GREEN, VGA_BLACK);
        print("vortex");
        set_color(VGA_LIGHT_GREY, VGA_BLACK);
        print("os");
        set_color(VGA_CYAN, VGA_BLACK);
        print(":~$ ");
        
        set_color(VGA_LIGHT_GREY, VGA_BLACK);
        
        /* 读取命令 */
        read_line_with_history(command, 64);
        
        /* 解析命令和参数 */
        parse_command(command, cmd, args);
        
        /* 执行命令 */
        if (strcmp(cmd, "help") == 0) {
            print_color("Available commands:\n", VGA_YELLOW, VGA_BLACK);
            print("  info     - Show system information\n");
            print("  calc     - Launch calculator\n");
            print("  time     - Show current time (HH:MM:SS)\n");
            print("  date     - Show current date (YYYY/MM/DD)\n");
            print("  timecode - Show full timestamp\n");
            print("  clear    - Clear the screen\n");
            print("  echo     - Display a line of text\n");
            print("  logo     - Show VortexOS logo\n");
            
            print_color("\nFile System Commands:\n", VGA_YELLOW, VGA_BLACK);
            print("  ls [dir]  - List directory contents\n");
            print("  mk <file> - Create a new file\n");
            print("  md <dir>  - Create a new directory\n");
            print("  rm <path> - Remove file/directory\n");
            print("  cat <file>- Display file contents\n");
            print("  wr <file> - Write to file (multi-line, end with EWT)\n");
            print("  df        - Show disk information\n");
            
            print_color("\nScript Commands:\n", VGA_YELLOW, VGA_BLACK);
            print("  run <file>- Execute a .brc script file\n");
            
            print_color("\nSystem Control:\n", VGA_YELLOW, VGA_BLACK);
            print("  shutdown  - Shutdown the system\n");
            print("  reboot    - Reboot the system\n");
            print("  format    - Format the disk\n");
            print("  help      - Show this help message\n");
            
        } else if (strcmp(cmd, "wr") == 0) {
            /* wr命令 - 多行写入文件（交互模式特有） */
            if (args[0] == '\0') {
                print("Usage: wr <filename>\n");
            } else {
                char filename[32];
                char line_buffer[256];
                char content[512] = "";
                int content_len = 0;
                int line_number = 1;
                
                strcpy(filename, args);
                
                if (!fat32_file_exists(filename)) {
                    fat32_create_file(filename);
                }
                
                print("Enter text (type 'EWT' on a new line to finish):\n");
                
                while (1) {
                    set_color(VGA_GREEN, VGA_BLACK);
                    print_int(line_number);
                    set_color(VGA_LIGHT_GREY, VGA_BLACK);
                    print("> ");
                    
                    read_line_with_history(line_buffer, 256);
                    
                    if (strcmp(line_buffer, "EWT") == 0) break;
                    
                    if (line_number > 1) {
                        if (content_len < 511) {
                            content[content_len] = '\n';
                            content_len++;
                        }
                    }
                    
                    int line_len = strlen(line_buffer);
                    for (int i = 0; i < line_len && content_len < 511; i++) {
                        content[content_len] = line_buffer[i];
                        content_len++;
                    }
                    content[content_len] = '\0';
                    line_number++;
                }
                
                if (fat32_write_file(filename, content, content_len)) {
                    print_color("File written successfully!\n", VGA_GREEN, VGA_BLACK);
                    print("File: ");
                    print(filename);
                    print(" (");
                    print_int(content_len);
                    print(" bytes)\n");
                } else {
                    print_color("ERROR: Failed to write file!\n", VGA_RED, VGA_BLACK);
                }
            }
            
        } else if (strcmp(cmd, "") == 0) {
            continue;
            
        } else {
            /* 使用统一命令执行函数 */
            cmd_execute(cmd, args, 1);  // show_output = 1（交互模式）
        }
    }
}