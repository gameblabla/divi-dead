@echo off
cls
REM tcc\tcc -run -I"TCC\include" -L"TCC\lib" -lSDL SRC\_ringbuf_reading.c
tcc\tcc -DRING_DEBUG -DRING_test -run -I"TCC\include" -L"TCC\lib" -lSDL SRC\ringread.c