#/bin/bash
gcc -O2 $(sdl-config --cflags) SRC/main.c $(sdl-config --libs) -lSDL_mixer -lSDL_image -lSDL_ttf -odividead