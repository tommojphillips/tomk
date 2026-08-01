set "run_exe=F:\Program Files\qemu\qemu-system-i386.exe"
set "run_elf=bin\kernel\kernel.elf"
set "run_params=-kernel "%run_elf%""

echo.
echo Running...
"%run_exe%" %run_params%