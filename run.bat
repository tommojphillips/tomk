@echo off

set "kernel_dbg=%~1"

if %kernel_dbg% == 1 (
    set "kernel_img=bin\debug\tomk.elf"
) else (
    set "kernel_img=bin\release\tomk.elf"
)

start "" /b powershell -ExecutionPolicy Bypass -File "%~dp0run-qemu.ps1"
qemu-system-i386 -kernel %kernel_img% -m 2048 -serial stdio -display gtk,zoom-to-fit=on
