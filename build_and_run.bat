@echo off
mkdir build
cd build
cmake ..
cmake --build . --config Debug
cd Debug
LMS.exe
