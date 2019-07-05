#ifndef CREDIT_H
#define CREDIT_H

#include "shared.h"

extern char* CREDIT_READ();
extern void SDL_DrawSurface(SDL_Surface* s, int x, int y);
extern SDL_Surface *CREDIT_SF_RESIZE(SDL_Surface *in);
extern void CREDIT_SHOW(int can_exit);

#endif
