// fat32.h - FAT32文件系统头文件
#ifndef FAT32_H
#define FAT32_H

#include "types.h"

/* FAT32常量 */
#define FAT32_SIGNATURE         0xAA55
#define FAT32_FS_SIGNATURE      "FAT32   "
#define FAT32_ENTRY_FREE        0xE5
#define FAT32_ENTRY_END         0x00
#define FAT32_CLUSTER_FREE      0x00000000
#define FAT32_CLUSTER_END       0x0FFFFFF8
#define FAT32_CLUSTER_BAD       0x0FFFFFF7

/* VortexOS格式化标记 */
#define VORTEXOS_MAGIC_SECTOR   1       /* 使用第1扇区存储标记 */
#define VORTEXOS_MAGIC_OFFSET   0       /* 偏移量 */
#define VORTEXOS_MAGIC_VALUE    0x564F5358  /* "VOSX" 的十六进制 */

/* FAT32 BPB结构 */
typedef struct __attribute__((packed)) {
    unsigned char  jmp_boot[3];
    unsigned char  oem_name[8];
    unsigned short bytes_per_sector;
    unsigned char  sectors_per_cluster;
    unsigned short reserved_sectors;
    unsigned char  num_fats;
    unsigned short root_entries;
    unsigned short total_sectors_16;
    unsigned char  media_type;
    unsigned short sectors_per_fat_16;
    unsigned short sectors_per_track;
    unsigned short num_heads;
    unsigned int   hidden_sectors;
    unsigned int   total_sectors_32;
    unsigned int   sectors_per_fat;
    unsigned short flags;
    unsigned short version;
    unsigned int   root_cluster;
    unsigned short fs_info_sector;
    unsigned short backup_boot_sector;
    unsigned char  reserved[12];
    unsigned char  drive_number;
    unsigned char  reserved1;
    unsigned char  boot_signature;
    unsigned int   volume_id;
    unsigned char  volume_label[11];
    unsigned char  fs_type[8];
} fat32_bpb_t;

/* 目录项 */
typedef struct __attribute__((packed)) {
    unsigned char  name[11];
    unsigned char  attributes;
    unsigned char  reserved;
    unsigned char  creation_time_tenth;
    unsigned short creation_time;
    unsigned short creation_date;
    unsigned short access_date;
    unsigned short first_cluster_high;
    unsigned short write_time;
    unsigned short write_date;
    unsigned short first_cluster_low;
    unsigned int   file_size;
} fat32_dir_entry_t;

/* 长文件名项 */
typedef struct __attribute__((packed)) {
    unsigned char  order;
    unsigned short name1[5];
    unsigned char  attributes;
    unsigned char  type;
    unsigned char  checksum;
    unsigned short name2[6];
    unsigned short zero;
    unsigned short name3[2];
} fat32_lfn_entry_t;

/* 属性定义 */
#define ATTR_READ_ONLY  0x01
#define ATTR_HIDDEN     0x02
#define ATTR_SYSTEM     0x04
#define ATTR_VOLUME_ID  0x08
#define ATTR_DIRECTORY  0x10
#define ATTR_ARCHIVE    0x20
#define ATTR_LFN        0x0F

/* 文件系统函数 */
int fat32_init(void);
int fat32_format(void);
void set_vortexos_magic(void);  /* 添加声明 */
int fat32_is_formatted(void);  /* 新增：检查是否已格式化 */
int fat32_create_file(const char *path);
int fat32_create_directory(const char *path);
int fat32_delete(const char *path);
int fat32_list_directory(const char *path);
int fat32_write_file(const char *path, const void *data, unsigned int size);
int fat32_read_file(const char *path, void *buffer, unsigned int size);
int fat32_file_exists(const char *path);
unsigned int fat32_get_file_size(const char *path);

#endif