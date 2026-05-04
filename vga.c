// vga.c - VGA显示驱动实现（修复光标位置）
#include "vga.h"
#include "io.h"
#include "string.h"

int cursor_x = 0;
int cursor_y = 0;
unsigned char current_color = VGA_LIGHT_GREY;

static struct {
    unsigned short data[SCREEN_BUFFER_ROWS][VGA_WIDTH];
    int total_rows;
    int view_offset;
    int is_scrolling;
    int saved_cursor_x;    // 进入滚动前的光标X
    int saved_cursor_y;    // 进入滚动前的光标Y
} screen_buffer;

void update_cursor(int x, int y) {
    unsigned short pos = y * VGA_WIDTH + x;
    outb(VGA_CTRL_REGISTER, 0x0F);
    outb(VGA_DATA_REGISTER, (unsigned char)(pos & 0xFF));
    outb(VGA_CTRL_REGISTER, 0x0E);
    outb(VGA_DATA_REGISTER, (unsigned char)((pos >> 8) & 0xFF));
    cursor_x = x;
    cursor_y = y;
}

void enable_cursor(unsigned char start, unsigned char end) {
    outb(VGA_CTRL_REGISTER, 0x0A);
    outb(VGA_DATA_REGISTER, (inb(VGA_DATA_REGISTER) & 0xC0) | start);
    outb(VGA_CTRL_REGISTER, 0x0B);
    outb(VGA_DATA_REGISTER, (inb(VGA_DATA_REGISTER) & 0xE0) | end);
}

void disable_cursor(void) {
    outb(VGA_CTRL_REGISTER, 0x0A);
    outb(VGA_DATA_REGISTER, 0x20);
}

void clear_screen() {
    unsigned short *vm = (unsigned short *)VIDEO_MEMORY;
    unsigned short blank = (VGA_BLACK << 8) | ' ';
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) vm[i] = blank;
    memset(&screen_buffer, 0, sizeof(screen_buffer));
    update_cursor(0, 0);
}

void set_color(unsigned char fg, unsigned char bg) {
    current_color = (bg << 4) | (fg & 0x0F);
}

// vga.c - 修改 save_visible_screen_to_buffer() 函数

/* 将当前物理屏幕所有非空行保存到缓冲区 */
static void save_visible_screen_to_buffer(void) {
    unsigned short *vm = (unsigned short *)VIDEO_MEMORY;
    
    // 找到最后一个非空行
    int last_row = VGA_HEIGHT - 1;
    while (last_row >= 0) {
        int empty = 1;
        for (int col = 0; col < VGA_WIDTH; col++) {
            if ((vm[last_row * VGA_WIDTH + col] & 0xFF) != ' ') {
                empty = 0;
                break;
            }
        }
        if (!empty) break;
        last_row--;
    }
    
    // 如果全空，至少保存一行
    if (last_row < 0) last_row = 0;
    
    // 检查是否已经保存过（避免重复保存）
    // 如果缓冲区已经有内容，检查最后一行是否与当前屏幕最后一行相同
    if (screen_buffer.total_rows > 0) {
        int match = 1;
        for (int col = 0; col < VGA_WIDTH; col++) {
            if (screen_buffer.data[screen_buffer.total_rows - 1][col] != vm[last_row * VGA_WIDTH + col]) {
                match = 0;
                break;
            }
        }
        if (match) {
            // 屏幕内容没有变化，不需要重新保存
            return;
        }
    }
    
    // 保存第0行到last_row行
    for (int row = 0; row <= last_row; row++) {
        if (screen_buffer.total_rows < SCREEN_BUFFER_ROWS) {
            for (int col = 0; col < VGA_WIDTH; col++)
                screen_buffer.data[screen_buffer.total_rows][col] = vm[row * VGA_WIDTH + col];
            screen_buffer.total_rows++;
        }
    }
}

/* 物理屏幕滚动时保存第0行 */
static void save_row0(void) {
    unsigned short *vm = (unsigned short *)VIDEO_MEMORY;
    if (screen_buffer.total_rows < SCREEN_BUFFER_ROWS) {
        for (int i = 0; i < VGA_WIDTH; i++)
            screen_buffer.data[screen_buffer.total_rows][i] = vm[i];
        screen_buffer.total_rows++;
    }
}

/* 渲染滚动视图 */
static void render_scroll_view(void) {
    unsigned short *vm = (unsigned short *)VIDEO_MEMORY;
    unsigned short blank = (VGA_BLACK << 8) | ' ';
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) vm[i] = blank;
    
    int start = screen_buffer.view_offset;
    int end = start + VGA_HEIGHT;
    if (end > screen_buffer.total_rows) end = screen_buffer.total_rows;
    
    for (int row = start; row < end; row++)
        for (int col = 0; col < VGA_WIDTH; col++)
            vm[(row - start) * VGA_WIDTH + col] = screen_buffer.data[row][col];
    
    disable_cursor();
}

/* 从缓冲区恢复物理屏幕（退出滚动时） */
static void restore_screen_from_buffer(void) {
    unsigned short *vm = (unsigned short *)VIDEO_MEMORY;
    unsigned short blank = (VGA_BLACK << 8) | ' ';
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) vm[i] = blank;
    
    int start = screen_buffer.total_rows - VGA_HEIGHT;
    if (start < 0) start = 0;
    
    int rows = screen_buffer.total_rows - start;
    for (int row = 0; row < rows; row++)
        for (int col = 0; col < VGA_WIDTH; col++)
            vm[row * VGA_WIDTH + col] = screen_buffer.data[start + row][col];
    
    enable_cursor(13, 14);
    // 恢复保存的光标位置
    update_cursor(screen_buffer.saved_cursor_x, screen_buffer.saved_cursor_y);
}

void putchar(char c) {
    unsigned short *vm = (unsigned short *)VIDEO_MEMORY;
    
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else if (c == '\t') {
        cursor_x = (cursor_x + 4) & ~3;
        if (cursor_x >= VGA_WIDTH) { cursor_x = 0; cursor_y++; }
    } else if (c == '\r') {
        cursor_x = 0;
    } else if (c == '\b') {
        if (cursor_x > 0) cursor_x--;
        else if (cursor_y > 0) { cursor_y--; cursor_x = VGA_WIDTH - 1; }
        vm[cursor_y * VGA_WIDTH + cursor_x] = (current_color << 8) | ' ';
    } else if (c >= ' ') {
        vm[cursor_y * VGA_WIDTH + cursor_x] = (current_color << 8) | c;
        cursor_x++;
    }
    
    if (cursor_x >= VGA_WIDTH) { cursor_x = 0; cursor_y++; }
    
    if (cursor_y >= VGA_HEIGHT) {
        if (!screen_buffer.is_scrolling) save_row0();
        for (int i = 0; i < (VGA_HEIGHT - 1) * VGA_WIDTH; i++)
            vm[i] = vm[i + VGA_WIDTH];
        unsigned short blank = (VGA_BLACK << 8) | ' ';
        for (int i = (VGA_HEIGHT - 1) * VGA_WIDTH; i < VGA_HEIGHT * VGA_WIDTH; i++)
            vm[i] = blank;
        cursor_y = VGA_HEIGHT - 1;
    }
    
    update_cursor(cursor_x, cursor_y);
    
    if (!screen_buffer.is_scrolling) {
        screen_buffer.view_offset = screen_buffer.total_rows - VGA_HEIGHT;
        if (screen_buffer.view_offset < 0) screen_buffer.view_offset = 0;
    }
}

void print(const char *str) { for (int i = 0; str[i]; i++) putchar(str[i]); }

void print_color(const char *str, unsigned char fg, unsigned char bg) {
    unsigned char old = current_color; set_color(fg, bg); print(str); current_color = old;
}

void print_int(int num) {
    char str[33]; extern char *itoa(int, char*, int);
    print(itoa(num, str, 10));
}

void print_two_digits(unsigned char num) {
    if (num < 10) {
        putchar('0');
    }
    print_int(num);
}
void print_four_digits(unsigned int num) {
    if (num < 1000) { if (num < 100) { if (num < 10) putchar('0'); putchar('0'); } putchar('0'); }
    print_int(num);
}

// vga.c - 修改 scroll_view_up() 函数

void scroll_view_up(void) {
    if (!screen_buffer.is_scrolling) {
        // 保存当前光标位置
        screen_buffer.saved_cursor_x = cursor_x;
        screen_buffer.saved_cursor_y = cursor_y;
        // 保存当前可见屏幕到缓冲区（只在第一次进入滚动时保存）
        save_visible_screen_to_buffer();
        // 重新计算偏移
        screen_buffer.view_offset = screen_buffer.total_rows - VGA_HEIGHT;
        if (screen_buffer.view_offset < 0) screen_buffer.view_offset = 0;
        screen_buffer.is_scrolling = 1;  // 立即设置滚动状态，防止重复保存
    }
    
    if (screen_buffer.total_rows > VGA_HEIGHT) {
        if (screen_buffer.view_offset > 0) {
            screen_buffer.view_offset--;
        }
        if (screen_buffer.view_offset < 0) screen_buffer.view_offset = 0;
        render_scroll_view();
    }
}

void scroll_view_down(void) {
    int max_offset = screen_buffer.total_rows - VGA_HEIGHT;
    if (max_offset < 0) max_offset = 0;
    
    if (screen_buffer.view_offset < max_offset) {
        screen_buffer.view_offset++;
        if (screen_buffer.view_offset > max_offset) screen_buffer.view_offset = max_offset;
        render_scroll_view();
    }
    
    if (screen_buffer.view_offset >= max_offset) {
        screen_buffer.is_scrolling = 0;
        restore_screen_from_buffer();
    }
}

int is_scrolled(void) { return screen_buffer.is_scrolling; }