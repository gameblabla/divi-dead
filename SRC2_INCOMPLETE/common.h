#ifndef __COMMON_H
#define __COMMON_H

	#include <SDL/SDL.h>
	#include <stdio.h>
	#include <stdlib.h>

	void error_fatal(char *s);
	char *game_normalize_name(char *s);
	SDL_Rect SRECT(int x, int y, int w, int h);
	void SWAIT(int time);
	
#endif
