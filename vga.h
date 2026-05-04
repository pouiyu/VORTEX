// vga.h - VGA显示驱动头文件
#ifndef VGA_H
#define VGA_H

#include "types.h"

/* 全局状态变量 */
extern int cursor_x;
extern int cursor_y;
extern unsigned char current_color;

/* 屏幕缓冲区大小 */
#define SCREEN_BUFFER_ROWS 1000  // 可保存1000行历史

/* VGA函数声明 */
void update_cursor(int x, int y);
void enable_cursor(unsigned char start, unsigned char end);
void clear_screen(void);
void set_color(unsigned char foreground, unsigned char background);
void scroll_screen(void);
void putchar(char c);
void print(const char *str);
void print_color(const char *str, unsigned char foreground, unsigned char background);
void print_int(int num);
void print_two_digits(unsigned char num);
void print_four_digits(unsigned int num);

/* 滚动查看函数 */
void scroll_view_up(void);
void scroll_view_down(void);
int is_scrolled(void);

#endif