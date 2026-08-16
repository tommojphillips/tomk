
doskey addr2line=i686-elf-addr2line.exe -a -p -s -e bin/kernel/kernel.sym $*
doskey b=build.bat
doskey x=qemu-system-i386.exe -kernel bin/kernel/kernel.elf -m 2048 $*