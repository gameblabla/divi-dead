////////////////////////////////////////////////////////////
// TEXT                                                   //
////////////////////////////////////////////////////////////

#include "text.h"

#ifdef SJIS_SUPPORT
#include "sijs_table.h"
unsigned short shift_jis_buffer[0x200] = {0};
#endif

int TextSizeEncoding(TTF_Font *font, const char *text, int *w, int *h) {
	int r;
	if (!is_shift_jis) {
		r = TTF_SizeText(font, text, w, h);
	} else {
		#ifdef SJIS_SUPPORT
		sjis_decode(text, shift_jis_buffer);
		r = TTF_SizeUNICODE(font, shift_jis_buffer, w, h);
		#endif
	}
	return r;
}

void GAME_TEXT_PRINT2_EX(unsigned char *text, SDL_Color color, SDL_Color bgcolor, SDL_Rect clip, int colorKey) {
	SDL_Surface *surface = NULL;
	int incy;
	
	if (is_shift_jis) {
		#ifdef SJIS_SUPPORT
		sjis_decode(text, shift_jis_buffer);
		surface = TTF_RenderUNICODE_Shaded(font, shift_jis_buffer, color, bgcolor);
		#endif
	} else {
		surface = TTF_RenderText_Shaded(font, text, color, bgcolor);
	}
	
	if (surface != NULL) {
		incy = font_height - surface->h + 1;
		clip.y += incy;
		clip.h -= incy;
		if (colorKey) SDL_SetColorKey(surface, SDL_SRCCOLORKEY, SDL_MapRGB(surface->format, bgcolor.r, bgcolor.g, bgcolor.b));
		SDL_BlitSurface(surface, NULL, screen, &clip);
		SDL_FreeSurface(surface);
	} else {
		printf("Can't print text('%s')\n", text);
	}
}

void GAME_TEXT_PRINT2(unsigned char *text, SDL_Color color, SDL_Color bgcolor, SDL_Rect clip) {
	GAME_TEXT_PRINT2_EX(text, color, bgcolor, clip, 1);
}

SDL_Surface *text_snap = NULL;

void GAME_TEXT_SNAP() {
	SDL_PixelFormat *pf = screen->format;
	if (!text_snap) text_snap = SDL_CreateRGBSurface(SDL_SWSURFACE, clip_text.w, clip_text.h, pf->BitsPerPixel, pf->Rmask, pf->Gmask, pf->Bmask, pf->Amask);
	SDL_BlitSurface(screen, &clip_text, text_snap, NULL);
}

void GAME_TEXT_CLEAR() {
	if (text_snap) {
		SDL_BlitSurface(text_snap, NULL, screen, &clip_text);
	} else {
		printf("WARNING! No TEXT SNAP to use; clearing with black");
		//SDL_FillRect(screen, &clip_text, SDL_MapRGB(screen->format, 0x00, 0x00, 0x00));
	}
}

void GAME_TEXT_UPDATE() {
	GAME_SCREEN_UPDATE_RECT(screen, &clip_text);
}

int GAME_TEXT_LINE_Y(int n) {
	return clip_text.y + text_pos.y + (font_height + text_pos.h) * n - text_margin_top;
}

int GAME_TEXT_LINE_X() {
	return clip_text.x + text_pos.x;
}

//void GAME_TEXT_PRINT_LINE_EX(unsigned char *text, SDL_Color color, SDL_Color bgcolor, SDL_Rect clip, int n) {
void GAME_TEXT_PRINT_LINE(unsigned char *text, SDL_Color color, SDL_Color bgcolor, SDL_Rect clip, int n) {
	SDL_Rect pos = clip;
	char temp[0x400];
	char *buf = temp;
	int mx = clip.x + clip.w;
	int w, h;
	int incy = (font_height + text_pos.h);
	int sx = clip.x + text_pos.x, sy = clip.y + text_pos.y + incy * n;
	int is_name_post = 0, is_name = 0;
	int count = 0, sj_first = 1;
	
	/*{
		int n;
		printf("\n TEXT (%08X) {", text);
		for (n = 0; n < strlen(text); n++) printf("%02X", (unsigned char)text[n]);
		printf("}\n");	
	}*/
	
	pos.x = sx; pos.y = sy;
	do {
		char c = *text;
		if (!is_shift_jis) {
			switch (c) {
				case '@': c = '"'; break;
				default: break;
			}
		}
		*buf++ = c; 

		#ifdef SJIS_SUPPORT
		if (is_shift_jis) {
			if (sjis_charlen(c) == 2) {
				if (sj_first) {
					sj_first = 0;
				} else {
					sj_first = 1;
					count++;
				}
			} else {
				sj_first = 1;
				count++;
			}
		}
		#endif
		
		if ((count >= 5) || (*text == ' ' || !*text)) {
			if (temp[0] == 0) break;
			if (*text == 0) *buf++ = ' ';
			*buf++ = 0;
			
			TextSizeEncoding(font, temp, &w, &h);
			
			if (pos.x + w >= mx) {
				pos.x = sx;
				pos.y += incy;
			}
			//printf("%s\n", temp);
			#ifdef SJIS_SUPPORT
			if (!is_shift_jis) {
				for (buf = temp; *buf; buf++) {
					if (*buf == '[') { is_name_post = is_name = 1; }
					if (*buf == ']') { is_name_post = 0; break; }
				}
			}
			#endif
			GAME_TEXT_PRINT2(temp, is_name ? blue : color, bgcolor, pos);
			is_name = is_name_post;
			pos.x += w;
			buf = temp;
			
			count = 0;
		}
	} while (*text++);
}

/*void GAME_TEXT_PRINT_LINE(unsigned char *text, SDL_Color color, SDL_Color bgcolor, SDL_Rect clip, int n) {
	if (is_shift_jis) {
		GAME_TEXT_PRINT_LINE_EX(text, color, bgcolor, clip, n);
	}
}*/

void GAME_TEXT_PRINT(char *text, SDL_Color color, SDL_Rect clip) {
	GAME_TEXT_PRINT_LINE(text, color, black, clip, 0);
}
