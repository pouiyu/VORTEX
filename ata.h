// ata.h - ATA/IDE硬盘驱动头文件
#ifndef ATA_H
#define ATA_H

#include "types.h"

/* ATA端口定义 */
#define ATA_PRIMARY_DATA        0x1F0
#define ATA_PRIMARY_ERROR       0x1F1
#define ATA_PRIMARY_SECTOR_COUNT 0x1F2
#define ATA_PRIMARY_LBA_LOW     0x1F3
#define ATA_PRIMARY_LBA_MID     0x1F4
#define ATA_PRIMARY_LBA_HIGH    0x1F5
#define ATA_PRIMARY_DRIVE       0x1F6
#define ATA_PRIMARY_COMMAND     0x1F7
#define ATA_PRIMARY_STATUS      0x1F7
#define ATA_PRIMARY_CONTROL     0x3F6

/* ATA命令 */
#define ATA_CMD_READ_SECTORS    0x20
#define ATA_CMD_WRITE_SECTORS   0x30
#define ATA_CMD_IDENTIFY        0xEC
#define ATA_CMD_FLUSH           0xE7

/* ATA状态寄存器 */
#define ATA_SR_BSY  0x80
#define ATA_SR_DRDY 0x40
#define ATA_SR_DRQ  0x08
#define ATA_SR_ERR  0x01

/* 磁盘信息结构 */
typedef struct {
    unsigned short cylinders;
    unsigned short heads;
    unsigned short sectors_per_track;
    unsigned int total_sectors;
    char model[41];
    char serial[21];
} disk_info_t;

/* ATA函数 */
int ata_init(void);
int ata_read_sectors(unsigned int lba, unsigned char count, void *buffer);
int ata_write_sectors(unsigned int lba, unsigned char count, const void *buffer);
int ata_identify(disk_info_t *info);

#endif