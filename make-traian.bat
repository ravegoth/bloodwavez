echo off
cls

rem INCHIDEREA PROCESULUI PRECEDENT ##############

taskkill /f /im game.exe >nul

if %ERRORLEVEL% EQU 0 (
    echo Previous process closed
) else (
    @REM echo Failed to close previous process
    cls
)

echo Compiling...

rem COMPILAREA PROIECTULUI #######################

g++ src/main.cpp -o bin/game.exe -I "X:\SFML-3.0.0\include" -L "X:\SFML-3.0.0\lib" -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio -lsfml-network -mwindows -static-libgcc -static-libstdc++

rem EXECUTAREA PROIECTULUI #######################

if %ERRORLEVEL% EQU 0 (
    echo Compilation successful
    echo Running...
    bin\game.exe
    echo Game closed
) else (
    echo Compilation failed
    pause
)

rem STERGERE EXECUTABIL #######################

del bin\game.exe -f