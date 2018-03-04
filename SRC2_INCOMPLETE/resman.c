#include "resman.h"

#define RESMAN_MAXNAME    0x30
#define RESMAN_MAXENTRIES 0x300

typedef struct {
	char name[RESMAN_MAXNAME];
	int priority;
	// Data
	char* data; int size;
} RESMAN_E;

typedef struct {
	RESMAN_E entries[RESMAN_MAXENTRIES];
	int maxpriority;
	int count, maxcount;
	int size, maxsize;
} RESMAN;

RESMAN resman;

void resman_init(int maxsize) {
	resman.count = 0;
	resman.size = 0;
	resman.maxcount = RESMAN_MAXENTRIES;
	resman.maxsize = maxsize;
	resman.maxpriority = 0;
}

void resman_print() {
	int n;
	printf("ResourceManager:\n");
	printf("  Size (max): %d\n", resman.maxsize);
	printf("  Size (cur): %d\n", resman.size);
	printf("  Items     : %d:\n", resman.count);
	for (n = 0; n < resman.count; n++) {
		RESMAN_E *e = &resman.entries[n];
		printf("    '%-24s':%05X (%d)\n", e->name, e->size, e->priority);
	}
}

void resman_free_entry(RESMAN_E* e) {
	free(e->data);
}

void resman_free_tick() {
	int n, i = 0, min = 0x7FFFFFF;
	
	if (resman.count == 0) error_fatal("Can't free more resources");
	
	for (n = 0; n < resman.count; n++) { int cv = resman.entries[n].priority;
		if (cv <= min) {
			min = cv;
			i = n;
		}
	}
	
	// Normalize
	resman.maxpriority = 0;
	for (n = 0; n < resman.count; n++) {
		resman.entries[n].priority -= min;
		if (resman.entries[n].priority > resman.maxpriority) resman.maxpriority = resman.entries[n].priority;
	}

	printf("Freeing: '%s' (%d)\n", resman.entries[i].name, resman.entries[i].size);
	resman_free_entry(&resman.entries[i]);
	resman.size -= resman.entries[i].size;
	resman.entries[i] = resman.entries[--resman.count];
}

void resman_free(int size) {
	while (resman.count >= resman.maxcount) resman_free_tick();
	while (resman.size + size >= resman.maxsize) resman_free_tick();
}

void resman_free_all() {
	while (resman.count > 0) resman_free_tick();
}

SDL_RWops* resman_get_rw(char* name, int priority) {
	SDL_RWops* rw;
	int n, size;
	// Lo tenemos en la caché
	for (n = 0; n < resman.count; n++) if (strcmp(resman.entries[n].name, name) == 0) { resman.entries[n].priority++; goto found; }
	
	// No está en la caché, vamos a leerlo del FileManager
	if ((rw = fileman_get(name)) == NULL) return NULL;

	size = SDL_RWseek(rw, 0, SEEK_END);

	resman_free(size);
	
	RESMAN_E* e = &resman.entries[resman.count++];
	e->size = size;
	SDL_RWseek(rw, 0, SEEK_SET);
	if ((e->data = malloc(e->size)) == NULL) error_fatal("Can't alloc enough memory");
	switch (priority) {
		case 0: e->priority = 0; break;
		default:
		case 1: e->priority = ++resman.maxpriority; break;
	}
	
	SDL_RWread(rw, e->data, 1, e->size);
	strcpy(e->name, name);
	n = resman.count - 1;
	resman.size += e->size;
	
	SDL_RWclose(rw);
	
	found: return SDL_RWFromConstMem(resman.entries[n].data, resman.entries[n].size);
}

char nname[0x40];
char *resman_nname(char *s) {
	strcpy(nname, s);
	return game_normalize_name(s);
}

SDL_Surface* resman_get_image(char* name, int priority) {
	char temp[0x40]; sprintf(temp, "G/640x480/%s.G", resman_nname(name));
	return IMG_Load_RW(resman_get_rw(temp, priority), 1);
}

Mix_Chunk* resman_get_sound(char* name, int priority) {
	char temp[0x40]; sprintf(temp, "S/%s.S", resman_nname(name));
	return Mix_LoadWAV_RW(resman_get_rw(temp, priority), 1);
}
