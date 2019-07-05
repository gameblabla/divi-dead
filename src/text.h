#ifndef TEXT_H
#define TEXT_H

#include "shared.h"

#ifdef SJIS_SUPPORT
extern unsigned short shift_jis_buffer[0x200];
#endif

extern int TextSizeEncoding(TTF_Font *font, const char *text, int *w, int *h);
extern void GAME_TEXT_PRINT2_EX(unsigned char *text, SDL_Color color, SDL_Color bgcolor, SDL_Rect clip, int colorKey);

extern void GAME_TEXT_PRINT2(unsigned char *text, SDL_Color color, SDL_Color bgcolor, SDL_Rect clip);
extern SDL_Surface *text_snap;

extern void GAME_TEXT_SNAP() ;
extern void GAME_TEXT_CLEAR();

extern void GAME_TEXT_UPDATE();

extern int GAME_TEXT_LINE_Y(int n);
extern int GAME_TEXT_LINE_X();

extern void GAME_TEXT_PRINT_LINE(unsigned char *text, SDL_Color color, SDL_Color bgcolor, SDL_Rect clip, int n);
extern void GAME_TEXT_PRINT(char *text, SDL_Color color, SDL_Rect clip);

#endif
