#/usr/bin/sh
gcc \
-I/Library/Frameworks/SDL.framework/Headers \
-I/Library/Frameworks/SDL_mixer.framework/Headers \
-I/Library/Frameworks/SDL_image.framework/Headers \
-I/Library/Frameworks/SDL_ttf.framework/Headers \
-I. \
-framework SDL \
-framework SDL_image \
-framework SDL_mixer \
-framework SDL_ttf \
-framework Cocoa \
-ISRC/smpeg \
-LSRC/smpeg \
-lsmpeg \
-lstdc++ \
SDLMain.m \
SRC/main.c -o dividead_mac
