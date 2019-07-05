#ifndef IMAGES_H
#define IMAGES_H

#include "shared.h"

#define INTRINSIC_CHARACTERS

#define fadeOutPixel_c(c)  if (*c >= param) *c -= param; else *c = 0;
#define fadeOut2Pixel_c(c) if (*c >= 0x100 - param) *c += param; else *c = 0xFF;

#define processSurface_iterate() { \
	SDL_LockSurface(s); \
	cur = s->pixels; \
	end = s->pixels + s->pitch * s->h; \
	for (;cur < end; cur++) { \
		SDL_GetRGBA(*cur, pf, &r, &g, &b, &a); \
		func(param, &r, &g, &b, &a); \
		*cur = SDL_MapRGBA(pf, r, g, b, a); \
	} \
	SDL_UnlockSurface(s); \
}

#define PROCESS_V(idx) v[idx] = (*ptr >> (idx * 8)) & 0xFF; v[idx] -= step; if (v[idx] < 0) v[idx] = 0;
#define PROCESS_V2(idx) v[idx] = (*ptr >> (idx * 8)) & 0xFF; v[idx] += step; if (v[idx] > 0xFF) v[idx] = 0xFF;
#define PROCESS_I(idx) (v[idx] << (idx * 8))

#ifdef INTRINSIC_CHARACTERS
extern SDL_RWops *ichars[256];
extern void ICHAR_process(int dummy_size);
extern void ICHAR_load_process();
extern SDL_Surface *CompositeCharacterGetColor(int type, int alt);

extern SDL_Surface *CompositeCharacterGetMask(int type, int alt);

extern void colorTransformSaturate(float *c) ;

extern void colorTransform(int type, float *r, float *g, float *b, float *a);
extern void colorTransformC(int type, SDL_Color* c) ;

extern int SDL_GetPixel(SDL_Surface *s, int x, int y);
extern SDL_Surface *CompositeCharacter(int type, int alt, int face, int palette) ;
extern SDL_Surface *CompositeCharacterName(char *name);
extern SDL_Surface *character_img;
#endif


extern void fadeOutPixel(int param, uint8_t *r, uint8_t *g, uint8_t *b, uint8_t *a);
extern void fadeOut2Pixel(int param, uint8_t *r, uint8_t *g, uint8_t *b, uint8_t *a);

extern void processSurface(SDL_Surface *s, SDL_Rect *rect, void (*func)(int, uint8_t*, uint8_t*, uint8_t*, uint8_t*), int param);

extern void GAME_BUFFER_FADEOUT_EX(int step);

extern void GAME_BUFFER_FADEOUT() ;
extern void GAME_BUFFER_FADEOUT2_EX(int step) ;

extern void GAME_BUFFER_FADEOUT2();
extern int SDL_RWsize(SDL_RWops *f);

extern void reduceMemory() ;
extern SDL_Surface *GAME_IMAGE_GET_EX3(char *_name1, char *_name2, int usecache, int scale) ;
extern SDL_Surface *GAME_IMAGE_GET_EX2(char *_name1, char *_name2, int usecache);

extern SDL_Surface *GAME_IMAGE_GET_EX(char *name, int usecache);
extern SDL_Surface *GAME_IMAGE_GET(char *name);

extern void GAME_BACKGROUND(char *name, SDL_Rect clip, int colorKey);
extern void GAME_BACKGROUND_O(char *name);

extern void GAME_BACKGROUND_I(char *name);
extern void GAME_BACKGROUND_I_MASK(char *name);

extern void GAME_CHARA_EX(char *_name, SDL_Rect clip);

extern void GAME_CHARA(char *name);

extern void GAME_CHARA2(char *name1, char *name2);

#endif
