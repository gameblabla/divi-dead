#include "fileman.h"

void fileman_init() {
}

SDL_RWops* fileman_get(char* name) {
	char fname[0x50];
	sprintf(fname, "VER/%s", name);
	printf("Request: '%s'\n", name);
	return SDL_RWFromFile(fname, "rb");
}
