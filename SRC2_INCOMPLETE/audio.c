#include "hooks.h"
#include "audio.h"

Mix_Music *music = NULL;
Mix_Chunk *voice = NULL, *effect = NULL;
int audio_init = 0;

#define CHN_VOICE  0
#define CHN_EFFECT 1
#define AUDIO_CHECK() if (audio_init == 0) return;

void sound_init() {
	if (!HOOK_AUDIO_HAS) return;
	
	SDL_InitSubSystem(SDL_INIT_AUDIO);
	HOOK_INIT_SOUND_PRE();
	Mix_OpenAudio(
		HOOK_AUDIO_FREQ,
		HOOK_AUDIO_FORMAT,
		HOOK_AUDIO_CHANNELS,
		HOOK_AUDIO_CSIZE
	);
	audio_init = 1;
}

void sound_stop(int channel, Mix_Chunk **chunk, int stop) {
	if (!stop && Mix_Playing(channel)) return;
	Mix_HaltChannel(channel);
	if (!*chunk) return;
	Mix_FreeChunk(*chunk);
	chunk = NULL; 		
}

#define _sound_effect_stop(stop) sound_stop(CHN_EFFECT, &effect, stop)
#define _sound_voice_stop(stop) sound_stop(CHN_VOICE, &voice, stop)

void sound_effect_play(char *name) {
	AUDIO_CHECK();
	_sound_effect_stop(1);
	effect = resman_get_sound(name, 1);
	Mix_PlayChannel(CHN_EFFECT, effect, 0);
}

void sound_effect_stop() {
	AUDIO_CHECK();
	_sound_effect_stop(1);
}

void sound_voice_play(char *name) {
	AUDIO_CHECK();
	_sound_voice_stop(1);
	voice = resman_get_sound(name, 0);
	Mix_PlayChannel(CHN_VOICE, voice, 0);
}

void sound_gc() {
	AUDIO_CHECK();
	_sound_effect_stop(0);
	_sound_voice_stop(0);
}
