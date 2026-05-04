// ata.c - ATA/IDE硬盘驱动实现
#include "ata.h"
#include "io.h"
#include "vga.h"
#include "string.h"

static int ata_wait_ready(void) {
    unsigned char status;
    /* 等待BSY清零 */
    while ((status = inb(ATA_PRIMARY_STATUS)) & ATA_SR_BSY);
    return (status & ATA_SR_DRDY) ? 1 : 0;
}

static int ata_wait_drq(void) {
    unsigned char status;
    /* 等待DRQ置位 */
    while (1) {
        status = inb(ATA_PRIMARY_STATUS);
        if (status & ATA_SR_ERR) return 0;
        if (status & ATA_SR_DRQ) return 1;
    }
}

/* 初始化ATA驱动 */
int ata_init(void) {
    print("Initializing ATA driver...\n");
    
    /* 选择主盘 */
    outb(ATA_PRIMARY_DRIVE, 0xE0);
    
    /* 等待就绪 */
    for (int i = 0; i < 1000; i++) {
        unsigned char status = inb(ATA_PRIMARY_STATUS);
        if (status & ATA_SR_DRDY) {
            print("ATA drive detected and ready.\n");
            return 1;
        }
        for (volatile int j = 0; j < 10000; j++);
    }
    
    print("No ATA drive detected.\n");
    return 0;
}

/* 读取扇区 */
int ata_read_sectors(unsigned int lba, unsigned char count, void *buffer) {
    unsigned short *buf = (unsigned short *)buffer;
    
    /* 等待驱动器就绪 */
    if (!ata_wait_ready()) return 0;
    
    /* 设置LBA地址（28位）*/
    outb(ATA_PRIMARY_DRIVE, 0xE0 | ((lba >> 24) & 0x0F));
    outb(ATA_PRIMARY_ERROR, 0x00);
    outb(ATA_PRIMARY_SECTOR_COUNT, count);
    outb(ATA_PRIMARY_LBA_LOW, lba & 0xFF);
    outb(ATA_PRIMARY_LBA_MID, (lba >> 8) & 0xFF);
    outb(ATA_PRIMARY_LBA_HIGH, (lba >> 16) & 0xFF);
    
    /* 发送读命令 */
    outb(ATA_PRIMARY_COMMAND, ATA_CMD_READ_SECTORS);
    
    /* 读取数据 */
    for (int s = 0; s < count; s++) {
        if (!ata_wait_drq()) return 0;
        
        /* 读取256个字（512字节）*/
        for (int i = 0; i < 256; i++) {
            buf[s * 256 + i] = inw(ATA_PRIMARY_DATA);
        }
    }
    
    return 1;
}

/* 写入扇区 */
int ata_write_sectors(unsigned int lba, unsigned char count, const void *buffer) {
    const unsigned short *buf = (const unsigned short *)buffer;
    
    /* 等待驱动器就绪 */
    if (!ata_wait_ready()) return 0;
    
    /* 设置LBA地址 */
    outb(ATA_PRIMARY_DRIVE, 0xE0 | ((lba >> 24) & 0x0F));
    outb(ATA_PRIMARY_ERROR, 0x00);
    outb(ATA_PRIMARY_SECTOR_COUNT, count);
    outb(ATA_PRIMARY_LBA_LOW, lba & 0xFF);
    outb(ATA_PRIMARY_LBA_MID, (lba >> 8) & 0xFF);
    outb(ATA_PRIMARY_LBA_HIGH, (lba >> 16) & 0xFF);
    
    /* 发送写命令 */
    outb(ATA_PRIMARY_COMMAND, ATA_CMD_WRITE_SECTORS);
    
    /* 写入数据 */
    for (int s = 0; s < count; s++) {
        if (!ata_wait_drq()) return 0;
        
        /* 写入256个字 */
        for (int i = 0; i < 256; i++) {
            outw(ATA_PRIMARY_DATA, buf[s * 256 + i]);
        }
    }
    
    /* 刷新缓存 */
    outb(ATA_PRIMARY_COMMAND, ATA_CMD_FLUSH);
    ata_wait_ready();
    
    return 1;
}

/* 识别磁盘 */
int ata_identify(disk_info_t *info) {
    unsigned short identify_data[256];
    memset(identify_data, 0, sizeof(identify_data));
    
    /* 等待就绪 */
    if (!ata_wait_ready()) return 0;
    
    /* 发送IDENTIFY命令 */
    outb(ATA_PRIMARY_DRIVE, 0xE0);
    outb(ATA_PRIMARY_COMMAND, ATA_CMD_IDENTIFY);
    
    /* 检查状态 */
    unsigned char status = inb(ATA_PRIMARY_STATUS);
    if (status == 0) return 0;
    
    /* 等待数据 */
    while ((status = inb(ATA_PRIMARY_STATUS)) & ATA_SR_BSY) {
        if (status & ATA_SR_ERR) return 0;
    }
    
    /* 读取识别数据 */
    for (int i = 0; i < 256; i++) {
        identify_data[i] = inw(ATA_PRIMARY_DATA);
    }
    
    /* 解析数据 */
    info->cylinders = identify_data[1];
    info->heads = identify_data[3];
    info->sectors_per_track = identify_data[6];
    
    /* 使用memcpy避免类型双关问题 */
    unsigned int total_sectors;
    memcpy(&total_sectors, &identify_data[60], sizeof(unsigned int));
    info->total_sectors = total_sectors;
    
    /* 提取型号（需要字节交换）*/
    for (int i = 0; i < 20; i++) {
        info->model[i * 2] = (identify_data[27 + i] >> 8) & 0xFF;
        info->model[i * 2 + 1] = identify_data[27 + i] & 0xFF;
    }
    info->model[40] = '\0';
    
    /* 去除尾部空格 */
    for (int i = 39; i >= 0; i--) {
        if (info->model[i] == ' ') info->model[i] = '\0';
        else break;
    }
    
    return 1;
}