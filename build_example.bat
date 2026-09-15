@echo off
setlocal

set PATH=C:\msys64\ucrt64\bin;%PATH%

g++ -std=c++17 ^
    Example\main.cpp ^
    ProvidedGUI\ChessFrontend.cpp ^
    -IProvidedGUI ^
    -o gui_example.exe ^
    -lsfml-graphics ^
    -lsfml-window ^
    -lsfml-system

if errorlevel 1 (
    echo.
    echo BUILD FAILED
    pause
    exit /b 1
)

echo.
echo BUILD SUCCEEDED
echo Run: gui_example.exe
pause
