////////////////////////////////////////////////////////////
// AUDIO                                                  //
////////////////////////////////////////////////////////////
#include "audio.h"

#ifndef Mix_LoadMUS_RW
extern DECLSPEC Mix_Music * SDLCALL Mix_LoadMUS_RW(SDL_RWops *rw);
#endif

#ifdef RINGBUF_AUDIO
#define LoadFromMusic(a)  (music = Mix_LoadMUS_RW(RING_RW_open(SDL_RWFromFile(a, "rb"), 0x60000, 0x6000)))
#else
#define LoadFromMusic(a)  (music = Mix_LoadMUS_RW(SDL_RWFromFile(temp, "rb")))
#endif

void GAME_MUSIC_PLAY(char *name) {
	char temp[512], temp2[512], temp3[512], music_name[256];
	char *tempp;
	
	//printf("%s\n", save.music);
	
	if (!audio_initialized) return;
	if (music) {
		//printf("GAME_MUSIC_PLAY(): (1)\n");
		Mix_HaltMusic();
		//printf("GAME_MUSIC_PLAY(): (2)\n");
		Mix_FreeMusic(music);
		//printf("GAME_MUSIC_PLAY(): (3)\n");
		music = NULL;
	}

	if (name) strcpy(save.music, name);

	if (!strlen(save.music)) return;
	if (!audio_music_enabled) return;
	
	//printf("GAME_MUSIC_PLAY(): (4)\n");
	
	#if defined(GAME_HOME_DIRECTORY)
	snprintf(music_name, sizeof(music_name), "%s", save.music);
	for (tempp = music_name; (*tempp != '\0'); tempp++) *tempp = toupper(*tempp);
	#endif

	#ifndef DISABLE_MIDI
		#if defined(GAME_HOME_DIRECTORY)
		snprintf(temp, sizeof(temp), strrchr(save.music, '.') ? "%s/MID/%s" : "%s/MID/%s.MID", game_directory, music_name);
		#else
		snprintf(temp, sizeof(temp), strrchr(save.music, '.') ? "%sMID/%s" : "%sMID/%s.MID", FILE_PREFIX, save.music);
		for (tempp = temp; *tempp; tempp++) if (*tempp >= 'a' && *tempp <= 'z') *tempp = ((*tempp - 'a') + 'A');
		#endif
	#endif

	#ifdef AUDIO_CHECK_MOD
		//printf("GAME_MUSIC_PLAY(): (5) '%s'\n", save.music);
		// Using OGG
		#if defined(GAME_HOME_DIRECTORY)
		snprintf(temp2, sizeof(temp2), strrchr(save.music, '.') ? "%s/OGG/%s.OGG" : "%s/OGG/%s.MID.OGG", game_directory, music_name);
		snprintf(temp3, sizeof(temp3), strrchr(save.music, '.') ? "%s/ogg/%s.ogg" : "%s/ogg/%s.mid.ogg", game_directory, save.music);
		#else
		snprintf(temp2, sizeof(temp2), strrchr(save.music, '.') ? "%sOGG/%s.OGG" : "%sOGG/%s.MID.OGG", FILE_PREFIX, save.music);
		for (tempp = temp2; (*tempp != '\0'); tempp++) *tempp = toupper(*tempp);
		snprintf(temp3, sizeof(temp3), strrchr(save.music, '.') ? "%ogg/%s.ogg" : "%ogg/%s.mid.ogg", game_directory, save.music);
		#endif

		//printf("GAME_MUSIC_PLAY(): (6) '%s'\n", temp2);
		//printf("GAME_MUSIC_PLAY(): (7)\n");
		if (_file_exists(temp2)) {
			snprintf(temp, sizeof(temp), "%s", temp2);
		}
		else if (_file_exists(temp3)) {
			snprintf(temp, sizeof(temp), "%s", temp3);
		}
	#endif
	
	printf("Loading Music file : '%s'...\n", temp);
	
	#ifndef ENABLE_MUSIC
		printf("Music disabled\n");
		return;
	#endif

	if (!LoadFromMusic(temp)) 
	{
		printf("Can't play music '%s' from any directory\n", save.music);
		return;
	}
	
	printf("Playing...\n"); fflush(stdout);
	Mix_HaltMusic();
	Mix_VolumeMusic(volume_music);
	Mix_PlayMusic(music, -1);
	Mix_VolumeMusic(volume_music);
}

void GAME_MUSIC_STOP() {
	if (!audio_initialized) return;
	Mix_HaltMusic();
	Mix_FreeMusic(music);
	music = NULL;	
}

void GAME_SOUND_PLAY_CH(Mix_Chunk **chunk, char *name, int channel) {
	SDL_RWops *f; char temp[0x40];
	
	if (!audio_initialized) return;
	
	Mix_HaltChannel(channel);
	
	if (*chunk) {
		Mix_FreeChunk(*chunk);
		*chunk = NULL;
	}
	
	sprintf(temp, strrchr(name, '.') ? "%s" : "%s.WAV", name);
	if (!(f = VFS_LOAD(temp))) {
		printf("Can't sound ('%s')", temp);
		return;
	}
	
	*chunk = Mix_LoadWAV_RW(f, 1);
	
	if (!*chunk) {
		printf("Can't load wavfile '%s' (Out of memory?)", name);
		//PROGRAM_EXIT_ERROR("Can't load wavfile '%s' (Out of memory?)", name);
	}
	
	Mix_PlayChannel(channel, *chunk, 0);
}

void GAME_SOUND_PLAY(char *name) {
	if (!audio_initialized) return;
	if (!audio_voice_enabled) return;
	Mix_Volume(1, volume_effect);
	GAME_SOUND_PLAY_CH(&effect, name, 1);
}

void GAME_VOICE_PLAY(char *name) {
	if (!audio_initialized) return;
	if (!audio_voice_enabled) return;
	Mix_Volume(2, volume_voice);
	GAME_SOUND_PLAY_CH(&voice, name, 2);
}

void GAME_VOICE_STOP() {
	if (!audio_initialized) return;
	Mix_HaltChannel(2);
}

void GAME_SOUND_STOP() {
	if (!audio_initialized) return;
	Mix_HaltChannel(1);
}

void GAME_SOUND_CLICK() {
	if (!audio_initialized) return;
	Mix_Volume(3, volume_effect);
	Mix_PlayChannel(3, click, 0);
}

void GAME_SOUND_GC_COLLECT() {
	if (!audio_initialized) return;

	if (!Mix_Playing(2) && voice) {
		printf("((GC_voice))");
		Mix_HaltChannel(2);
		Mix_FreeChunk(voice);
		voice = NULL;
	}

	if (!Mix_Playing(1) && effect) {
		printf("((GC_effect))");
		Mix_HaltChannel(1);
		Mix_FreeChunk(effect);
		effect = NULL;
	}
}
