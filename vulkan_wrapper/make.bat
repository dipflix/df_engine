@echo off
rd /s /q build
mkdir build 2>nul
cmake -S . -B build
cmake --build build --config Release
pause
