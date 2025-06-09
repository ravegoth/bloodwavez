@echo off

echo Compiling and running AI test...
g++ ./test.cpp -o ./test.exe 

if %errorlevel% neq 0 (
    echo Compilation failed.
    exit /b %errorlevel%
)

cls
REM Run the test executable
test.exe

if %errorlevel% neq 0 (
    echo Test execution failed.
    exit /b %errorlevel%
)

del test.exe