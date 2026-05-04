// pit.c - PIT定时器实现
#include "types.h"
#include "io.h"
#include "pit.h"

/* 读取PIT计数器（16位）*/
unsigned short read_pit_count() {
    unsigned short count;
    
    outb(PIT_COMMAND, 0x00);  /* 通道0，锁存计数 */
    
    count = inb(PIT_CHANNEL0);
    count |= (inb(PIT_CHANNEL0) << 8);
    
    return count;
}