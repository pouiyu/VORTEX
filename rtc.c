// rtc.c - RTC时钟实现
#include "types.h"
#include "io.h"
#include "rtc.h"

/* 从CMOS读取数据 */
unsigned char read_cmos(unsigned char reg) {
    outb(RTC_INDEX_PORT, reg);
    io_delay();
    return inb(RTC_DATA_PORT);
}

/* BCD转二进制 */
unsigned char bcd_to_bin(unsigned char bcd) {
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

/* 检查RTC是否在更新 */
int rtc_is_updating() {
    outb(RTC_INDEX_PORT, 0x0A);
    return (inb(RTC_DATA_PORT) & 0x80);
}

/* 读取RTC日期时间 */
void read_rtc_datetime(unsigned char *hour, unsigned char *minute, 
                       unsigned char *second, unsigned int *year, 
                       unsigned char *month, unsigned char *day) {
    unsigned char register_b;
    unsigned char cmos_year, cmos_century;
    
    while (rtc_is_updating());
    
    *second = read_cmos(RTC_SECONDS);
    *minute = read_cmos(RTC_MINUTES);
    *hour = read_cmos(RTC_HOURS);
    *day = read_cmos(RTC_DAY_OF_MONTH);
    *month = read_cmos(RTC_MONTH);
    cmos_year = read_cmos(RTC_YEAR);
    cmos_century = read_cmos(RTC_CENTURY);
    
    register_b = read_cmos(0x0B);
    if (!(register_b & 0x04)) {
        *second = bcd_to_bin(*second);
        *minute = bcd_to_bin(*minute);
        *hour = bcd_to_bin(*hour);
        *day = bcd_to_bin(*day);
        *month = bcd_to_bin(*month);
        cmos_year = bcd_to_bin(cmos_year);
    }
    
    if (cmos_century > 0x00 && cmos_century != 0xFF) {
        if (!(register_b & 0x04)) {
            cmos_century = bcd_to_bin(cmos_century);
        }
        *year = cmos_century * 100 + cmos_year;
    } else {
        *year = 2000 + cmos_year;
    }
}