// fat32.c - FAT32文件系统实现
#include "fat32.h"
#include "ata.h"
#include "string.h"
#include "vga.h"

// 添加 NULL 定义
#ifndef NULL
#define NULL ((void *)0)
#endif

/* 全局文件系统信息 */
static struct {
    unsigned int first_data_sector;
    unsigned int first_fat_sector;
    unsigned int root_cluster;
    unsigned int sectors_per_cluster;
    unsigned int bytes_per_sector;
    unsigned int total_sectors;
} fs_info;

static unsigned char sector_buffer[512];

/* 从簇号计算扇区号 */
static unsigned int cluster_to_sector(unsigned int cluster) {
    return fs_info.first_data_sector + 
           (cluster - 2) * fs_info.sectors_per_cluster;
}

/* 读取FAT表项 */
static unsigned int fat32_get_next_cluster(unsigned int cluster) {
    unsigned int fat_offset = cluster * 4;
    unsigned int fat_sector = fs_info.first_fat_sector + 
                              (fat_offset / fs_info.bytes_per_sector);
    unsigned int ent_offset = fat_offset % fs_info.bytes_per_sector;
    
    if (!ata_read_sectors(fat_sector, 1, sector_buffer)) {
        return FAT32_CLUSTER_END;
    }
    
    unsigned int value;
    memcpy(&value, &sector_buffer[ent_offset], sizeof(unsigned int));
    return value & 0x0FFFFFFF;
}

/* 设置FAT表项 */
static int fat32_set_next_cluster(unsigned int cluster, unsigned int value) {
    unsigned int fat_offset = cluster * 4;
    unsigned int fat_sector = fs_info.first_fat_sector + 
                              (fat_offset / fs_info.bytes_per_sector);
    unsigned int ent_offset = fat_offset % fs_info.bytes_per_sector;
    
    if (!ata_read_sectors(fat_sector, 1, sector_buffer)) {
        return 0;
    }
    
    unsigned int *entry = (unsigned int *)&sector_buffer[ent_offset];
    *entry = (*entry & 0xF0000000) | (value & 0x0FFFFFFF);
    
    return ata_write_sectors(fat_sector, 1, sector_buffer);
}

/* 分配空闲簇 */
static unsigned int fat32_alloc_cluster(void) {
    unsigned int max_cluster = fs_info.total_sectors / fs_info.sectors_per_cluster;
    
    for (unsigned int cluster = 2; cluster < max_cluster && cluster < 65536; cluster++) {
        unsigned int next = fat32_get_next_cluster(cluster);
        if (next == FAT32_CLUSTER_FREE) {
            fat32_set_next_cluster(cluster, FAT32_CLUSTER_END);
            return cluster;
        }
    }
    return 0;
}

/* 在目录中创建目录项 */
static int create_dir_entry(const char *name, unsigned char attr, 
                            unsigned int first_cluster, unsigned int size,
                            unsigned int parent_cluster) {
    unsigned char dir_sector[512];
    unsigned int sector = cluster_to_sector(parent_cluster);
    
    if (!ata_read_sectors(sector, 1, dir_sector)) {
        return 0;
    }
    
    fat32_dir_entry_t *entry = (fat32_dir_entry_t *)dir_sector;
    
    for (int i = 0; i < 16; i++) {
        if (entry[i].name[0] == FAT32_ENTRY_FREE || 
            entry[i].name[0] == FAT32_ENTRY_END) {
            
            memset(entry[i].name, ' ', 11);
            int j;
            for (j = 0; j < 8 && name[j] != '.' && name[j] != '\0'; j++) {
                entry[i].name[j] = name[j];
            }
            
            const char *ext = name;
            while (*ext && *ext != '.') ext++;
            if (*ext == '.') {
                ext++;
                for (j = 0; j < 3 && ext[j] != '\0'; j++) {
                    entry[i].name[8 + j] = ext[j];
                }
            }
            
            entry[i].attributes = attr;
            entry[i].first_cluster_low = first_cluster & 0xFFFF;
            entry[i].first_cluster_high = (first_cluster >> 16) & 0xFFFF;
            entry[i].file_size = size;
            
            return ata_write_sectors(sector, 1, dir_sector);
        }
    }
    
    return 0;
}

// 在 fat32.c 中添加以下代码

/* 检查磁盘是否已被VortexOS格式化 */
int fat32_is_formatted(void) {
    unsigned char buffer[512];
    
    /* 读取标记扇区 */
    if (!ata_read_sectors(VORTEXOS_MAGIC_SECTOR, 1, buffer)) {
        return 0;
    }
    
    /* 检查VortexOS魔数 */
    unsigned int magic;
    memcpy(&magic, &buffer[VORTEXOS_MAGIC_OFFSET], sizeof(unsigned int));
    
    return (magic == VORTEXOS_MAGIC_VALUE) ? 1 : 0;
}

/* 设置VortexOS格式化标记 */
void set_vortexos_magic(void) {   /* 移除 static */
    unsigned char buffer[512];
    
    /* 读取标记扇区 */
    if (!ata_read_sectors(VORTEXOS_MAGIC_SECTOR, 1, buffer)) {
        return;
    }
    
    /* 写入VortexOS魔数 */
    unsigned int magic = VORTEXOS_MAGIC_VALUE;
    memcpy(&buffer[VORTEXOS_MAGIC_OFFSET], &magic, sizeof(unsigned int));
    
    /* 写回磁盘 */
    ata_write_sectors(VORTEXOS_MAGIC_SECTOR, 1, buffer);
}

/* 清除VortexOS格式化标记（用于用户手动格式化时）*/

static void __attribute__((unused)) clear_vortexos_magic(void) {
    unsigned char buffer[512];
    
    /* 读取标记扇区 */
    if (!ata_read_sectors(VORTEXOS_MAGIC_SECTOR, 1, buffer)) {
        return;
    }
    
    /* 清除魔数 */
    unsigned int magic = 0;
    memcpy(&buffer[VORTEXOS_MAGIC_OFFSET], &magic, sizeof(unsigned int));
    
    /* 写回磁盘 */
    ata_write_sectors(VORTEXOS_MAGIC_SECTOR, 1, buffer);
}

/* FAT32初始化 */
int fat32_init(void) {
    print("Initializing FAT32 filesystem...\n");
    
    if (!ata_read_sectors(0, 1, sector_buffer)) {
        print("ERROR: Cannot read boot sector!\n");
        return 0;
    }
    
    fat32_bpb_t *bpb = (fat32_bpb_t *)sector_buffer;
    
    if (*(unsigned short *)&sector_buffer[510] != FAT32_SIGNATURE) {
        print("ERROR: Invalid boot sector signature!\n");
        return 0;
    }
    
    unsigned int reserved_sectors = bpb->reserved_sectors;
    unsigned int fat_size = bpb->sectors_per_fat;
    unsigned int hidden_sectors = bpb->hidden_sectors;
    
    fs_info.bytes_per_sector = bpb->bytes_per_sector;
    fs_info.sectors_per_cluster = bpb->sectors_per_cluster;
    fs_info.first_fat_sector = reserved_sectors + hidden_sectors;
    fs_info.root_cluster = bpb->root_cluster;
    fs_info.total_sectors = bpb->total_sectors_32;
    
    fs_info.first_data_sector = fs_info.first_fat_sector + 
                                (bpb->num_fats * fat_size);
    
    print("FAT32 filesystem mounted.\n");
    print("Total sectors: ");
    print_int(fs_info.total_sectors);
    print("\n");
    
    return 1;
}

/* 格式化FAT32 */
int fat32_format(void) {
    print("Formatting disk as FAT32...\n");
    
    unsigned char buffer[512];
    memset(buffer, 0, 512);
    
    fat32_bpb_t *bpb = (fat32_bpb_t *)buffer;
    
    // 添加简单的引导代码，显示消息并停止
    // 这段代码会在尝试从硬盘引导时显示错误并停止
    unsigned char boot_code[] = {
        0xEB, 0x3C, 0x90,                                          // JMP short to skip BPB
        // ... (BPB区域将在下面被覆盖)
    };
    
    // 复制引导代码
    memcpy(buffer, boot_code, sizeof(boot_code));
    
    // 设置BPB参数
    bpb->jmp_boot[0] = 0xEB;
    bpb->jmp_boot[1] = 0x58;
    bpb->jmp_boot[2] = 0x90;
    
    memcpy(bpb->oem_name, "VORTEXOS", 8);
    
    bpb->bytes_per_sector = 512;
    bpb->sectors_per_cluster = 1;
    bpb->reserved_sectors = 32;
    bpb->num_fats = 2;
    bpb->media_type = 0xF8;
    bpb->sectors_per_track = 63;
    bpb->num_heads = 16;
    bpb->total_sectors_32 = 102400; /* 50MB */
    bpb->sectors_per_fat = 200;
    bpb->root_cluster = 2;
    bpb->fs_info_sector = 1;
    bpb->backup_boot_sector = 6;
    
    memcpy(bpb->fs_type, "FAT32   ", 8);
    
    // 在偏移0x3E处写入错误消息和停机代码
    // 这会在尝试从硬盘引导时显示错误
    unsigned char *code_area = buffer + 0x3E;
    
    // 简单的x86汇编：显示错误并停止
    // mov ah, 0x0E    ; BIOS teletype function
    // mov si, message ; Point to message
    // ... (output loop)
    // cli             ; Clear interrupts
    // hlt             ; Halt CPU
    unsigned char bootstrap_code[] = {
        0xB4, 0x0E,                    // mov ah, 0x0E (BIOS print char)
        0xBE, 0x7C, 0x00,             // mov si, 0x007C (message offset from start)
        0xAC,                         // lodsb
        0x08, 0xC0,                   // or al, al
        0x74, 0x04,                   // jz done
        0xCD, 0x10,                   // int 0x10
        0xEB, 0xF7,                   // jmp print_loop
        0xFA,                         // cli
        0xF4,                         // hlt
        // Message (null-terminated)
        'N', 'o', 't', ' ', 'a', ' ', 'b', 'o', 'o', 't', 
        'a', 'b', 'l', 'e', ' ', 'd', 'i', 's', 'k', '.', 
        ' ', 'U', 's', 'e', ' ', 'C', 'D', '-', 'R', 'O', 'M', '.', 0
    };
    
    // 确保代码不会覆盖重要区域
    if (sizeof(bootstrap_code) < (510 - 0x3E)) {
        memcpy(code_area, bootstrap_code, sizeof(bootstrap_code));
    }
    
    buffer[510] = 0x55;
    buffer[511] = 0xAA;
    
    if (!ata_write_sectors(0, 1, buffer)) {
        print("ERROR: Cannot write boot sector!\n");
        return 0;
    }
    
    // 清空并写入FAT
    memset(buffer, 0, 512);
    unsigned int *fat_entry = (unsigned int *)buffer;
    fat_entry[0] = 0x0FFFFFF8;
    fat_entry[1] = 0x0FFFFFFF;
    
    if (!ata_write_sectors(32, 1, buffer)) {
        print("ERROR: Cannot write FAT!\n");
        return 0;
    }
    
    /* 设置VortexOS格式化标记 */
    set_vortexos_magic();
    
    print("Format complete.\n");
    return 1;
}

// fat32.c - 修复 fat32_create_file 函数

/* 创建文件 */
int fat32_create_file(const char *path) {
    unsigned int cluster = fat32_alloc_cluster();
    if (cluster == 0) {
        print("ERROR: No free clusters!\n");
        return 0;
    }
    
    if (!create_dir_entry(path, ATTR_ARCHIVE, cluster, 0, 
                          fs_info.root_cluster)) {
        fat32_set_next_cluster(cluster, FAT32_CLUSTER_FREE);
        print("ERROR: Cannot create file!\n");
        return 0;
    }
    
    // 初始化文件数据扇区为空
    unsigned int file_sector = cluster_to_sector(cluster);
    unsigned char empty_sector[512];
    memset(empty_sector, 0, 512);
    ata_write_sectors(file_sector, 1, empty_sector);
    
    print("File created: ");
    print(path);
    print("\n");
    return 1;
}

/* 创建目录 */
int fat32_create_directory(const char *path) {
    unsigned int cluster = fat32_alloc_cluster();
    if (cluster == 0) {
        print("ERROR: No free clusters!\n");
        return 0;
    }
    
    if (!create_dir_entry(path, ATTR_DIRECTORY, cluster, 0, 
                          fs_info.root_cluster)) {
        fat32_set_next_cluster(cluster, FAT32_CLUSTER_FREE);
        return 0;
    }
    
    unsigned char cluster_data[512];
    memset(cluster_data, 0, 512);
    fat32_dir_entry_t *entries = (fat32_dir_entry_t *)cluster_data;
    
    memset(entries[0].name, ' ', 11);
    entries[0].name[0] = '.';
    entries[0].attributes = ATTR_DIRECTORY;
    entries[0].first_cluster_low = cluster & 0xFFFF;
    entries[0].first_cluster_high = (cluster >> 16) & 0xFFFF;
    
    memset(entries[1].name, ' ', 11);
    entries[1].name[0] = '.';
    entries[1].name[1] = '.';
    entries[1].attributes = ATTR_DIRECTORY;
    entries[1].first_cluster_low = fs_info.root_cluster & 0xFFFF;
    entries[1].first_cluster_high = (fs_info.root_cluster >> 16) & 0xFFFF;
    
    unsigned int sector = cluster_to_sector(cluster);
    ata_write_sectors(sector, 1, cluster_data);
    
    print("Directory created: ");
    print(path);
    print("\n");
    return 1;
}

/* 列出目录 */
int fat32_list_directory(const char *path) {
    (void)path;
    
    unsigned int cluster = fs_info.root_cluster;
    unsigned int sector = cluster_to_sector(cluster);
    unsigned char dir_data[512];
    
    if (!ata_read_sectors(sector, 1, dir_data)) {
        print("ERROR: Cannot read directory!\n");
        return 0;
    }
    
    fat32_dir_entry_t *entries = (fat32_dir_entry_t *)dir_data;
    
    print("\nDirectory listing:\n");
    print("--------------------------------------------------\n");
    
    int count = 0;
    for (int i = 0; i < 16; i++) {
        if (entries[i].name[0] == FAT32_ENTRY_END) break;
        if (entries[i].name[0] == FAT32_ENTRY_FREE) continue;
        if (entries[i].attributes == ATTR_LFN) continue;
        if (entries[i].attributes & ATTR_VOLUME_ID) continue;
        
        char filename[13];
        int j;
        for (j = 0; j < 8 && entries[i].name[j] != ' '; j++) {
            filename[j] = entries[i].name[j];
        }
        
        if (entries[i].name[8] != ' ') {
            filename[j++] = '.';
            for (int k = 0; k < 3 && entries[i].name[8 + k] != ' '; k++) {
                filename[j++] = entries[i].name[8 + k];
            }
        }
        filename[j] = '\0';
        
        if (entries[i].attributes & ATTR_DIRECTORY) {
            print_color("[DIR]  ", VGA_CYAN, VGA_BLACK);
        } else {
            print("       ");
        }
        
        print(filename);
        
        if (!(entries[i].attributes & ATTR_DIRECTORY)) {
            for (int s = 0; s < (15 - j); s++) {
                putchar(' ');
            }
            print_int(entries[i].file_size);
            print(" bytes");
        }
        
        putchar('\n');
        count++;
    }
    
    print("--------------------------------------------------\n");
    print_int(count);
    print(" items found.\n\n");
    
    return count;
}

/* 删除文件/目录 */
int fat32_delete(const char *path) {
    unsigned int cluster = fs_info.root_cluster;
    unsigned int sector = cluster_to_sector(cluster);
    unsigned char dir_data[512];
    
    if (!ata_read_sectors(sector, 1, dir_data)) {
        print("ERROR: Cannot read directory!\n");
        return 0;
    }
    
    fat32_dir_entry_t *entries = (fat32_dir_entry_t *)dir_data;
    
    for (int i = 0; i < 16; i++) {
        if (entries[i].name[0] == FAT32_ENTRY_END) break;
        if (entries[i].name[0] == FAT32_ENTRY_FREE) continue;
        
        char filename[13];
        int j;
        for (j = 0; j < 8 && entries[i].name[j] != ' '; j++) {
            filename[j] = entries[i].name[j];
        }
        if (entries[i].name[8] != ' ') {
            filename[j++] = '.';
            for (int k = 0; k < 3 && entries[i].name[8 + k] != ' '; k++) {
                filename[j++] = entries[i].name[8 + k];
            }
        }
        filename[j] = '\0';
        
        if (strcmp(filename, path) == 0) {
            unsigned int clus = entries[i].first_cluster_low | 
                               (entries[i].first_cluster_high << 16);
            while (clus < FAT32_CLUSTER_END && clus > 1) {
                unsigned int next = fat32_get_next_cluster(clus);
                fat32_set_next_cluster(clus, FAT32_CLUSTER_FREE);
                clus = (next == clus || next == 0) ? FAT32_CLUSTER_END : next;
            }
            
            entries[i].name[0] = FAT32_ENTRY_FREE;
            ata_write_sectors(sector, 1, dir_data);
            
            print("Deleted: ");
            print(path);
            print("\n");
            return 1;
        }
    }
    
    print("ERROR: Not found!\n");
    return 0;
}

// fat32.c - 修复 fat32_write_file 函数

/* 写入文件 */
int fat32_write_file(const char *path, const void *data, unsigned int size) {
    unsigned int cluster = fs_info.root_cluster;
    unsigned int sector = cluster_to_sector(cluster);
    unsigned char dir_data[512];
    
    if (!ata_read_sectors(sector, 1, dir_data)) return 0;
    
    fat32_dir_entry_t *entries = (fat32_dir_entry_t *)dir_data;
    
    for (int i = 0; i < 16; i++) {
        if (entries[i].name[0] == FAT32_ENTRY_END) break;
        if (entries[i].name[0] == FAT32_ENTRY_FREE) continue;
        
        char filename[13];
        int j;
        for (j = 0; j < 8 && entries[i].name[j] != ' '; j++) {
            filename[j] = entries[i].name[j];
        }
        if (entries[i].name[8] != ' ') {
            filename[j++] = '.';
            for (int k = 0; k < 3 && entries[i].name[8 + k] != ' '; k++) {
                filename[j++] = entries[i].name[8 + k];
            }
        }
        filename[j] = '\0';
        
        if (strcmp(filename, path) == 0) {
            unsigned int file_cluster = entries[i].first_cluster_low | 
                                       (entries[i].first_cluster_high << 16);
            unsigned int file_sector = cluster_to_sector(file_cluster);
            
            // 准备写入数据
            unsigned char write_buffer[512];
            memset(write_buffer, 0, 512);
            
            if (size > 0 && data != NULL) {
                // 复制数据到缓冲区
                unsigned int copy_size = (size < 512) ? size : 512;
                const unsigned char *src = (const unsigned char *)data;
                for (unsigned int k = 0; k < copy_size; k++) {
                    write_buffer[k] = src[k];
                }
            }
            
            // 写入数据扇区
            if (!ata_write_sectors(file_sector, 1, write_buffer)) return 0;
            
            // 更新文件大小
            entries[i].file_size = size;
            
            // 写回目录项
            if (!ata_write_sectors(sector, 1, dir_data)) return 0;
            
            return 1;
        }
    }
    
    return 0;
}

/* 读取文件 */
int fat32_read_file(const char *path, void *buffer, unsigned int size) {
    unsigned int cluster = fs_info.root_cluster;
    unsigned int sector = cluster_to_sector(cluster);
    unsigned char dir_data[512];
    
    if (!ata_read_sectors(sector, 1, dir_data)) return 0;
    
    fat32_dir_entry_t *entries = (fat32_dir_entry_t *)dir_data;
    
    for (int i = 0; i < 16; i++) {
        if (entries[i].name[0] == FAT32_ENTRY_END) break;
        if (entries[i].name[0] == FAT32_ENTRY_FREE) continue;
        
        char filename[13];
        int j;
        for (j = 0; j < 8 && entries[i].name[j] != ' '; j++) {
            filename[j] = entries[i].name[j];
        }
        if (entries[i].name[8] != ' ') {
            filename[j++] = '.';
            for (int k = 0; k < 3 && entries[i].name[8 + k] != ' '; k++) {
                filename[j++] = entries[i].name[8 + k];
            }
        }
        filename[j] = '\0';
        
        if (strcmp(filename, path) == 0) {
            unsigned int file_size = entries[i].file_size;
            if (size > file_size) size = file_size;
            
            unsigned int file_cluster = entries[i].first_cluster_low | 
                                       (entries[i].first_cluster_high << 16);
            unsigned int file_sector = cluster_to_sector(file_cluster);
            
            return ata_read_sectors(file_sector, 1, buffer) ? size : 0;
        }
    }
    
    return 0;
}

/* 检查文件是否存在 */
int fat32_file_exists(const char *path) {
    unsigned int cluster = fs_info.root_cluster;
    unsigned int sector = cluster_to_sector(cluster);
    unsigned char dir_data[512];
    
    if (!ata_read_sectors(sector, 1, dir_data)) return 0;
    
    fat32_dir_entry_t *entries = (fat32_dir_entry_t *)dir_data;
    
    for (int i = 0; i < 16; i++) {
        if (entries[i].name[0] == FAT32_ENTRY_END) break;
        if (entries[i].name[0] == FAT32_ENTRY_FREE) continue;
        
        char filename[13];
        int j;
        for (j = 0; j < 8 && entries[i].name[j] != ' '; j++) {
            filename[j] = entries[i].name[j];
        }
        if (entries[i].name[8] != ' ') {
            filename[j++] = '.';
            for (int k = 0; k < 3 && entries[i].name[8 + k] != ' '; k++) {
                filename[j++] = entries[i].name[8 + k];
            }
        }
        filename[j] = '\0';
        
        if (strcmp(filename, path) == 0) {
            return 1;
        }
    }
    
    return 0;
}

/* 获取文件大小 */
unsigned int fat32_get_file_size(const char *path) {
    unsigned int cluster = fs_info.root_cluster;
    unsigned int sector = cluster_to_sector(cluster);
    unsigned char dir_data[512];
    
    if (!ata_read_sectors(sector, 1, dir_data)) return 0;
    
    fat32_dir_entry_t *entries = (fat32_dir_entry_t *)dir_data;
    
    for (int i = 0; i < 16; i++) {
        if (entries[i].name[0] == FAT32_ENTRY_END) break;
        if (entries[i].name[0] == FAT32_ENTRY_FREE) continue;
        
        char filename[13];
        int j;
        for (j = 0; j < 8 && entries[i].name[j] != ' '; j++) {
            filename[j] = entries[i].name[j];
        }
        if (entries[i].name[8] != ' ') {
            filename[j++] = '.';
            for (int k = 0; k < 3 && entries[i].name[8 + k] != ' '; k++) {
                filename[j++] = entries[i].name[8 + k];
            }
        }
        filename[j] = '\0';
        
        if (strcmp(filename, path) == 0) {
            return entries[i].file_size;
        }
    }
    
    return 0;
}