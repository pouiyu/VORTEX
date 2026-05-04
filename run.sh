#!/bin/bash
# run.sh - 运行VortexOS

echo "======================================="
echo "  Running VortexOS with QEMU"
echo "======================================="

# 检查内核文件是否存在
if [ ! -f kernel.bin ]; then
    echo "ERROR: kernel.bin not found. Please run build.sh first."
    exit 1
fi

# 检查磁盘镜像是否存在
if [ ! -f disk.img ]; then
    echo "Creating new disk image..."
    dd if=/dev/zero of=disk.img bs=1024 count=102400 2>/dev/null
else
    echo "Using existing disk image..."
fi

# 创建ISO（如果需要）
if [ ! -f vortexos.iso ]; then
    echo "Creating ISO image..."
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
    rm -rf iso
fi

echo "Starting QEMU..."
# 使用 -boot d 确保从CD-ROM引导
qemu-system-i386 -cdrom vortexos.iso -hda disk.img -boot d -m 64

echo ""
echo "QEMU exited."