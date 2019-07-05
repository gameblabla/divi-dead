#ifndef FONT_H
#define FONT_H

#include "shared.h"

extern int font_char_index(FONT *f, unsigned short c);

extern int font_char_size_index(FONT *f, unsigned short idx);

extern int font_char_width_index(FONT *f, unsigned short idx);

extern int font_char_size(FONT *f, unsigned short c);

extern int font_char_width(FONT *f, unsigned short c);
extern int font_string_width(FONT *f, unsigned short *c, int len);

extern int font_string_decode(FONT *f, unsigned char *c, unsigned short *out);
extern SDL_Surface *font_char_render_index(FONT *f, unsigned short idx);

extern SDL_Surface *font_char_render(FONT *f, unsigned short c);

extern SDL_Surface *font_string_render(FONT *f, unsigned char *text, int height);


#endif
