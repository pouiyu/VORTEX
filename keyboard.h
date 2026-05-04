// keyboard.h - 键盘输入处理头文件
#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "types.h"

/* 扩展键定义 */
#define KEY_UP     0x01F0
#define KEY_DOWN   0x01F1
#define KEY_LEFT   0x01F2
#define KEY_RIGHT  0x01F3

/* 键盘扫描码到ASCII转换表（保持不变）*/
static const char scancode_to_ascii_lower[] = {
    0,   0,   '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t','q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0,   'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';','\'', '`', 
    0,   '\\','z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0,   ' '
};

static const char scancode_to_ascii_upper[] = {
    0,   0,   '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t','Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0,   'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
    0,   '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,
    '*', 0,   ' '
};

/* 键盘状态变量 */
extern int shift_pressed;
extern int caps_lock;

/* 键盘函数 */
char getchar(void);
int read_key(void);
void read_line(char *buffer, int max_length);
void read_line_with_scroll(char *buffer, int max_length);
int keyboard_has_data(void);

/* 历史命令的最大数量 */
#define HISTORY_MAX 50

/* 历史命令的输入函数 */
void read_line_with_history(char *buffer, int max_length);

#endif