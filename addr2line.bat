@echo off

set "elf=bin/kernel/kernel.sym"

i686-elf-addr2line.exe -a -p -s -e %elf% %*
