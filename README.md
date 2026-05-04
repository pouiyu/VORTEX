# VortexOS

<div align="center">

![VortexOS Logo](https://img.shields.io/badge/VortexOS-Operating%20System-blue)
![License](https://img.shields.io/badge/license-MIT-green)
![Architecture](https://img.shields.io/badge/arch-x86--32-orange)
![状态](https://img.shields.io/badge/status-development-yellow)

**一个基于 x86 架构的轻量级操作系统内核**

[功能特性](#-功能特性) • [快速开始](#-快速开始) • [系统架构](#-系统架构) • [开发文档](#-开发文档) • [贡献指南](#-贡献指南)

</div>

---

## 📖 项目简介

VortexOS 是一个从零开始构建的 32 位 x86 操作系统，旨在帮助开发者深入理解操作系统的底层原理。该项目采用单体内核架构，实现了从引导加载、硬件初始化到文件系统管理的完整操作系统核心功能。

### ✨ 核心特性

- **🖥️ VGA 文本模式驱动** - 支持彩色文本显示和光标控制
- **⌨️ 键盘输入处理** - 实时捕获和处理键盘事件
- **💾 FAT32 文件系统** - 完整的文件读写、目录管理功能
- **🔧 ATA 磁盘驱动** - IDE 硬盘控制器支持
- **📜 BRC 脚本引擎** - 自定义批处理脚本执行环境
- **🕐 RTC/PIT 定时器** - 实时时钟和可编程间隔定时器
- **🐚 交互式 Shell** - 内置命令历史和丰富的系统工具
- **⚡ 系统控制** - 支持关机和重启操作

---

## 🚀 快速开始

### 前置要求

在开始之前，请确保您的系统已安装以下工具：

```bash
# Linux/WSL (推荐)
sudo apt-get install nasm gcc qemu-system-x86 grub-pc-bin xorriso

# macOS
brew install nasm gcc qemu grub

# Windows (使用 WSL2 或 MSYS2)
# 推荐使用 WSL2 以获得最佳兼容性
```

### 编译与运行

#### 方式一：使用构建脚本（推荐）

```bash
# 赋予执行权限
chmod +x build.sh run.sh

# 一键编译并运行
./build.sh
```

#### 方式二：手动编译

```bash
# 1. 清理旧文件
rm -f *.o kernel.bin vortexos.iso

# 2. 编译汇编代码
nasm -f elf32 boot.asm -o boot.o

# 3. 编译 C 源文件
gcc -m32 -c -ffreestanding -O2 -Wall -Wextra -I. *.c

# 4. 链接内核
ld -m elf_i386 -T link.ld -o kernel.bin \
    boot.o string.o vga.o keyboard.o rtc.o pit.o \
    system.o ata.o fat32.o fs.o shell.o brc.o cmd_exec.o kernel.o

# 5. 创建 ISO 镜像
mkdir -p iso/boot/grub
cp kernel.bin iso/boot/
# 创建 grub.cfg 配置文件
grub-mkrescue -o vortexos.iso iso/

# 6. 创建磁盘镜像（首次运行）
dd if=/dev/zero of=disk.img bs=1024 count=102400

# 7. 启动 QEMU
qemu-system-i386 -cdrom vortexos.iso -hda disk.img -boot d -m 64
```

### 首次启动

首次启动时，系统会提示您格式化磁盘以使用 FAT32 文件系统：

```
Disk is not formatted.
Do you want to format the disk as FAT32? (y/n): y

Formatting disk...
Format successful!
Filesystem mounted successfully!
Type 'help' for available commands.
```

---

## 💻 系统架构

### 整体架构图

```
┌─────────────────────────────────────┐
│         User Space (Shell)          │
│  ┌──────────┬──────────┬──────────┐ │
│  │ Commands │ Scripts  │ Utils    │ │
│  └──────────┴──────────┴──────────┘ │
├─────────────────────────────────────┤
│      Kernel Space (Ring 0)          │
│  ┌────────────────────────────────┐ │
│  │   Shell & Command Handler      │ │
│  ├────────────────────────────────┤ │
│  │   File System Layer            │ │
│  │  (FAT32 / FS Abstraction)      │ │
│  ├────────────────────────────────┤ │
│  │   Device Drivers               │ │
│  │  (ATA / Keyboard / VGA)        │ │
│  ├────────────────────────────────┤ │
│  │   Hardware Abstraction         │ │
│  │  (RTC / PIT / System)          │ │
│  └────────────────────────────────┘ │
├─────────────────────────────────────┤
│         Hardware (x86)              │
│  CPU | Memory | I/O Ports | Disks   │
└─────────────────────────────────────┘
```

### 内存布局

根据 `link.ld` 链接器脚本配置：

| 段名称 | 起始地址 | 对齐方式 | 说明 |
|--------|---------|---------|------|
| `.text` | 1 MB | 4 KB | 代码段（包含 Multiboot 头） |
| `.rodata` | - | 4 KB | 只读数据段 |
| `.data` | - | 4 KB | 数据段 |
| `.bss` | - | 4 KB | 未初始化数据段 |

### 引导流程

```
BIOS → Bootloader (boot.asm) → Kernel Entry (start) → kernel_main() → Shell
```

1. **BIOS 阶段**: 实模式初始化
2. **Bootloader**: 切换到保护模式，加载内核到 1MB 位置
3. **Kernel Entry**: 设置栈指针，跳转到 C 语言入口
4. **kernel_main**: 初始化硬件驱动和文件系统
5. **Shell**: 进入交互式命令行界面

---

## 📁 项目结构

```
VortexOS/
├── 📄 核心文件
│   ├── boot.asm          # 引导加载程序（汇编）
│   ├── kernel.c          # 内核主入口
│   ├── link.ld           # 链接器脚本
│   └── types.h           # 基础类型定义
│
├── 🎨 显示模块
│   ├── vga.c/h           # VGA 文本模式驱动
│   └── string.c/h        # 字符串处理库
│
├── ⌨️ 输入模块
│   ├── keyboard.c/h      # PS/2 键盘驱动
│   └── shell.c/h         # Shell 命令解释器
│
├── 💾 存储模块
│   ├── ata.c/h           # ATA/IDE 磁盘驱动
│   ├── fat32.c/h         # FAT32 文件系统实现
│   └── fs.c/h            # 文件系统抽象层
│
├── ⏱️ 定时模块
│   ├── rtc.c/h           # 实时时钟驱动
│   └── pit.c/h           # 可编程间隔定时器
│
├── 🔧 系统模块
│   ├── system.c/h        # 系统调用与控制
│   ├── brc.c/h           # BRC 脚本引擎
│   └── cmd_exec.c/h      # 统一命令执行框架
│
├── 🛠️ 构建工具
│   ├── build.sh          # 自动化构建脚本
│   ├── run.sh            # 快速运行脚本
│   └── iso/              # ISO 镜像生成目录
│
└── 📚 参考实现
    └── xv6-riscv/        # xv6 RISC-V 参考代码
```

---

## 📋 可用命令

启动系统后，您可以在 Shell 中使用以下命令：

### 系统信息
| 命令 | 说明 | 示例 |
|------|------|------|
| `info` | 显示系统信息 | `info` |
| `time` | 显示当前时间 | `time` |
| `date` | 显示当前日期 | `date` |
| `timecode` | 显示完整时间戳 | `timecode` |

### 实用工具
| 命令 | 说明 | 示例 |
|------|------|------|
| `clear` | 清屏 | `clear` |
| `echo` | 输出文本 | `echo Hello World` |
| `logo` | 显示 VortexOS Logo | `logo` |
| `calc` | 启动计算器 | `calc` |

### 文件系统
| 命令 | 说明 | 示例 |
|------|------|------|
| `ls [dir]` | 列出目录内容 | `ls /` |
| `mk <file>` | 创建文件 | `mk test.txt` |
| `md <dir>` | 创建目录 | `md docs` |
| `rm <path>` | 删除文件/目录 | `rm test.txt` |
| `cat <file>` | 查看文件内容 | `cat readme.txt` |
| `wr <file>` | 写入文件（多行，以 EWT 结束） | `wr note.txt` |
| `df` | 显示磁盘信息 | `df` |

### 脚本执行
| 命令 | 说明 | 示例 |
|------|------|------|
| `run <file>` | 执行 .brc 脚本文件 | `run script.brc` |

### 系统控制
| 命令 | 说明 | 示例 |
|------|------|------|
| `shutdown` | 关闭系统 | `shutdown` |
| `reboot` | 重启系统 | `reboot` |

---

## 🔨 开发文档

### 添加新命令

1. 在 `shell.c` 的命令处理部分添加新的 `if-else` 分支
2. 实现命令逻辑函数
3. 在 `help` 命令中添加使用说明

示例：
```c
if (strcmp(cmd, "mycommand") == 0) {
    print("My custom command!\n");
}
```

### 添加新驱动

1. 创建驱动文件 `driver.c/h`
2. 在 `kernel.c` 的 `kernel_main()` 中初始化驱动
3. 更新 `build.sh` 添加编译规则

### 调试技巧

```bash
# 启用 GDB 调试
qemu-system-i386 -cdrom vortexos.iso -hda disk.img -boot d -m 64 -s -S

# 在另一个终端连接 GDB
gdb kernel.bin
(gdb) target remote localhost:1234
(gdb) break kernel_main
(gdb) continue
```

---

## 🧪 测试

### 手动测试清单

- [ ] VGA 显示正常（颜色、光标）
- [ ] 键盘输入响应正确
- [ ] 文件系统读写操作
- [ ] RTC 时间读取准确
- [ ] Shell 命令执行无误
- [ ] BRC 脚本解析正确
- [ ] 系统关机/重启功能

### 已知问题

- 目前仅支持单核 CPU
- 不支持 USB 设备
- 网络功能尚未实现
- 图形界面模式未开发

---

## 🤝 贡献指南

我们欢迎所有形式的贡献！如果您想参与项目开发，请遵循以下步骤：

1. **Fork 本仓库**
2. **创建特性分支**: `git checkout -b feature/amazing-feature`
3. **提交更改**: `git commit -m 'Add amazing feature'`
4. **推送到分支**: `git push origin feature/amazing-feature`
5. **开启 Pull Request**

### 代码规范

- 使用 K&R 风格的 C 代码格式
- 函数名使用小写加下划线（如 `kernel_main`）
- 每个函数添加简要注释说明功能
- 保持代码简洁，避免过度复杂的逻辑

---

## 📄 许可证

本项目采用 MIT 许可证 - 详见 [LICENSE](LICENSE) 文件

---

## 🙏 致谢

- **[OSDev Wiki](https://wiki.osdev.org/)** - 操作系统开发知识库
- **[xv6](https://github.com/mit-pdos/xv6-public)** - MIT 教学操作系统参考
- **[QEMU](https://www.qemu.org/)** - 强大的硬件模拟器
- **[GNU GRUB](https://www.gnu.org/software/grub/)** - 引导加载程序

---

<div align="center">

由通义灵码编写

</div>
