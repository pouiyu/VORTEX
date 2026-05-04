#!/bin/bash
# build.sh - 编译并运行VortexOS

set -e

echo "======================================="
echo "  VortexOS Build Script"
echo "======================================="

# 清理
echo "Cleaning old build files..."
rm -f *.o kernel.bin vortexos.iso
rm -rf iso

# 编译汇编
echo "[1/2] Compiling assembly..."
nasm -f elf32 boot.asm -o boot.o

# 编译C源文件
echo "[2/2] Compiling C sources..."

CFLAGS="-m32 -c -ffreestanding -O2 -Wall -Wextra -I."

echo "  - string.c"
gcc $CFLAGS string.c -o string.o
echo "  - vga.c"
gcc $CFLAGS vga.c -o vga.o
echo "  - keyboard.c"
gcc $CFLAGS keyboard.c -o keyboard.o
echo "  - rtc.c"
gcc $CFLAGS rtc.c -o rtc.o
echo "  - pit.c"
gcc $CFLAGS pit.c -o pit.o
echo "  - system.c"
gcc $CFLAGS system.c -o system.o
echo "  - ata.c"
gcc $CFLAGS ata.c -o ata.o
echo "  - fat32.c"
gcc $CFLAGS fat32.c -o fat32.o
echo "  - fs.c"
gcc $CFLAGS fs.c -o fs.o
echo "  - shell.c"
gcc $CFLAGS shell.c -o shell.o
echo "  - kernel.c"
gcc $CFLAGS kernel.c -o kernel.o
echo "  - brc.c"
gcc $CFLAGS brc.c -o brc.o
# 在编译部分添加
echo "  - cmd_exec.c"
gcc $CFLAGS cmd_exec.c -o cmd_exec.o

# 链接
ld -m elf_i386 -T link.ld -o kernel.bin \
    boot.o string.o vga.o keyboard.o rtc.o pit.o \
    system.o ata.o fat32.o fs.o shell.o brc.o cmd_exec.o kernel.o

# 创建ISO
mkdir -p iso/boot/grub
cp kernel.bin iso/boot/

cat > iso/boot/grub/grub.cfg << 'EOF'
set timeout=0
set default=0

menuentry "VortexOS" {
    multiboot /boot/kernel.bin
    boot
}
EOF

grub-mkrescue -o vortexos.iso iso/ 2>/dev/null

echo ""
echo "======================================="
echo "  Build Complete!"
echo "======================================="

# 检查磁盘镜像是否存在
if [ ! -f disk.img ]; then
    echo "Creating new disk image..."
    dd if=/dev/zero of=disk.img bs=1024 count=102400 2>/dev/null
else
    echo "Using existing disk image..."
fi

echo "Starting VortexOS in QEMU..."
# 修改QEMU启动参数，设置从CD-ROM引导
qemu-system-i386 -cdrom vortexos.iso -hda disk.img -boot d -m 64