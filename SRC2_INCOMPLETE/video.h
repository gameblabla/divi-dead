#ifndef __VIDEO_H
#define __VIDEO_H

	#include <SDL/SDL.h>
	
	#include "resman.h"
	#include "common.h"
	
	extern SDL_Surface *screen;
	
	void video_character1_draw(char *name1);
	void video_character2_draw(char *name1, char *name2);
	void video_foreground_draw(char* name);
	void video_background_draw(char* name);
	
	void video_transition_play(int type);
	
#endif
