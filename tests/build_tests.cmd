@echo off
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
if errorlevel 1 exit /b 1
set PATH=D:\anaconda3\Library\bin;D:\opencv-3.4.16\opencv\build\x64\vc15\bin;%PATH%
cd /d "%~dp0"

cd tst_mainwidget
qmake -spec win32-msvc tst_mainwidget.pro
if errorlevel 1 exit /b 1
nmake
if errorlevel 1 exit /b 1
cd ..\tst_hardware
qmake -spec win32-msvc tst_hardware.pro
if errorlevel 1 exit /b 1
nmake
if errorlevel 1 exit /b 1
cd ..

set QT_QPA_PLATFORM=offscreen
tst_mainwidget\release\tst_mainwidget.exe -o result_mainwidget.txt
if errorlevel 1 exit /b 1
tst_hardware\release\tst_hardware.exe -o result_hardware.txt
if errorlevel 1 exit /b 1
type result_mainwidget.txt
type result_hardware.txt
