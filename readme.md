# TOMK

### Windows toolchain

| Dependency           |    Version | Description                                                                                    |
| -------------------- | ---------: | ---------------------------------------------------------------------------------------------- |
| `GNU Make`           |     `3.81` | [GNU Make for Windows](https://gnuwin32.sourceforge.net/packages/make.htm)                     |
| `GCC Cross Compiler` |    `7.1.0` | [i686-elf-tools 7.1.0](https://github.com/lordmilko/i686-elf-tools/releases#release-7.1.0)     |
| `QEMU`               | `3.0.93.0` | [QEMU for Windows](https://qemu.weilnetz.de/w64/)                                              |


### Build
 ```bat
 cd tomk
 ```

 Debug
 ```bat
 make DEBUG=1
 ```

 Release
 ```bat
 make DEBUG=0
 ```
 
 Clean
 ```bat
 make clean
 ```
 