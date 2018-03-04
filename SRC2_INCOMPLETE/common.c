#include "common.h"

void error_fatal(char* s) {
	printf("ERROR!");
	exit(-1);
}

char *game_normalize_name(char *s) {
	char *r = s;
	for (;*s; s++) {
		if (*s >= 'a' && *s <= 'z') *s += ('a' - 'A');
		if (*s == '.') { *s = '\0'; break; }
	}
	return r;
}

SDL_Rect SRECT(int x, int y, int w, int h) {
	SDL_Rect r = {x, y, w, h};
	return r;
}

void SWAIT(int time) {
	int event = 0;
	unsigned int start = SDL_GetTicks();
	while (SDL_GetTicks() < start + time) {
		SDL_PumpEvents();
		SDL_Delay(1);
		event++;
	}
	if (!event) SDL_PumpEvents();
}
