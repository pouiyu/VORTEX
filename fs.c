// fs.c - 文件系统命令实现
#include "fs.h"
#include "fat32.h"
#include "vga.h"
#include "string.h"

/* mk - 创建文件 */
int fs_cmd_mkfile(int argc, char *argv[]) {
    if (argc < 1) {
        print("Usage: mk <filename>\n");
        return 0;
    }
    
    return fat32_create_file(argv[0]);
}

/* md - 创建目录 */
int fs_cmd_mkdir(int argc, char *argv[]) {
    if (argc < 1) {
        print("Usage: md <dirname>\n");
        return 0;
    }
    
    return fat32_create_directory(argv[0]);
}

/* ls - 列出目录 */
int fs_cmd_ls(int argc, char *argv[]) {
    const char *path = argc > 0 ? argv[0] : "/";
    return fat32_list_directory(path);
}

/* rm - 删除文件 */
int fs_cmd_rm(int argc, char *argv[]) {
    if (argc < 1) {
        print("Usage: rm <filename>\n");
        return 0;
    }
    
    return fat32_delete(argv[0]);
}

/* cat - 显示文件内容 */
int fs_cmd_cat(int argc, char *argv[]) {
    if (argc < 1) {
        print("Usage: cat <filename>\n");
        return 0;
    }
    
    unsigned char buffer[512];
    int bytes_read = fat32_read_file(argv[0], buffer, 512);
    
    if (bytes_read > 0) {
        for (int i = 0; i < bytes_read; i++) {
            putchar(buffer[i]);
        }
        putchar('\n');
        return 1;
    }
    
    print("ERROR: Cannot read file!\n");
    return 0;
}

/* wr - 写入文件 */
int fs_cmd_write(int argc, char *argv[]) {
    if (argc < 2) {
        print("Usage: wr <filename> <content>\n");
        return 0;
    }
    
    /* 重新组合内容（可能包含空格）*/
    char content[512] = "";
    for (int i = 1; i < argc; i++) {
        if (i > 1) {
            strcat(content, " ");
        }
        strcat(content, argv[i]);
    }
    
    if (!fat32_file_exists(argv[0])) {
        fat32_create_file(argv[0]);
    }
    
    int len = strlen(content);
    return fat32_write_file(argv[0], content, len);
}

/* df - 显示磁盘信息 */
int fs_cmd_df(int argc, char *argv[]) {
    (void)argc;  /* 避免未使用参数警告 */
    (void)argv;  /* 避免未使用参数警告 */
    
    print("\nDisk Partition: hd0\n");
    print("Filesystem: FAT32\n");
    print("Total space: ");
    print_int(512);  /* 简化显示 */
    print(" MB\n");
    print("Available: ");
    print_int(400);  /* 简化显示 */
    print(" MB\n\n");
    
    return 1;
}