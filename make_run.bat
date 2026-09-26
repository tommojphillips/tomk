@echo off 

set "kernel_dbg=1"

if not "%~1" == "" (
    set "kernel_dbg=%~1"
)

make -j DEBUG=%kernel_dbg% || goto :error 
run.bat %kernel_dbg%

:error
 exit /b 0
