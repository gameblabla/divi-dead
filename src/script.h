#ifndef SCRIPT_H
#define SCRIPT_H

#include "shared.h"

extern uint8_t  SCRIPT_GET8 ();
extern uint16_t SCRIPT_GET16();
extern uint32_t   SCRIPT_GET32();
extern char  *STRING_GETSZ();
extern int    RANGE_COLLAPSE(int v, int m, int M);
extern void GAME_SCRIPT_JUMP(int addr);
extern int filter_animation(char c);

extern int GAME_SCRIPT_LOAD(char *name);

extern void GAME_SAVE_FULL(SDL_RWops *f);
extern int GAME_SAVE(int n);

extern void GAME_LOAD_FULL(SDL_RWops *f);
extern void GAME_LOAD_MIN(SDL_RWops *f);
extern int GAME_LOAD(int n);

extern void GAME_SAVE_POSITION_2();
extern void GAME_SCRIPT_PROCESS();

#endif
