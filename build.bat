@echo off
REM Just a little file to compile C/C++.
REM You will need MSVC in order to use it.

setlocal

REM Set your stuff here
set LIBS=ws2_32.lib
set SOURCES_EXE=src/main.c
set INCLUDES_EXE=
set DEFINES=

set OUTPUT_DIR=build
set NAME=bog
REM ////////////////////

set CFLAGS_EXE= 
set LFLAGS_EXE=
set CLPATH=

set "TAB=   "

set "vswhere_exe=%~dp0\vswhere.exe"
set "vcvars_path="
set "clpath="

if not exist "C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe" (
	if not exist "%vswhere_exe%" (
		echo [+] Unable to find vswhere.exe, downloading...
		powershell -command "(New-Object Net.WebClient).DownloadFile('https://github.com/Microsoft/vswhere/releases/latest/download/vswhere.exe', '%vswhere_exe%')"
	)
) else (
	set "vswhere_exe=C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe"
)

echo [+] Searching for cl.exe
for /f "tokens=*" %%a in ('"%vswhere_exe%" -latest -property installationPath') do (
    set "vs_path=%%a"
	set "vcvars_path=%%a\VC\Auxiliary\Build\vcvars64.bat"
	for /f "delims=" %%b in ('dir /b /ad "%%a\VC\Tools\MSVC" ^| findstr /r "^14\.[0-9]*\.[0-9]*$"') do set "clpath=%%a\VC\Tools\MSVC\%%b\bin\Hostx64\x64\cl.exe"
)

if "%cl_path%"=="" (
	for /f "delims=" %%a in ('where cl.exe') do (
		set "clpath=%%a"
	)
)

if defined vs_path (
	echo [+] Searching for vcvars64.bat
    if not exist "%vcvars_path%" (
        echo "vcvars64.bat" not found at: "%vs_path%\VC\Auxiliary\Build\vcvars64.bat"
		exit 1
    )

	if not exist "%clpath%" (
		echo [-] Could not find cl.exe
		exit 1
	)
) else (
    echo Visual Studio not found.
	exit 1
)

echo [+] Calling vcvars64.bat
echo:

call "%vcvars_path%" >nul

set NEW_INCLUDES_EXE=
set NEW_DEFINES_EXE=
setlocal EnableDelayedExpansion
for %%a in (%INCLUDES_EXE%) do (
	if exist %%a (
		set "NEW_INCLUDES_EXE=!NEW_INCLUDES_EXE! /I %%a"
	) else (
		echo [-] Error! Path %%a does not exist
		exit 1
	)
)

set "isval="
for %%a in (%DEFINES%) do (
	if defined isval (
		set "NEW_DEFINES_EXE=!NEW_DEFINES_EXE!=%%a"
		set "isval="
	) else (
		set "NEW_DEFINES_EXE=!NEW_DEFINES_EXE! /D%%a"
		set "isval=y"
	)
)


set CFLAGS=/Z7 /W4 /sdl /MT /GR- /EHa /WX /nologo /Oi /Gm- /MP
set LFLAGS=/opt:ref /DEBUG

echo [+] Libraries: %LIBS%
echo [+] CFlags: %CFLAGS%
echo [+] LFlags: %LFLAGS%
echo [+] Sources: %SOURCES_EXE%
echo [+] Includes: %INCLUDE%

REM Print include paths
if not "%INCLUDES_EXE%"=="" (
	echo [+] Custom Includes: 
	echo [+] Include paths:
	for %%a in (%INCLUDES_EXE%) do (
		echo [+] %TAB%%%a
	)
)

REM Print defines
if not "%DEFINES%"=="" (
	echo [+] Defines: %DEFINES%
)

if not exist %OUTPUT_DIR% (
	mkdir %OUTPUT_DIR%
)

echo:
echo [+] Building...

"%clpath%" %NEW_INCLUDES_EXE% %NEW_DEFINES_EXE% %DEFINES_EXE% %CFLAGS% %CFLAGS_EXE% %SOURCES_EXE% /link %LFLAGS% %LFLAGS_EXE% %LIBS% /OUT:%OUTPUT_DIR%/%NAME%.exe

if %errorlevel% neq 0 (
	echo [-] Failed compilation, exiting...
	exit 1
)

exit 0