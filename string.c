// string.c - 字符串和内存操作实现
#include "string.h"

/* 内存拷贝 */
void *memcpy(void *dest, const void *src, unsigned int n) {
    char *d = (char *)dest;
    const char *s = (const char *)src;
    for (unsigned int i = 0; i < n; i++) {
        d[i] = s[i];
    }
    return dest;
}

/* 内存设置 */
void *memset(void *s, int c, unsigned int n) {
    unsigned char *p = (unsigned char *)s;
    for (unsigned int i = 0; i < n; i++) {
        p[i] = (unsigned char)c;
    }
    return s;
}

/* 字符串长度 */
int strlen(const char *str) {
    int len = 0;
    while (str[len] != '\0') {
        len++;
    }
    return len;
}

/* 字符串比较 */
int strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(unsigned char *)s1 - *(unsigned char *)s2;
}

/* 字符串连接 */
char *strcat(char *dest, const char *src) {
    char *ptr = dest;
    while (*ptr) ptr++;
    while (*src) {
        *ptr++ = *src++;
    }
    *ptr = '\0';
    return dest;
}

/* 字符串复制 */
char *strcpy(char *dest, const char *src) {
    char *ptr = dest;
    while (*src) {
        *ptr++ = *src++;
    }
    *ptr = '\0';
    return dest;
}

/* ASCII转整数 */
int atoi(const char *str) {
    int result = 0;
    int sign = 1;
    int i = 0;
    
    if (str[0] == '-') {
        sign = -1;
        i++;
    }
    
    for (; str[i] != '\0'; i++) {
        if (str[i] >= '0' && str[i] <= '9') {
            result = result * 10 + (str[i] - '0');
        } else {
            break;
        }
    }
    
    return sign * result;
}

/* 整数转ASCII */
char *itoa(int value, char *str, int base) {
    char *ptr = str, *ptr1 = str, tmp_char;
    int tmp_value;
    
    if (base < 2 || base > 36) {
        *str = '\0';
        return str;
    }
    
    if (value == 0) {
        *ptr++ = '0';
        *ptr = '\0';
        return str;
    }
    
    if (value < 0 && base == 10) {
        *ptr++ = '-';
        value = -value;
    }
    
    tmp_value = value;
    while (tmp_value) {
        int remainder = tmp_value % base;
        if (remainder < 10) {
            *ptr++ = '0' + remainder;
        } else {
            *ptr++ = 'a' + (remainder - 10);
        }
        tmp_value /= base;
    }
    
    *ptr-- = '\0';
    
    while (ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr-- = *ptr1;
        *ptr1++ = tmp_char;
    }
    
    return str;
}