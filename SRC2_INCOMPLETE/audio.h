#ifndef __AUDIO_H
#define __AUDIO_H

	#include <SDL/SDL.h>
	#include <SDL/SDL_mixer.h>
	
	#include "resman.h"
	#include "common.h"
	
	void sound_effect_play(char *name);
	void sound_effect_stop();
	void sound_voice_play(char *name);
	void sound_gc();
#endif
