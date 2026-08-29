@echo off

set "run_exe=qemu-system-i386.exe"
set "run_elf=bin\kernel\kernel.elf"
set "run_params=-kernel "%run_elf%" -m 2048"

"%run_exe%" %run_params%