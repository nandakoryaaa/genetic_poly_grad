g++ %~n1.c -std=c99 -O5 -lSDL2main -lSDL2 -IC:\usr\dev\SDL2-2.0.16\x86_64-w64-mingw32\include -o %~n1.exe
rem -S -fverbose-asm -masm=intel
