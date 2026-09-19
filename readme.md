# TOMK
 x86 32bit kernel project

## Implemented systems
 - Multiboot v1 boot support
 - IDT
 - GDT
 - Paging (Statically allocated page directory & tables)
 - Higher Half Kernel
 - Memory stack
   - Physical memory manager (PMM)
   - Virtual memory manager (VMM)
   - Heap (KHEAP)
 - PS/2 driver
 - UART driver
 - Kernel processes
 - Pre-emptive Scheduler (Round robin)

## Partially Implemented systems
 - LIBC implementation
 - Debugging infrastructure
 - Timer/Counter driver
 - TTY driver
 - Dynamic page table allocation
 - ELF loader

## TODO
 - Build system for CDROM boot (current build system only outputs a .elf binary)
 - Process stack protection
 - HAL
 - Filesystem
 - Userspace
 - User processes
 - Syscalls
 - Signals

## Windows toolchain

| Dependency           |    Version | Description                                                                                        |
| -------------------- | ---------: | -------------------------------------------------------------------------------------------------- |
| `GNU Make`           |     `3.81` | [GNU Make for Windows](https://gnuwin32.sourceforge.net/packages/make.htm)                         |
| `GCC Cross Compiler` |    `7.1.0` | [i686-elf-tools-windows 7.1.0](https://github.com/lordmilko/i686-elf-tools/releases#release-7.1.0) |
| `QEMU`               | `3.0.93.0` | [QEMU for Windows](https://qemu.weilnetz.de/w64/)                                                  |

## Build
 `cd tomk`

#### Debug
 `make DEBUG=1`

#### Release
 `make DEBUG=0`
 
#### Clean
 `make clean`
 
## Usage
 `qemu-system-i386.exe -kernel tomk.elf -m 2048 -serial stdio`
