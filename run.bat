@echo off
start "" /b powershell -ExecutionPolicy Bypass -File "%~dp0run-qemu.ps1" %*
qemu-system-i386 -kernel bin\debug\tomk.elf -m 2048 -serial stdio -display gtk,zoom-to-fit=on
