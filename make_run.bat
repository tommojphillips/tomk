@echo off 
make -j DEBUG=1 || goto :error 
run.bat 1

:error
 exit /b 0
