// rtc.h - RTC时钟头文件
#ifndef RTC_H
#define RTC_H

/* RTC函数 */
unsigned char read_cmos(unsigned char reg);
unsigned char bcd_to_bin(unsigned char bcd);
int rtc_is_updating(void);
void read_rtc_datetime(unsigned char *hour, unsigned char *minute, 
                       unsigned char *second, unsigned int *year, 
                       unsigned char *month, unsigned char *day);

#endif