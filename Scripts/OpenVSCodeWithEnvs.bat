@echo off

set THISFOLDER=%~dp0
call %THISFOLDER%\..\src\config\sensitive\set_aio_env.bat
if %errorlevel% neq 0 goto error

"C:\Users\Rui\AppData\Local\Programs\Microsoft VS Code\Code.exe" .
if %errorlevel% neq 0 goto error

:: Everything ok
endlocal
goto end

:: Error
:error
endlocal

echo .
echo ERROR!!!
echo This scripts expects .\src\config\sensitive\set_aio_env.bat to exist.
echo That batch file should set a AIO_USER and AIO_KEY environment variables, with your Adafruit IO's credentials
echo .
pause
exit /b %errorlevel%

:: OK End
:end
echo Success

