#ifndef __FILEMAN_H
#define __FILEMAN_H

	#include <SDL/SDL.h>

	void fileman_init();
	SDL_RWops* fileman_get(char* name);
	
#endif
