@echo off

set THISFOLDER=%~dp0

if not defined AIO_USER (
	goto setup_env
)

if not defined AIO_KEY (
	goto setup_env
)

goto runtool

:setup_env
call %THISFOLDER%\..\src\config\sensitive\set_aio_env.bat
if %errorlevel% neq 0 goto env_error

:runtool
python %THISFOLDER%\AutoWatering.py %*
if %errorlevel% neq 0 goto error

:: Everything ok
endlocal
goto end

:: Error
:error
endlocal
pause
exit /b %errorlevel%

:: Error
:env_error
endlocal

echo .
echo ERROR!!!
echo This script expects ..\src\config\sensitive\set_aio_env.bat to exist.
echo That batch file should set a AIO_USER and AIO_KEY environment variables, with your Adafruit IO's credentials
echo .
pause
exit /b %errorlevel%

:: OK End
:end
echo Success

