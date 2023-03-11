@echo off

if exist build (
    del /s /q build
) else (
    mkdir build
)

echo.
echo === Building DLLs ===
echo -- d1.dll
g++ -Wall --std=c++14 ./dlltest.cpp -fPIC -shared -O2 -s -o ./build/d1.dll || goto failed

echo.
echo === Building Main === 
gcc -Wall --std=c99 ./deps/dlfcn.c -c -o ./build/dlfcn.o || goto failed
g++ -Wall --std=c++14 -I./deps main.cpp -c -o ./build/main.o || goto failed
g++ ./build/dlfcn.o ./build/main.o -s -O2 -o ./build/main.exe || goto failed

echo.
del build\*.o
echo Build completed.
goto eof

:failed
echo Build failed!

:eof
pause
