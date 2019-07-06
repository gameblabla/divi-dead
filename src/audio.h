#ifndef AUDIO_H
#define AUDIO_H

#include "shared.h"

extern void GAME_MUSIC_PLAY(char *name);
extern void GAME_SOUND_PLAY(char *name);
extern void GAME_VOICE_PLAY(char *name);
extern void GAME_VOICE_STOP();
extern void GAME_SOUND_STOP();
extern void GAME_SOUND_CLICK();
extern void GAME_SOUND_GC_COLLECT();

extern void GAME_SOUND_PLAY_CH(Mix_Chunk **chunk, char *name, int channel);
extern void GAME_MUSIC_STOP();

#endif
