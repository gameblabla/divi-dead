typedef struct {
	/*
	fwrite($f, pack('V', $len_header)); // chars_ptr
	fwrite($f, pack('V', sizeof($chars))); // chars_count
	fwrite($f, pack('V', $len_header + $len_chars)); // widths_ptr
	fwrite($f, pack('V', sizeof($widths))); // widths_count
	fwrite($f, pack('V', $len_header + $len_chars + $len_widths)); // glyphs_ptr
	fwrite($f, pack('V', sizeof($chars))); // glyphs_count	
	*/
	unsigned short* chars;
	int chars_count;
	unsigned char* widths;
	int widths_count;
	char* glyphs;
	int glyphs_count;
} FONT;

int font_char_index(FONT *f, unsigned short c) {
	return 0;
}

int font_char_size_index(FONT *f, unsigned short idx) {
	return 1;
}

int font_char_width_index(FONT *f, unsigned short idx) {
	if (idx < 0 || idx >= f->widths_count) return 0;
	return f->widths[idx];
}

int font_char_size(FONT *f, unsigned short c) {
	return font_char_size_index(f, font_char_index(f, c));
}

int font_char_width(FONT *f, unsigned short c) {
	return font_char_width_index(f, font_char_index(f, c));
}

int font_string_width(FONT *f, unsigned short *c, int len) {
	int n, w = 0;
	for (n = 0; n < len; n++) w += font_char_width(f, c[n]);
	return w;
}

int font_string_decode(FONT *f, unsigned char *c, unsigned short *out) {
	int count = 0;
	while (c[0]) {
		int s = 1;
		unsigned short v = ((int)c[0]) | ((int)(c[1]) << 8);
		if (c[1]) s = font_char_size(f, v);
		if (s == 1) v &= 0xFF;
		*(out++) = v;
		c += s;
		count++;
	}
	*(out++) = 0;
	return count;
}

SDL_Surface *font_char_render_index(FONT *f, unsigned short idx) {
	SDL_Surface *s = SDL_CreateRGBSurface(SDL_SRCALPHA | SDL_SWSURFACE, 20, 20, 32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
	int n, m;
	unsigned int *out;
	unsigned char *in = f->glyphs + idx * 100;
	
	if (s) {
		SDL_LockSurface(s);
			data = s->pixels;
			for (n = 0; n < 100; n++) {
				unsigned char c = *(in++);
				for (m = 0; m < 4; m++, c >>= 2) {
					int v = (c & 3);
					int a = (v * 0xFF) / 3;
					int r = 0xff, g = 0xff, b = 0xff;
					
					*data = (a << 0) | (g << 8) | (b << 16) | (r << 24);
					data++;
				}
			}
		SDL_UnlockSurface(s);
	}
	
	return s;
}

SDL_Surface *font_char_render(FONT *f, unsigned short c) {
	return font_char_render_index(f, font_char_index(f, c));
}

SDL_Surface *font_string_render(FONT *f, unsigned char *text, int height) {
	SDL_Surface *s;
	int buf_len;
	int w, h;
	unsigned short buffer[0x800];
	buf_len = font_string_decode(f, text, buffer);
	w = font_string_width(f, buffer, buf_len);
	h = 19;
	
	s = SDL_CreateRGBSurface(SDL_SRCALPHA | SDL_SWSURFACE, w, h, 32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
	
	
	return s;
}