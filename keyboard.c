// keyboard.c - 键盘输入处理实现
#include "types.h"
#include "io.h"
#include "vga.h"
#include "keyboard.h"
#include "string.h"   // 提供 strcmp, strcpy, strlen 等函数

// 添加 NULL 定义
#ifndef NULL
#define NULL ((void *)0)
#endif

/* 键盘状态变量定义 */
int shift_pressed = 0;
int caps_lock = 0;

/* 检查键盘是否有输入 */
int keyboard_has_data(void) {
    return inb(KEYBOARD_STATUS_PORT) & 0x01;
}

/**
 * 获取可打印字符（忽略特殊键）
 */
char getchar(void) {
    int key;
    while (1) {
        key = read_key();
        // 只返回可打印字符
        if ((key >= 0x20 && key < 0x7F) || key == '\n' || key == '\b' || key == '\t') {
            return (char)key;
        }
        // 忽略方向键等特殊键
    }
}

/**
 * 读取键盘输入（含方向键检测）
 * 返回：普通键返回ASCII码，方向键返回 KEY_UP/DOWN/LEFT/RIGHT
 */
int read_key(void) {
    unsigned char scan_code;
    unsigned char ascii_char = 0;
    int extended = 0;
    
    while (1) {
        /* 不断轮询键盘状态 */
        while (!keyboard_has_data()) {
            io_delay();
        }
        
        /* 读取扫描码 */
        scan_code = inb(KEYBOARD_DATA_PORT);
        
        /* 检查扩展键前缀 (0xE0) */
        if (scan_code == 0xE0) {
            extended = 1;
            continue;
        }
        
        /* 检查是否是按键释放事件 */
        if (scan_code & 0x80) {
            unsigned char released_key = scan_code & 0x7F;
            
            if (released_key == KEY_LSHIFT || released_key == KEY_RSHIFT) {
                shift_pressed = 0;
            }
            // 重置扩展标志（方向键释放也会发送0xE0前缀）
            extended = 0;
            continue;
        }
        
        /* 处理扩展键（方向键等） */
        if (extended) {
            extended = 0;  // 立即重置
            switch (scan_code) {
                case 0x48: return KEY_UP;      // 上箭头
                case 0x50: return KEY_DOWN;    // 下箭头
                case 0x4B: return KEY_LEFT;    // 左箭头
                case 0x4D: return KEY_RIGHT;   // 右箭头
                default: 
                    // 未知扩展键，继续等待
                    continue;
            }
        }
        
        /* 按键按下事件处理 */
        if (scan_code == KEY_LSHIFT || scan_code == KEY_RSHIFT) {
            shift_pressed = 1;
            continue;
        }
        
        if (scan_code == KEY_CAPSLOCK) {
            caps_lock = !caps_lock;
            continue;
        }
        
        if (scan_code == KEY_LCTRL || scan_code == KEY_LALT) {
            continue;
        }
        
        /* 转换扫描码到ASCII */
        if (scan_code < sizeof(scancode_to_ascii_lower)) {
            int use_upper = shift_pressed ^ caps_lock;
            
            if (use_upper) {
                ascii_char = scancode_to_ascii_upper[scan_code];
            } else {
                ascii_char = scancode_to_ascii_lower[scan_code];
            }
            
            if (ascii_char != 0) {
                return ascii_char;
            }
        }
    }
}

/**
 * 读取一行输入（普通版本，不处理方向键）
 */
void read_line(char *buffer, int max_length) {
    int i = 0;
    char c;
    
    while (i < max_length - 1) {
        c = getchar();
        
        if (c == '\n') {
            putchar('\n');
            break;
        } else if (c == '\b') {
            if (i > 0) {
                i--;
                putchar('\b');
            }
        } else if (c == '\t') {
            if (i < max_length - 2) {
                buffer[i] = '\t';
                i++;
                putchar('\t');
            }
        } else if (c >= ' ') {
            buffer[i] = c;
            i++;
            putchar(c);
        }
    }
    
    buffer[i] = '\0';
}

/**
 * 读取一行输入（支持方向键滚动查看历史输出）
 */
void read_line_with_scroll(char *buffer, int max_length) {
    int i = 0;
    int key;
    
    while (i < max_length - 1) {
        key = read_key();
        
        if (key == KEY_UP) {
            scroll_view_up();
            continue;
        } else if (key == KEY_DOWN) {
            scroll_view_down();
            continue;
        } else if (key == '\n') {
            putchar('\n');
            break;
        } else if (key == '\b') {
            if (i > 0) {
                i--;
                putchar('\b');
            }
        } else if (key == '\t') {
            if (i < max_length - 2) {
                buffer[i] = '\t';
                i++;
                putchar('\t');
            }
        } else if (key >= ' ' && key < 0x7F) {
            buffer[i] = (char)key;
            i++;
            putchar((char)key);
        }
    }
    
    buffer[i] = '\0';
}

/* 历史命令存储 */
static char history[HISTORY_MAX][256];
static int history_count = 0;     // 当前存储的历史命令数量
static int history_current = 0;   // 当前浏览到的历史命令索引

/* 添加命令到历史记录 */
void add_to_history(const char *command) {
    // 忽略空命令
    if (command == NULL || command[0] == '\0') {
        return;
    }
    
    // 如果与上一条命令相同，不重复添加
    if (history_count > 0 && strcmp(history[history_count - 1], command) == 0) {
        return;
    }
    
    // 如果历史已满，移除最旧的命令（将所有命令前移）
    if (history_count >= HISTORY_MAX) {
        for (int i = 0; i < HISTORY_MAX - 1; i++) {
            strcpy(history[i], history[i + 1]);
        }
        history_count = HISTORY_MAX - 1;
    }
    
    // 添加新命令
    strcpy(history[history_count], command);
    history_count++;
}

/* 清除当前行（用于切换历史命令时刷新显示）*/
static void clear_current_line(int length) {
    // 将光标移到行首
    for (int i = length; i > 0; i--) {
        putchar('\b');
    }
    // 用空格覆盖
    for (int i = 0; i < length; i++) {
        putchar(' ');
    }
    // 再次移回行首
    for (int i = 0; i < length; i++) {
        putchar('\b');
    }
}

/**
 * 读取一行输入（支持方向键切换历史命令）
 * 左右键：浏览历史命令
 * 上下键：滚动屏幕查看历史输出
 */
void read_line_with_history(char *buffer, int max_length) {
    int i = 0;
    int key;
    
    // 重置历史浏览位置到最新
    history_current = history_count;
    
    while (i < max_length - 1) {
        key = read_key();
        
        if (key == KEY_LEFT) {
            // 左键：浏览上一条历史命令
            if (history_current > 0) {
                // 保存当前正在输入的命令（如果是浏览历史命令）
                if (history_current == history_count) {
                    buffer[i] = '\0';
                    // 临时存储当前输入
                    static char temp_input[256];
                    strcpy(temp_input, buffer);
                }
                
                history_current--;
                
                // 清除当前行
                clear_current_line(i);
                
                // 复制历史命令到 buffer 并显示
                strcpy(buffer, history[history_current]);
                i = strlen(buffer);
                
                // 显示历史命令
                print(buffer);
            }
        } else if (key == KEY_RIGHT) {
            // 右键：浏览下一条历史命令
            if (history_current < history_count) {
                history_current++;
                
                // 清除当前行
                clear_current_line(i);
                
                if (history_current < history_count) {
                    // 显示下一条历史命令
                    strcpy(buffer, history[history_current]);
                    i = strlen(buffer);
                    print(buffer);
                } else {
                    // 到达最新位置，恢复之前输入的内容或清空
                    buffer[0] = '\0';
                    i = 0;
                }
            }
        } else if (key == KEY_UP) {
            // 上键：滚动查看屏幕历史
            scroll_view_up();
        } else if (key == KEY_DOWN) {
            // 下键：滚动查看屏幕历史
            scroll_view_down();
        } else if (key == '\n') {
            // 回车：确认输入
            buffer[i] = '\0';
            putchar('\n');
            
            // 将命令添加到历史记录
            add_to_history(buffer);
            
            // 重置历史浏览位置
            history_current = history_count;
            break;
        } else if (key == '\b') {
            // 退格键
            if (i > 0) {
                i--;
                putchar('\b');
                // 更新缓冲区
                buffer[i] = '\0';
                // 重置历史浏览位置
                history_current = history_count;
            }
        } else if (key == '\t') {
            // Tab 键
            if (i < max_length - 2) {
                buffer[i] = '\t';
                i++;
                putchar('\t');
                history_current = history_count;
            }
        } else if (key >= ' ' && key < 0x7F) {
            // 可打印字符
            buffer[i] = (char)key;
            i++;
            putchar((char)key);
            // 当用户输入新字符时，重置历史浏览位置
            history_current = history_count;
        }
    }
    
    buffer[i] = '\0';
}