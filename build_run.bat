call build.bat || goto :exit__
call run.bat

:exit__
 exit /b %errorlevel%