#include <stdio.h>

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>

#include <stdlib.h>
#include <stdio.h>

#include "resman.h"
#include "hooks.h"

#include "audio.h"
#include "video.h"

int game_screen_current = 0;

void game_init() {
	HOOK_INIT_PRE();

	SDL_Init(0);

	video_init();
	sound_init();
}

void game_exit() {
	exit(0);
}

void game_events() {
	SDL_Event event;
	while (SDL_PollEvent(&event))  {
		switch (event.type) {
			case SDL_QUIT: game_exit(); break;
			case SDL_KEYDOWN:
			break;
		}
	}
}

#undef main

typedef int (*game_screen)(int type);

int game_screen_none(int type) {
	return 0;
}

int game_screen_main(int type) {
	switch (type) {
		case 1:
			video_foreground_draw("WAKU_A1");
			video_background_draw("TITLE");
			
			video_character2_draw("B01_1B", "B02A_2B");

			sound_voice_play("SAC0306");
			
			resman_print();
			
			video_transition_play(1);
			
			SWAIT(1000);

			video_background_draw("TITLE");
			video_character1_draw("B02A_1A");
			video_transition_play(2);
		break;
		case 0:
		break;
		case -1:
		break;		
	}
	return 0;
}

game_screen game_screen_list[] = {
	game_screen_none,
	game_screen_main
};


void game_screen_change(int gsid) {
	if (gsid == game_screen_current) return;
	game_screen_list[game_screen_current](-1);
	game_screen_list[gsid](+1);
	game_screen_current = gsid;
}

int main(int argc, char *argv[]) {
	game_init();
	fileman_init();
	resman_init(HOOK_RESMAN_SIZE);
	
	game_screen_change(1);
	
	while (1) {
		game_events();
		game_screen_list[game_screen_current](0);
	}
	
	return 0;
}
