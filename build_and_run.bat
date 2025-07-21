@echo off
rem ===========================================
rem           Build and Run Script
rem ===========================================

rem Step 1: clean old build
echo Cleaning old build files...
if exist build (
    rmdir /s /q build
)

@echo off
rem Create the build directory if it does not exist
if not exist build mkdir build

rem Compile the program
g++ -g -o build\main.exe main.cpp -static-libgcc -static-libstdc++ -Iinclude
@REM g++ -H -g -Llib -o build\main.exe main.cpp -static-libgcc -static-libstdc++

rem Check if the compilation was successful
if %errorlevel% neq 0 (
    echo Compilation failed.
    exit /b %errorlevel%
)

rem Run the compiled program
rem Redirect std::cout to img.ppm and keep std::clog on the console
echo Compilation successful. Running the program...
build\main.exe 1> out\img.ppm 2> con

if %errorlevel% neq 0 (
    echo Failed to execute the program.
    exit /b %errorlevel%
)
echo Output written to out\img.ppm

rem Use ImageMagick to convert PPM to PNG
magick out\img.ppm out\raytracer.png
if %errorlevel% neq 0 (
    echo Failed to convert img.ppm to PNG.
    exit /b %errorlevel%
)
echo Successfully converted img.ppm to PNG: out\raytracer.png
