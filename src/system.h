#ifndef SYSTEM_H
#define SYSTEM_H

#include "shared.h"

#define VAR_SAVE_L(var, len) SDL_RWwrite(f, var, 1, len);
#define VAR_LOAD_L(var, len) SDL_RWread (f, var, 1, len);

extern char save_buffer[0x1800];
extern int save_buffer_size;

extern SDL_RWops *save_buffer_open();

extern SDL_RWops *save_buffer_open_load();
extern void SYS_SAVE_FULL(SDL_RWops *f);

extern void SYS_SAVE_MIN(SDL_RWops *f);

extern int SYS_SAVE() ;
extern void SYS_LOAD_FULL(SDL_RWops *f);
extern void SYS_LOAD_MIN(SDL_RWops *f);
extern int SYS_LOAD() ;
extern int FLIST_LOAD();
extern int FLIST_CHECK(char *name);

#endif
