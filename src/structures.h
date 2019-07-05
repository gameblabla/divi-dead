#ifndef STRUCTURES_H
#define STRUCTURES_H

#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <stdarg.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_mixer.h>
#include <SDL/SDL_thread.h>
#include <SDL/SDL_mutex.h>
#include <SDL/SDL_ttf.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct {
	char name[0x20];
	SDL_Surface *surface;
	int used;
} IMAGE_CACHE;

typedef struct {
	char *i_d;
	char *i_e;
	int from, up, down;
} CG_PAGE;

typedef struct {
	int ptr;
	int x1, y1, x2, y2;
} MAP_OPTION;

typedef struct {
	char *text;
	int ptr;
} OPTION;

typedef struct {
	char *text;
	int (*callback)(int);
} OPTION_GAME_MENU;


typedef struct  {
	int pressed;
	uint32_t delay;
	uint32_t time;
} JOYB;

typedef struct {
	unsigned short* chars;
	int chars_count;
	unsigned char* widths;
	int widths_count;
	char* glyphs;
	int glyphs_count;
} FONT;


#endif
