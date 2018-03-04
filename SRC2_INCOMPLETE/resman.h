#ifndef __RESMAN_H
#define __RESMAN_H

	#include <SDL/SDL.h>
	#include <SDL/SDL_image.h>
	#include <SDL/SDL_mixer.h>
	
	#include "fileman.h"
	#include "common.h"

	void resman_init(int maxsize);
	SDL_Surface* resman_get_image(char* name, int cache);
	Mix_Chunk* resman_get_sound(char* name, int cache);
	void resman_free_all();
	
#endif
