@echo off

:: NOTE: Using :: for comments is problematic inside (...) blocks, so I'll be using rem
:: See https://stackoverflow.com/questions/12407800/which-comment-style-should-i-use-in-batch-files

setlocal
set THISFOLDER=%~dp0
pushd %THISFOLDER%

if not exist "./temp" (
	md temp
)

if not exist "./temp/extra_deps" (
	md temp\extra_deps
)

rem My development setup normally has all my personal project repositories in <ROOTFOLDER>\crazygaze\<REPO>,
rem so if a given folder name is found at the same level as AutoWatering, then it's probably the thing to use,
rem and we create a link.
rem If there isn't one, the we clone the repo

rem ** czmicromuc **
if not exist "./temp/extra_deps/czmicromuc" (
	if exist "./../czmicromuc" (
		rem Link to the already existing repo
		mklink /J "./temp/extra_deps/czmicromuc" "../czmicromuc/lib"
	) else (
		echo ***Cloning czmicromuc***
		echo on
		git clone https://github.com/ruifig/czmicromuc "./temp/repos/czmicromuc"
		rem Link to just the "lib" folder
		mklink /J "./temp/extra_deps/czmicromuc" "./temp/repos/czmicromuc/lib"
		echo off
	)
)

:: Everything ok
endlocal
goto end

:: Error
:error
endlocal
echo ERROR!!
exit /b %errorlevel%

:end
popd
pause


