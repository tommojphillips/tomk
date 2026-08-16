@echo off

set "out_fn=kernel"
set "out_dir=bin\%out_fn%\"
set "obj_dir=obj\%out_fn%\"
set "link_file=src\%out_fn%\linker.ld"

set "dbg=1"

set "c_includes=-I "src\kernel\include" -I "src\driver\include" -I "src\libc\include" -I "src\disasm""
set "c_arguments=-std=gnu99 -ffreestanding -O2 -Wall -Wextra -march=i386"
set "asm_args=-f elf32"
set "obj_str="
set "link_args=-m elf_i386"

if not exist "%out_dir%" (
 mkdir "%out_dir%"
)

if not exist "%obj_dir%" (
 mkdir "%obj_dir%"
)

if %dbg%  == 1 (
    set "c_arguments=%c_arguments% -g"
    set "asm_args=%asm_args% -g"
)

cls

echo.
echo Assembling/Compiling %out_fn%...

REM BOOT
call :assemble "src\boot\" "multiboot.asm" || goto :exit__

REM LIBC
call :assemble_dir "src\libc\" "obj\libc\" "-DLIBK" || goto :exit__
call :compile_dir "src\libc\" "obj\libc\" "-DLIBK" || goto :exit__

REM DRIVERS
call :assemble_dir "src\driver\" "obj\driver\" || goto :exit__
call :compile_dir "src\driver\" "obj\driver\" || goto :exit__

REM KERNEL
call :assemble_dir "src\kernel\" "obj\kernel\" || goto :exit__
call :compile_dir "src\kernel\" "obj\kernel\" || goto :exit__

REM DISASM
call :assemble_dir "src\disasm\" "obj\disasm\" || goto :exit__
call :compile_dir "src\disasm\" "obj\disasm\" || goto :exit__

echo.
echo Linking...

if not exist "%link_file%" (
 echo Link file: %link_file% not found. 
 goto :exit__
)

i686-elf-ld %link_args% "-Map=%out_dir%%out_fn%.map" -T "%link_file%" -o "%out_dir%%out_fn%.elf" %obj_str% || goto :exit__

echo Decompiling..
i686-elf-objdump -m i386 -M intel -d %out_dir%%out_fn%.elf > %out_dir%%out_fn%.asm || goto :exit__
 
if %dbg% == 1 (
 echo Creating symbol file...
 objcopy --only-keep-debug %out_dir%%out_fn%.elf %out_dir%%out_fn%.sym || goto :exit__
 
 echo Stripping...
 objcopy --strip-debug %out_dir%%out_fn%.elf
 
 echo out -^>^ %out_dir%%out_fn%.sym
 echo out -^>^ %out_dir%%out_fn%.elf
) else (
 echo out -^>^ %out_dir%%out_fn%.elf
)
echo out -^>^ %out_dir%%out_fn%.asm

exit /b 0

:assemble
 if not exist "%~1%~2" (
  echo assembly file: %~1%~2 not found. 
  goto :exit__
 )
 nasm %asm_args% "%~1%~2" -o "%obj_dir%%~2.o" -l "%obj_dir%%~3.list" %~3 || goto :exit__
 set "obj_str=%obj_str%%obj_dir%%~2.o "
 setlocal EnableDelayedExpansion
 set "SRC=%~1%~2"
 set "OUT=%obj_dir%%~2.o"
 echo !SRC:%CD%\=! -^>^ !OUT:%CD%\=!
 endlocal
 exit /b 0

:compile
 if not exist "%~1%~2" (
  echo c file: %~1%~2 not found. 
  goto :exit__
 )
 i686-elf-gcc -c "%~1%~2" -o "%obj_dir%%~2.o" %c_includes% %~3 %c_arguments% || goto :exit__
 set "obj_str=%obj_dir%%~2.o %obj_str%"
 setlocal EnableDelayedExpansion
 set "SRC=%~1%~2"
 set "OUT=%obj_dir%%~2.o"
 echo !SRC:%CD%\=! -^>^ !OUT:%CD%\=!
 endlocal
 exit /b 0

:assemble_dir
 for /R "%~1" %%F in (*.asm) do (
  set "obj_dir=%~2" 
  call :assemble %%~dpF "%%~nF.asm" "%~3" || goto :exit__
 )
 exit /b 0

:compile_dir
 for /R "%~1" %%F in (*.c) do (
  set "obj_dir=%~2"
  call :compile %%~dpF "%%~nF.c" "%~3" || goto :exit__
 )
 exit /b 0

:exit__
 exit /b %errorlevel%
 