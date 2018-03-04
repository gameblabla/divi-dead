#ifdef ENABLE_VIDEO_SMPEG
#include <smpeg.h>
#endif

void MOVIE_PLAY(char *name, int skip) {
#ifndef ENABLE_VIDEO_SMPEG
	printf("Playing '%s'... (compiled without video/mpeg support)\n", name);
#else
    SMPEG* mpeg;
    SMPEG_Info mpeg_info;
	int rw, rh;
	
	printf("Playing '%s'...", name);
	
	if (!_file_exists(name)) { printf("Not found\n"); return; }

	//mpeg = SMPEG_new(name, &mpeg_info, 0);
	//mpeg = SMPEG_new_rwops(RING_RW_open(SDL_RWFromFile(name, "rw"), 0x800000), &mpeg_info, 0);
	//mpeg = SMPEG_new_rwops(RING_RW_open(SDL_RWFromFile(name, "rw"), 0x20000), &mpeg_info, 0);

	#ifdef RINGBUF_AUDIO
		mpeg = SMPEG_new_rwops(RING_RW_open(SDL_RWFromFile(name, "rw"), 0x200000, 0x2000), &mpeg_info, 0);
		//mpeg = SMPEG_new_rwops(RING_RW_open(SDL_RWFromFile(name, "rw"), 0x20000, 0x1000), &mpeg_info, 0);
		//mpeg = SMPEG_new(name, &mpeg_info, 0);
	#else
		mpeg = SMPEG_new(name, &mpeg_info, 0);
	#endif
	
	mpeg = SMPEG_new(name, &mpeg_info, 0);
	
	if (1) {
		printf("\n");
		printf("MOVIE('%s') {\n", name);
		printf(" mpeg : %08X\n", mpeg);
		printf(" VIDEO: %d\n", mpeg_info.has_video);
		printf(" AUDIO: %d (%s)\n", mpeg_info.has_audio, mpeg_info.audio_string);
		printf(" SIZE : %dx%d\n", mpeg_info.width, mpeg_info.height);
		printf(" FPS  : %f\n", (float)mpeg_info.current_fps);
		printf(" TIME : %f\n", (float)mpeg_info.total_time);
		printf("}...");
	}
	
	if ((mpeg == NULL) || (mpeg_info.width == 0)) { printf("Invalid file\n"); return; }
	
	printf("Ok\n");
	
	SMPEG_enableaudio(mpeg, 0);
	if (1) {
		SDL_AudioSpec audiofmt;
		Uint16 format;
		int freq, channels;

		Mix_QuerySpec(&freq, &format, &channels);
		audiofmt.format = format;
		audiofmt.freq = freq;
		audiofmt.channels = channels;
		SMPEG_actualSpec(mpeg, &audiofmt);

		Mix_HookMusic(SMPEG_playAudioSDL, mpeg);
		SMPEG_enableaudio(mpeg, 1);
	}

	rw = mpeg_info.width;
	rh = mpeg_info.height;
	
	SMPEG_setdisplay(mpeg, screen_video, NULL, NULL);

	if ((screen->w == 640 && screen->h == 480) || (abs(rw - screen->w) < 80)) {
		SMPEG_scaleXY(mpeg, rw, rh);
	} else {
		rw = screen->w - 16;
		rh = (rh * rw) / mpeg_info.width;
		SMPEG_scaleXY(mpeg, rw, rh);
	}

	SMPEG_move(mpeg, screen->w / 2 - rw / 2, screen->h / 2 - rh / 2);
	SMPEG_setvolume(mpeg, volume_movie);
	
	printf("Play...");
    SMPEG_play(mpeg);

	printf("Loop...");
    while (SMPEG_status(mpeg) == SMPEG_PLAYING) {
		KEYS_UPDATE();
		if (skip && ((keys & K_A) || (keys & K_B))) break;
		PROGRAM_DELAY(6);
	}
	printf("End\n");

	SMPEG_stop(mpeg);
	Mix_HookMusic(NULL, NULL);
	SMPEG_delete(mpeg);
	
	printf("MOVIE_PLAY:END\n");
	
	//exit(-1);
#endif
}
