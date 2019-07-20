#ifndef VFS_H
#define VFS_H

#include "shared.h"
#include "structures.h"

#define temp1_update(len) temp_update(&temp1, len);
#define temp2_update(len) temp_update(&temp2, len);

//#define _file_exists debug_file_exists
//#define _file_exists file_exists
#define _file_exists rw_file_exists

extern SDL_RWops *VFS_LOAD(char *name);
extern FSLI *VFS_FIND(char *name);
extern SDL_RWops *STREAM_UNCOMPRESS_MEM(void* start, int size);
extern int VFS_MOUNT(char *name);

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
