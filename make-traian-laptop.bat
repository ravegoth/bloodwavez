echo off
cls

rem INCHIDEREA PROCESULUI PRECEDENT ##############

taskkill /f /im bloodwavez.exe >nul

if %ERRORLEVEL% EQU 0 (
    echo Previous process closed
) else (
    @REM echo Failed to close previous process
    cls
)

echo Compiling...

rem COMPILAREA PROIECTULUI #######################

C:\mingw64\bin\g++.exe src/main.cpp -o bin/bloodwavez.exe -I "C:\SFML-3.0.0\include" -L "C:\SFML-3.0.0\lib" -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio -lsfml-network -mwindows -static-libgcc -static-libstdc++

rem EXECUTAREA PROIECTULUI #######################

if %ERRORLEVEL% EQU 0 (
    echo Compilation successful
    echo Running...
    bin\bloodwavez.exe
    echo Game closed
) else (
    echo Compilation failed
    pause
)

rem STERGERE EXECUTABIL #######################

del bin\bloodwavez.exe -f

if %ERRORLEVEL% EQU 0 (
    echo Executable deleted
) else (
    echo Failed to delete executable
    pause
)