// shell.h - Shell命令处理头文件 (更新版)
#ifndef SHELL_H
#define SHELL_H

void shell(void);
void show_logo(void);
void show_system_info(void);
void calculator(void);
void add_to_history(const char *command);
void read_line_with_history(char *buffer, int max_length);
int brc_execute(const char *filename);

#endif