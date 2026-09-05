# TOMK
 x86 32bit kernel project

## Implemented systems
 - Multiboot v1 boot support
 - Higher-Half-Kernel
 - Kernel address-space protection (.text & .rodata sections are write protected)
 - Memory stack
   - Physical memory manager (PMM)
   - Virtual memory manager (VMM)
   - Heap (KHEAP)
 - PS/2 driver

## Partially Implemented systems
 - LIBC implementation
 - Debugging infrastructure
 - Timer/Counter driver
 - TTY driver

## TODO
 - Build system for CDROM boot
 - Kernel stack protection
 - HAL
 - Scheduler
 - Filesystem
 - Userspace
 - Userspace Applications

## Windows toolchain

| Dependency           |    Version | Description                                                                                    |
| -------------------- | ---------: | ---------------------------------------------------------------------------------------------- |
| `GNU Make`           |     `3.81` | [GNU Make for Windows](https://gnuwin32.sourceforge.net/packages/make.htm)                     |
| `GCC Cross Compiler` |    `7.1.0` | [i686-elf-tools-windows 7.1.0](https://github.com/lordmilko/i686-elf-tools/releases#release-7.1.0)     |
| `QEMU`               | `3.0.93.0` | [QEMU for Windows](https://qemu.weilnetz.de/w64/)                                              |

## Build
 `cd tomk`

#### Debug
 `make DEBUG=1`

#### Release
 `make DEBUG=0`
 
#### Clean
 `make clean`
 
## Usage
 `qemu-system-i386 -kernel tomk.elf -m 2048 -serial stdio`
