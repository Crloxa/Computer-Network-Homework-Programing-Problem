@echo off
chcp 65001 > nul
setlocal

REM ?? OpenCV ??
set PATH=E:\Opencv\opencv\build\x64\vc16\bin;%PATH%

REM ?????
set FILE=%1
if "%FILE%"=="" set FILE=1_Sender.cpp

REM ????????????
for %%F in ("%FILE%") do set BASENAME=%%~nF

echo ???? %FILE% ...
E:\LLVM\bin\clang++.exe -g -I E:/Opencv/opencv/build/include %FILE% -L E:/Opencv/opencv/build/x64/vc16/lib -lopencv_world4120 -o %BASENAME%.exe

if %ERRORLEVEL% EQU 0 (
    echo ?????????...
    echo.
    %BASENAME%.exe
) else (
    echo ?????
    pause
)
