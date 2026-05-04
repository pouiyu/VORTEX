// string.h - 字符串和内存操作头文件
#ifndef STRING_H
#define STRING_H

/* 字符串和内存函数 */
void *memcpy(void *dest, const void *src, unsigned int n);
void *memset(void *s, int c, unsigned int n);
int strlen(const char *str);
int strcmp(const char *s1, const char *s2);
char *strcat(char *dest, const char *src);
char *strcpy(char *dest, const char *src);
int atoi(const char *str);
char *itoa(int value, char *str, int base);

#endif