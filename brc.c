// brc.c - BRC脚本执行实现（简化版）
#include "brc.h"
#include "fat32.h"
#include "vga.h"
#include "string.h"
#include "shell.h"
#include "cmd_exec.h"  // 添加统一命令执行头文件

// 添加 NULL 定义
#ifndef NULL
#define NULL ((void *)0)
#endif

/* 执行BRC脚本文件 */
int brc_execute(const char *filename) {
    char script_buffer[4096];
    int bytes_read;
    
    /* 检查文件是否存在 */
    if (!fat32_file_exists(filename)) {
        print_color("ERROR: Script file not found: ", VGA_RED, VGA_BLACK);
        print(filename);
        putchar('\n');
        return 0;
    }
    
    /* 读取脚本内容 */
    bytes_read = fat32_read_file(filename, script_buffer, 4096);
    if (bytes_read <= 0) {
        print_color("ERROR: Cannot read script file!\n", VGA_RED, VGA_BLACK);
        return 0;
    }
    
    if (bytes_read < 4096) {
        script_buffer[bytes_read] = '\0';
    } else {
        script_buffer[4095] = '\0';
        bytes_read = 4095;
    }
    
    /* NSO模式标志 */
    int nso_mode = 0;
    
    /* 显示脚本标题 */
    print_color("Executing script: ", VGA_CYAN, VGA_BLACK);
    print(filename);
    print("\n========================================\n");
    
    /* 逐行执行脚本 */
    char *line_ptr = script_buffer;
    int line_number = 0;
    int success_count = 0;
    int error_count = 0;
    
    while (*line_ptr != '\0') {
        char line[256];
        int line_len = 0;
        
        /* 提取一行 */
        while (*line_ptr != '\0' && *line_ptr != '\n' && line_len < 255) {
            line[line_len] = *line_ptr;
            line_len++;
            line_ptr++;
        }
        line[line_len] = '\0';
        
        if (*line_ptr == '\n') line_ptr++;
        
        line_number++;
        
        /* 跳过空行 */
        if (line[0] == '\0') continue;
        
        /* 跳过前导空格 */
        char *cmd_start = line;
        while (*cmd_start == ' ' || *cmd_start == '\t') cmd_start++;
        if (*cmd_start == '\0') continue;
        
        /* 解析命令和参数 */
        char cmd[32];
        char args[224];
        int i = 0, j = 0;
        
        while (cmd_start[i] != ' ' && cmd_start[i] != '\0' && j < 31) {
            cmd[j] = cmd_start[i];
            i++; j++;
        }
        cmd[j] = '\0';
        
        while (cmd_start[i] == ' ' || cmd_start[i] == '\t') i++;
        
        j = 0;
        while (cmd_start[i] != '\0' && j < 223) {
            args[j] = cmd_start[i];
            i++; j++;
        }
        args[j] = '\0';
        
        /* 处理NSO/NSE命令 */
        if (strcmp(cmd, "NSO") == 0) {
            nso_mode = 1;
            success_count++;
            continue;
        }
        
        if (strcmp(cmd, "NSE") == 0) {
            nso_mode = 0;
            print("========================================\n");
            print_color("Script output resumed.\n", VGA_CYAN, VGA_BLACK);
            success_count++;
            continue;
        }
        
        /* 注释行 */
        if (line[0] == '#' || line[0] == ';') {
            if (!nso_mode) {
                set_color(VGA_DARK_GREY, VGA_BLACK);
                print("  [");
                print_int(line_number);
                print("] ");
                print(line);
                putchar('\n');
                set_color(VGA_LIGHT_GREY, VGA_BLACK);
            }
            continue;
        }
        
        /* 显示命令（非NSO模式） */
        if (!nso_mode) {
            set_color(VGA_GREEN, VGA_BLACK);
            print("  [");
            print_int(line_number);
            print("] ");
            set_color(VGA_LIGHT_GREY, VGA_BLACK);
            print(line);
            putchar('\n');
        }
        
        /* 使用统一命令执行函数 */
        int result = cmd_execute(cmd, args, !nso_mode);
        
        if (result) {
            success_count++;
        } else {
            if (!nso_mode) {
                set_color(VGA_YELLOW, VGA_BLACK);
                print("  Command failed at line ");
                print_int(line_number);
                putchar('\n');
                set_color(VGA_LIGHT_GREY, VGA_BLACK);
            }
            error_count++;
        }
    }
    
    /* 显示执行结果 */
    if (!nso_mode) {
        print("========================================\n");
        print_color("Script execution completed.\n", VGA_CYAN, VGA_BLACK);
        print("  Lines processed: ");
        print_int(line_number);
        print("\n  Successful: ");
        print_int(success_count);
        print("\n  Errors: ");
        print_int(error_count);
        print("\n\n");
    }
    
    return (error_count == 0) ? 1 : 0;
}