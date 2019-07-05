#ifndef VFS_H
#define VFS_H

#include "shared.h"
#include "structures.h"

#define temp1_update(len) temp_update(&temp1, len);
#define temp2_update(len) temp_update(&temp2, len);

//#define _file_exists debug_file_exists
//#define _file_exists file_exists
#define _file_exists rw_file_exists

typedef struct {
	char path[0x100];
	SDL_RWops *rw;
} PAK;

typedef struct {
	PAK *pak;
	char name[0x10];
	int pos, len;
} FSLI;

// Pak list
extern PAK paks[0x10];
extern int paks_count;

// File list (224K) max entries (8192) game entries (966+4051=5017)
extern FSLI files[0x2000];
extern int files_count;

extern void temp_update(uint8_t **ptr, int newlen);
extern int rw_file_exists(char *name);
extern int file_exists(char *name);
extern int debug_file_exists(char *name);
extern void VFS_RESET();

#endif
