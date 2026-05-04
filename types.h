// types.h - 通用类型定义和宏
#ifndef TYPES_H
#define TYPES_H

/* VGA常量 */
#define VIDEO_MEMORY 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

/* VGA颜色定义 */
#define VGA_BLACK         0x0
#define VGA_BLUE          0x1
#define VGA_GREEN         0x2
#define VGA_CYAN          0x3
#define VGA_RED           0x4
#define VGA_MAGENTA       0x5
#define VGA_BROWN         0x6
#define VGA_LIGHT_GREY    0x7
#define VGA_DARK_GREY     0x8
#define VGA_LIGHT_BLUE    0x9
#define VGA_LIGHT_GREEN   0xA
#define VGA_LIGHT_CYAN    0xB
#define VGA_LIGHT_RED     0xC
#define VGA_LIGHT_MAGENTA 0xD
#define VGA_YELLOW        0xE
#define VGA_WHITE         0xF

/* 光标端口 */
#define VGA_CTRL_REGISTER 0x3D4
#define VGA_DATA_REGISTER 0x3D5

/* 键盘端口 */
#define KEYBOARD_DATA_PORT    0x60
#define KEYBOARD_STATUS_PORT  0x64

/* 键盘扫描码定义 */
#define KEY_ESC      0x01
#define KEY_BACKSPACE 0x0E
#define KEY_TAB      0x0F
#define KEY_ENTER    0x1C
#define KEY_LCTRL    0x1D
#define KEY_LSHIFT   0x2A
#define KEY_RSHIFT   0x36
#define KEY_LALT     0x38
#define KEY_CAPSLOCK 0x3A
#define KEY_F1       0x3B
#define KEY_F12      0x58

/* RTC端口 */
#define RTC_INDEX_PORT     0x70
#define RTC_DATA_PORT      0x71

/* RTC寄存器 */
#define RTC_SECONDS        0x00
#define RTC_MINUTES        0x02
#define RTC_HOURS          0x04
#define RTC_WEEKDAY        0x06
#define RTC_DAY_OF_MONTH   0x07
#define RTC_MONTH          0x08
#define RTC_YEAR           0x09
#define RTC_CENTURY        0x32

/* PIT端口 */
#define PIT_CHANNEL0 0x40
#define PIT_COMMAND  0x43

#endif